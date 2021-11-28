#pragma once

#include <tbb/spin_mutex.h>

#include "misaki/core/object.h"
#include "rfilter.h"

constexpr auto MSK_BLOCK_SIZE = 32;

namespace misaki {

class MSK_EXPORT ImageBlock : public Object {
public:
    ImageBlock(const Eigen::Vector2i &size, size_t channel_count,
               const ReconstructionFilter *filter = nullptr,
               bool warn_negative = true, bool warn_invalid = true,
               bool border = true);

    void put(const ImageBlock *block);

    inline bool put(const Eigen::Vector2f &pos, const Spectrum &value,
             const float &alpha) {
        float values[4] = { value.x(), value.y(), value.z(), alpha };
        return put(pos, values);
    }

    bool put(const Eigen::Vector2f &pos, const float *value);

    void clear();

    void set_offset(const Eigen::Vector2i &offset) { m_offset = offset; }

    void set_size(const Eigen::Vector2i &size);

    const Eigen::Vector2i &offset() const { return m_offset; }

    const Eigen::Vector2i &size() const { return m_size; }

    size_t width() const { return m_size.x(); }

    size_t height() const { return m_size.y(); }

    void set_warn_invalid(bool value) { m_warn_invalid = value; }

    bool warn_invalid() const { return m_warn_invalid; }

    void set_warn_negative(bool value) { m_warn_negative = value; }

    bool warn_negative() const { return m_warn_negative; }

    size_t channel_count() const { return (size_t) m_channel_count; }

    int border_size() const { return m_border_size; }

    std::vector<float> &data() { return m_data; }

    const std::vector<float> &data() const { return m_data; }

    MSK_DECLARE_CLASS()

protected:
    virtual ~ImageBlock();

    void accumulate_2d(const float *source, Eigen::Vector2i source_size,
                       float *target, Eigen::Vector2i target_size,
                       Eigen::Vector2i source_offset,
                       Eigen::Vector2i target_offset, Eigen::Vector2i size,
                       size_t channel_count);

protected:
    Eigen::Vector2i m_offset, m_size;
    uint32_t m_channel_count;
    int m_border_size;
    std::vector<float> m_data;
    const ReconstructionFilter *m_filter;
    float *m_weights_x, *m_weights_y;
    bool m_warn_negative;
    bool m_warn_invalid;
};

class MSK_EXPORT BlockGenerator : public Object {
public:
    BlockGenerator(const Eigen::Vector2i &size, const Eigen::Vector2i &offset,
                   int block_size);
    size_t max_block_size() const { return m_block_size; }
    size_t block_count() const { return m_block_count; }

    /// Reset the spiral to its initial state. Does not affect the number of
    /// passes.
    void reset();

    // Return the `offset` and `size`
    std::tuple<Eigen::Vector2i, Eigen::Vector2i, size_t> next_block();

    MSK_DECLARE_CLASS()
protected:
    enum class Direction { Right = 0, Down, Left, Up };

    size_t m_block_counter, //< Number of blocks generated so far
        m_block_count,      //< Total number of blocks to be generated
        m_block_size;       //< Size of the (square) blocks (in pixels)

    Eigen::Vector2i m_size, //< Size of the 2D image (in pixels).
        m_offset, //< Offset to the crop region on the sensor (pixels).
        m_blocks; //< Number of blocks in each direction.

    Eigen::Vector2i m_position; //< Relative position of the current block.
    /// Direction where the spiral is currently headed.
    Direction m_current_direction;
    /// Step counters.
    int m_steps_left, m_steps;

    /// Protects the spiral's state (thread safety).
    tbb::spin_mutex m_mutex;
};

} // namespace misaki