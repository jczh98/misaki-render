#pragma once

#include <tbb/spin_mutex.h>

#include "object.h"
#include "rfilter.h"

#define APR_BLOCK_SIZE 32

namespace aspirin {

class APR_EXPORT ImageBlock : public Object {
public:
    using DynamicBuffer =
        Eigen::Array<Color4, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    ImageBlock(const Vector2i &size,
               const ReconstructionFilter *filter = nullptr);

    virtual ~ImageBlock();

    void put(const ImageBlock *b);

    bool put(const Vector2 &pos, const Spectrum &val);

    void clear();

    std::string to_string() const override;

    void set_offset(const Vector2i &offset) { m_offset = offset; }

    void set_size(const Vector2i &size);

    const Vector2i &offset() const { return m_offset; }
    const Vector2i &size() const { return m_size; }
    int border_size() const { return m_border_size; }

    DynamicBuffer &data() { return m_buffer; }
    const DynamicBuffer &data() const { return m_buffer; }

    APR_DECLARE_CLASS()

protected:
    DynamicBuffer m_buffer;
    const ReconstructionFilter *m_filter;
    Vector2i m_offset, m_size;
    int m_border_size       = 0;
    Float *m_filter_weights = nullptr;
    Float m_filter_radius   = 0;
    Float *m_weight_x = nullptr, *m_weight_y = nullptr;
    Float m_lookup_factor = 0;
    mutable tbb::spin_mutex m_mutex;
};

class APR_EXPORT BlockGenerator : public Object {
public:
    BlockGenerator(const Vector2i &size, const Vector2i &offset, int block_size);
    size_t max_block_size() const { return m_block_size; }
    size_t block_count() const { return m_block_count; }

    /// Reset the spiral to its initial state. Does not affect the number of passes.
    void reset();

    // Return the `offset` and `size`
    std::tuple<Vector2i, Vector2i, size_t> next_block();

    APR_DECLARE_CLASS()
protected:
    enum class Direction {
        Right = 0,
        Down,
        Left,
        Up
    };

    size_t m_block_counter, //< Number of blocks generated so far
    m_block_count,   //< Total number of blocks to be generated
    m_block_size;    //< Size of the (square) blocks (in pixels)

    Vector2i m_size,        //< Size of the 2D image (in pixels).
    m_offset,      //< Offset to the crop region on the sensor (pixels).
    m_blocks;      //< Number of blocks in each direction.

    Vector2i  m_position;    //< Relative position of the current block.
    /// Direction where the spiral is currently headed.
    Direction m_current_direction;
    /// Step counters.
    int m_steps_left, m_steps;

    /// Protects the spiral's state (thread safety).
    tbb::spin_mutex m_mutex;
};

} // namespace aspirin