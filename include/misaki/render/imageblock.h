#pragma once

#include <tbb/spin_mutex.h>

#include "component.h"
#include "rfilter.h"

#define MSK_BLOCK_SIZE 32

namespace misaki::render {

class MSK_EXPORT ImageBlock : public Component {
 public:
  ImageBlock(const Vector2i &size, const ReconstructionFilter *filter = nullptr);
  ~ImageBlock();
  void put(const ImageBlock *b);
  bool put(const Vector2 &pos, const Color4 &val);
  void clear();
  std::string to_string() const;

  void set_offset(const Vector2i &offset) { m_offset = offset; }
  void set_size(const Vector2i &size);

  const Vector2i &offset() const { return m_offset; }
  const Vector2i &size() const { return m_size; }
  int border_size() const { return m_border_size; }
  math::Tensor<Color4, 2> &data() { return m_buffer; }
  const math::Tensor<Color4, 2> &data() const { return m_buffer; }

  MSK_DECL_COMP(Component)
 protected:
  math::Tensor<Color4, 2> m_buffer;
  const ReconstructionFilter *m_filter;
  Vector2i m_offset, m_size;
  int m_border_size = 0;
  Float *m_filter_weights = nullptr;
  Float m_filter_radius = 0;
  Float *m_weight_x = nullptr, *m_weight_y = nullptr;
  Float m_lookup_factor = 0;
  tbb::spin_mutex m_mutex;
};

class MSK_EXPORT BlockGenerator : public Component {
 public:
  BlockGenerator(const Vector2i &size, int block_size);
  size_t max_block_size() const { return m_block_size; }
  size_t block_count() const { return m_block_count; }
  void reset();
  // Return the `offset` and `size`
  std::tuple<Vector2i, Vector2i> next_block();

  MSK_DECL_COMP(Component)
 protected:
  enum class Direction {
    Right = 0,
    Down,
    Left,
    Up
  };
  size_t m_block_counter, m_block_count, m_block_size;
  Vector2i m_size, m_offset, m_blocks;
  Vector2i m_position;  // Relative position of current block
  Direction m_current_direction;
  int m_steps_left, m_steps;
  tbb::spin_mutex m_mutex;
};

}  // namespace misaki::render