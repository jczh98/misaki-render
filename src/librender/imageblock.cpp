#include <iostream>
#include <misaki/imageblock.h>
#include <misaki/logger.h>
#include <sstream>
#include <tbb/spin_mutex.h>

namespace misaki {

ImageBlock::ImageBlock(const Eigen::Vector2i &size,
                       const ReconstructionFilter *filter)
    : m_offset(Eigen::Vector2i::Zero()), m_filter(filter), m_size(size) {
    if (filter) {
        m_filter_radius  = filter->radius();
        m_border_size    = (int) std::ceil(m_filter_radius - 0.5f);
        m_filter_weights = new float[APR_FILTER_RESOLUTION + 1];
        for (int i = 0; i < APR_FILTER_RESOLUTION; ++i) {
            m_filter_weights[i] = filter->eval(float(m_filter_radius * i) /
                                               APR_FILTER_RESOLUTION);
        }
        m_filter_weights[APR_FILTER_RESOLUTION] = 0.f;
        m_lookup_factor = APR_FILTER_RESOLUTION / m_filter_radius;
        int weight_size = (int) std::ceil(2 * m_filter_radius) + 1;
        m_weight_x      = new float[weight_size];
        m_weight_y      = new float[weight_size];
        memset(m_weight_x, 0, sizeof(float) * weight_size);
        memset(m_weight_y, 0, sizeof(float) * weight_size);
    }
    m_buffer.resize(size.y() + 2 * m_border_size, size.x() + 2 * m_border_size);
}

ImageBlock::~ImageBlock() {
    delete[] m_filter_weights;
    delete[] m_weight_x;
    delete[] m_weight_y;
}

void ImageBlock::put(const ImageBlock *b) {
    Eigen::Vector2i offset =
        b->offset() - m_offset +
        Eigen::Vector2i::Constant(m_border_size - b->border_size());
    Eigen::Vector2i size =
        b->size() + Eigen::Vector2i::Constant(2 * b->border_size());
    tbb::spin_mutex::scoped_lock lock(m_mutex);
    m_buffer.block(offset.y(), offset.x(), size.y(), size.x()) +=
        b->data().topLeftCorner(size.y(), size.x());
}

bool ImageBlock::put(const Eigen::Vector2f &pos_, const Spectrum &val) {
    Eigen::Vector2f offset = m_offset.template cast<float>();
    // TODO: check all value are valid
    Eigen::Vector2f pos =
        pos_ - (offset - Eigen::Vector2f::Constant(m_border_size + 0.5f));
    Eigen::Vector2i
        lo = math::ceil2int(
                 Eigen::Vector2f(
                     pos - Eigen::Vector2f::Constant(m_filter_radius)))
                 .cwiseMax(Eigen::Vector2i::Zero()),
        hi = math::floor2int(
                 Eigen::Vector2f(
                     pos + Eigen::Vector2f::Constant(m_filter_radius)))
                 .cwiseMin(m_size +
                           Eigen::Vector2i::Constant(2 * m_border_size - 1));
    for (uint32_t x = lo.x(), idx = 0; x <= hi.x(); ++x)
        m_weight_x[idx++] =
            m_filter_weights[(int) (std::abs(x - pos.x()) * m_lookup_factor)];
    for (uint32_t y = lo.y(), idx = 0; y <= hi.y(); ++y)
        m_weight_y[idx++] =
            m_filter_weights[(int) (std::abs(y - pos.y()) * m_lookup_factor)];
    for (uint32_t y = lo.y(), yr = 0; y <= hi.y(); ++y, ++yr)
        for (uint32_t x = lo.x(), xr = 0; x <= hi.x(); ++x, ++xr) {
            m_buffer.coeffRef(y, x) += Color4(val.r(), val.g(), val.b(), 1.f) *
                                       m_weight_x[xr] * m_weight_y[yr];
        }
    return true;
}

void ImageBlock::set_size(const Eigen::Vector2i &size) { m_size = size; }

void ImageBlock::clear() { m_buffer.setConstant(Color4::Zero()); }

std::string ImageBlock::to_string() const {
    std::ostringstream oss;
    oss << "ImageBlock[" << std::endl
        << "  offset = " << m_offset << "," << std::endl
        << "  size = " << m_size << "," << std::endl;
    if (m_filter)
        oss << "," << std::endl
            << "  filter = " << string::indent(m_filter->to_string());
    oss << std::endl << "]";
    return oss.str();
}

// Image Block Generator in sprial
BlockGenerator::BlockGenerator(const Eigen::Vector2i &size,
                               const Eigen::Vector2i &offset, int block_size)
    : m_block_size(block_size), m_size(size), m_offset(offset) {

    m_blocks =
        Eigen::Vector2i((int) std::ceil(size.x() / (float) block_size),
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