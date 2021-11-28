#include <iostream>
#include <misaki/core/logger.h>
#include <misaki/render/imageblock.h>
#include <sstream>
#include <tbb/spin_mutex.h>

namespace misaki {

ImageBlock::ImageBlock(const Eigen::Vector2i &size, size_t channel_count,
                       const ReconstructionFilter *filter, bool warn_negative,
                       bool warn_invalid, bool border)
    : m_offset(Eigen::Vector2i::Zero()), m_size(Eigen::Vector2i::Zero()),
      m_channel_count((uint32_t) channel_count), m_filter(filter),
      m_weights_x(nullptr), m_weights_y(nullptr),
      m_warn_negative(warn_negative), m_warn_invalid(warn_invalid) {
    m_border_size =
        (uint32_t) ((filter != nullptr && border) ? filter->border_size() : 0);

    if (filter) {
        // Temporary buffers used in put()
        int filter_size = (int) std::ceil(2 * filter->radius()) + 1;
        m_weights_x     = new float[2 * filter_size];
        m_weights_y     = m_weights_x + filter_size;
        memset(m_weights_x, 0, sizeof(float) * filter_size);
        memset(m_weights_y, 0, sizeof(float) * filter_size);
    }

    set_size(size);
}

ImageBlock::~ImageBlock() {
    if (m_weights_x)
        delete[] m_weights_x;
}

void ImageBlock::put(const ImageBlock *block) {
    if (block->channel_count() != channel_count())
        Throw("ImageBlock::put(): mismatched channel counts!");

    Eigen::Vector2i source_size = block->size() + 2 * Eigen::Vector2i::Constant(
                                                          block->border_size()),
                    target_size =
                        size() + 2 * Eigen::Vector2i::Constant(border_size());

    Eigen::Vector2i source_offset = block->offset() - Eigen::Vector2i::Constant(
                                                          block->border_size()),
                    target_offset =
                        offset() - Eigen::Vector2i::Constant(border_size());

    accumulate_2d(block->data().data(), source_size, data().data(), target_size,
                  Eigen::Vector2i::Zero(), source_offset - target_offset,
                  source_size, channel_count());
}

bool ImageBlock::put(const Eigen::Vector2f &pos_, const float *value) {
    // Check if all sample values are valid
    if (m_warn_negative || m_warn_invalid) {
        bool is_valid = true;

        if (m_warn_negative) {
            for (uint32_t k = 0; k < m_channel_count; ++k)
                is_valid &= value[k] >= -1e-5f;
        }

        if (m_warn_invalid) {
            for (uint32_t k = 0; k < m_channel_count; ++k)
                is_valid &= std::isfinite(value[k]);
        }

        if (!is_valid) {
            std::ostringstream oss;
            oss << "Invalid sample value: [";
            for (uint32_t i = 0; i < m_channel_count; ++i) {
                oss << value[i];
                if (i + 1 < m_channel_count)
                    oss << ", ";
            }
            oss << "]";
            Log(Warn, "{}", oss.str());
        }
    }
    float filter_radius = m_filter->radius();
    Eigen::Vector2i size =
        m_size + 2 * Eigen::Vector2i::Constant(m_border_size);

    const Eigen::Vector2f pos(pos_.x() - 0.5f - (m_offset.x() - m_border_size),
                              pos_.y() - 0.5f - (m_offset.y() - m_border_size));

    const Eigen::Vector2i lo(
        std::max((int) std::ceil(pos.x() - filter_radius), 0),
        std::max((int) std::ceil(pos.y() - filter_radius), 0)),
        hi(std::min((int) std::floor(pos.x() + filter_radius), size.x() - 1),
           std::min((int) std::floor(pos.y() + filter_radius), size.y() - 1));

    for (int x = lo.x(), idx = 0; x <= hi.x(); ++x)
        m_weights_x[idx++] = m_filter->eval_discretized(x - pos.x());
    for (int y = lo.y(), idx = 0; y <= hi.y(); ++y)
        m_weights_y[idx++] = m_filter->eval_discretized(y - pos.y());

    for (int y = lo.y(), yr = 0; y <= hi.y(); ++y, ++yr) {
        const float weight_y = m_weights_y[yr];
        float *dest =
            m_data.data() + (y * (size_t) size.x() + lo.x()) * m_channel_count;

        for (int x = lo.x(), xr = 0; x <= hi.x(); ++x, ++xr) {
            const float weight = m_weights_x[xr] * weight_y;

            for (uint32_t k = 0; k < m_channel_count; ++k)
                *dest++ += weight * value[k];
        }
    }

    return false;
}

void ImageBlock::clear() {
    Eigen::Vector2i expand_size =
        m_size + 2 * Eigen::Vector2i::Constant(m_border_size);
    size_t size = m_channel_count * expand_size.x() * expand_size.y();
    memset(m_data.data(), 0, size * sizeof(float));
}

void ImageBlock::set_size(const Eigen::Vector2i &size) {
    if (size == m_size)
        return;
    m_size = size;
    Eigen::Vector2i expand_size =
        size + 2 * Eigen::Vector2i::Constant(m_border_size);
    size_t size_ = m_channel_count * expand_size.x() * expand_size.y();
    m_data.resize(size_);
}

void ImageBlock::accumulate_2d(const float *source, Eigen::Vector2i source_size,
                               float *target, Eigen::Vector2i target_size,
                               Eigen::Vector2i source_offset,
                               Eigen::Vector2i target_offset,
                               Eigen::Vector2i size, size_t channel_count) {
    Eigen::Vector2i offset_increase(
        std::max(0, std::max(-source_offset.x(), -target_offset.x())),
        std::max(0, std::max(-source_offset.y(), -target_offset.y())));

    source_offset += offset_increase;
    target_offset += offset_increase;
    size -= offset_increase;

    Eigen::Vector2i size_decrease(
        std::max(0, std::max(source_offset.x() + size.x() - source_size.x(),
                             target_offset.x() + size.x() - target_size.x())),
        std::max(0, std::max(source_offset.y() + size.y() - source_size.y(),
                             target_offset.y() + size.y() - target_size.y())));

    size -= size_decrease;

    if (size.x() <= 0 || size.y() <= 0)
        return;

    const size_t columns = (size_t) size.x() * channel_count;

    source +=
        (source_offset.x() + source_offset.y() * (size_t) source_size.x()) *
        channel_count;
    target +=
        (target_offset.x() + target_offset.y() * (size_t) target_size.x()) *
        channel_count;

    for (int y = 0; y < size.y(); ++y) {
        for (size_t i = 0; i < columns; ++i)
            target[i] += source[i];
        source += source_size.x() * channel_count;
        target += target_size.x() * channel_count;
    }
    return;
}

// Image Block Generator in sprial
BlockGenerator::BlockGenerator(const Eigen::Vector2i &size,
                               const Eigen::Vector2i &offset, int block_size)
    : m_block_size(block_size), m_size(size), m_offset(offset) {

    m_blocks = Eigen::Vector2i((int) std::ceil(size.x() / (float) block_size),
                               (int) std::ceil(size.y() / (float) block_size));
    m_block_count = m_blocks.x() * m_blocks.y();

    reset();
}

void BlockGenerator::reset() {
    m_block_counter     = 0;
    m_current_direction = Direction::Right;
    m_position          = m_blocks / 2;
    m_steps_left        = 1;
    m_steps             = 1;
}

std::tuple<Eigen::Vector2i, Eigen::Vector2i, size_t>
BlockGenerator::next_block() {
    std::lock_guard<tbb::spin_mutex> lock(m_mutex);

    if (m_block_count == m_block_counter) {
        return { Eigen::Vector2i(0), Eigen::Vector2i(0), (size_t) -1 };
    }

    // Calculate a unique identifer per block
    size_t block_id = m_block_counter;

    Eigen::Vector2i offset(m_position * (int) m_block_size);
    Eigen::Vector2i size =
        (m_size - offset).cwiseMin(Eigen::Vector2i::Constant(m_block_size));
    offset += m_offset;

    // assert((size > 0).all());

    ++m_block_counter;

    if (m_block_counter != m_block_count) {
        // Prepare the next block's position along the spiral.
        do {
            switch (m_current_direction) {
                case Direction::Right:
                    ++m_position.x();
                    break;
                case Direction::Down:
                    ++m_position.y();
                    break;
                case Direction::Left:
                    --m_position.x();
                    break;
                case Direction::Up:
                    --m_position.y();
                    break;
            }

            if (--m_steps_left == 0) {
                m_current_direction =
                    Direction(((int) m_current_direction + 1) % 4);
                if (m_current_direction == Direction::Left ||
                    m_current_direction == Direction::Right)
                    ++m_steps;
                m_steps_left = m_steps;
            }
        } while ((m_position.array() < 0).any() ||
                 (m_position.array() >= m_blocks.array()).any());
    }

    return { offset, size, block_id };
}

MSK_IMPLEMENT_CLASS(ImageBlock, Object)
MSK_IMPLEMENT_CLASS(BlockGenerator, Object)

} // namespace misaki