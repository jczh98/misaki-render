#include <misaki/render/imageblock.h>
#include <tbb/spin_mutex.h>

#include <sstream>

namespace misaki::render {

ImageBlock::ImageBlock(const Vector2i &size, const ReconstructionFilter *filter)
    : m_offset(0, 0), m_filter(filter) {
  if (filter) {
    m_filter_radius = filter->radius();
    m_border_size = (int)std::ceil(m_filter_radius - 0.5f);
    m_filter_weights = new Float[MSK_FILTER_RESOLUTION + 1];
    for (int i = 0; i < MSK_FILTER_RESOLUTION; ++i) {
      m_filter_weights[i] = filter->eval((m_filter_radius * i) / MSK_FILTER_RESOLUTION);
    }
    m_filter_weights[MSK_FILTER_RESOLUTION] = 0.f;
    m_lookup_factor = MSK_FILTER_RESOLUTION / m_filter_radius;
    int weight_size = (int)std::ceil(2 * m_filter_radius) + 1;
    m_weight_x = new Float[weight_size];
    m_weight_y = new Float[weight_size];
    memset(m_weight_x, 0, sizeof(Float) * weight_size);
    memset(m_weight_y, 0, sizeof(Float) * weight_size);
  }
  set_size(size);
}

ImageBlock::~ImageBlock() {
  delete[] m_filter_weights;
  delete[] m_weight_x;
  delete[] m_weight_y;
}

void ImageBlock::put(const ImageBlock *b) {
  Vector2i source_size = b->size() + 2 * b->border_size(),
           target_size = m_size + 2 * border_size();

  Vector2i source_offset = b->offset() - b->border_size(),
           target_offset = m_offset - border_size();
  target_offset = source_offset - target_offset;
  source_offset = Vector2i(0);
  auto size = b->size() + 2 * b->border_size();
  auto shift = math::cwise_max(math::cwise_max(-source_offset, -target_offset), 0);
  source_offset += shift;
  target_offset += shift;
  size -= math::cwise_max(source_offset + size - source_size, 0);
  size -= math::cwise_max(target_offset + size - target_size, 0);

  std::lock_guard<tbb::spin_mutex> lock(m_mutex);
  for (int y = 0; y < size.y(); ++y)
    for (int x = 0; x < size.x(); ++x) {
      auto dst_x = target_offset.x() + x, dst_y = target_offset.y() + y;
      auto src_x = source_offset.x() + x, src_y = source_offset.y() + y;
      m_buffer.at({dst_y, dst_x}) += b->data().at({src_y, src_x});
    }
}

bool ImageBlock::put(const Vector2 &pos_, const Color4 &val) {
  auto offset = Vector2(m_offset);
  // TODO: check all value are valid
  Vector2 pos = pos_ - (offset - m_border_size + 0.5f);
  Vector2u lo = Vector2u(math::cwise_max(math::ceil2int(pos - m_filter_radius), 0)),
           hi = Vector2u(math::cwise_min(math::floor2int(pos + m_filter_radius), m_size + 2 * m_border_size - 1));
  for (int x = lo.x(), idx = 0; x <= hi.x(); ++x)
    m_weight_x[idx++] = m_filter_weights[(int)(std::abs(x - pos.x()) * m_lookup_factor)];
  for (int y = lo.y(), idx = 0; y <= hi.y(); ++y)
    m_weight_y[idx++] = m_filter_weights[(int)(std::abs(y - pos.y()) * m_lookup_factor)];
  for (int y = lo.y(), yr = 0; y <= hi.y(); ++y, ++yr)
    for (int x = lo.x(), xr = 0; x <= hi.x(); ++x, ++xr) {
      m_buffer.at({y, x}) += val * m_weight_x[xr] * m_weight_y[yr];
    }
  return true;
}

void ImageBlock::set_size(const Vector2i &size) {
  if (m_size == size) return;
  m_size = size;
  auto actual_size = m_size + 2 * m_border_size;
  m_buffer = math::Tensor<Color4, 2>::from_linear_indexed({actual_size.y(), actual_size.x()}, [&](int) { return Color4(); });
}

void ImageBlock::clear() {
  m_buffer = math::Tensor<Color4, 2>::from_linear_indexed(m_buffer.shape(), [&](int) { return Color4(); });
}

std::string ImageBlock::to_string() const {
  std::ostringstream oss;
  oss << "ImageBlock[" << std::endl
      << "  offset = " << m_offset << "," << std::endl
      << "  size = " << m_size << "," << std::endl;
  if (m_filter)
    oss << "," << std::endl
        << "  filter = " << string::indent(m_filter->to_string());
  oss << std::endl
      << "]";
  return oss.str();
}

// Image Block Generator in sprial
BlockGenerator::BlockGenerator(const Vector2i &size, int block_size)
    : m_block_size(block_size),
      m_size(size),
      m_offset(0) {
  m_blocks = Vector2i(math::ceil2int(Vector2f(m_size) / m_block_size));
  m_block_count = math::hprod(m_blocks);
  reset();
}

void BlockGenerator::reset() {
  m_block_counter = 0;
  m_current_direction = Direction::Right;
  m_position = m_blocks / 2;
  m_steps_left = 1;
  m_steps = 1;
}

std::tuple<Vector2i, Vector2i> BlockGenerator::next_block() {
  std::lock_guard<tbb::spin_mutex> lock(m_mutex);

  if (m_block_count == m_block_counter) {
    return {Vector2i(0), Vector2i(0)};
  }

  // Calculate a unique identifer per block

  Vector2i offset(m_position * (int)m_block_size);
  Vector2i size = math::cwise_min(m_size - offset, (int)m_block_size);
  offset += m_offset;

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
        m_current_direction = Direction(((int)m_current_direction + 1) % 4);
        if (m_current_direction == Direction::Left ||
            m_current_direction == Direction::Right)
          ++m_steps;
        m_steps_left = m_steps;
      }
    } while ((m_position.array() < 0 || m_position.array() >= m_blocks.array()).any());
  }

  return {offset, size};
}

}  // namespace misaki::render