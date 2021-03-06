#pragma once

#include <tbb/spin_mutex.h>

#include "object.h"
#include "rfilter.h"

#define APR_BLOCK_SIZE 32

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT ImageBlock : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)

    using ReconstructionFilter = ReconstructionFilter<Float, Spectrum>;
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
    using Float = float;
    APR_IMPORT_CORE_TYPES(float)

    BlockGenerator(const Vector2i &size, int block_size);
    size_t max_block_size() const { return m_block_size; }
    size_t block_count() const { return m_blocks_left; }
    // Return the `offset` and `size`
    std::tuple<Vector2i, Vector2i> next_block();

    APR_DECLARE_CLASS()
protected:
    enum Direction { Right = 0, Down, Left, Up };
    Vector2i m_block, m_size, m_num_blocks;
    int m_block_size, m_num_steps, m_blocks_left, m_steps_left;
    int m_direction;
    tbb::spin_mutex m_mutex;
};

APR_EXTERN_CLASS(ImageBlock)

} // namespace aspirin