#pragma once

#include "imageblock.h"
#include "object.h"
#include "rfilter.h"

namespace aspirin {

template <typename Float, typename Spectrum>
class APR_EXPORT Film : public Object {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using ReconstructionFilter = ReconstructionFilter<Float, Spectrum>;

    virtual void put(const ImageBlock *block);
    virtual void set_destination_file(const fs::path &filename);
    virtual void develop();
    const Vector2i &size() const { return m_size; }
    const ReconstructionFilter *filter() const { return m_filter; }

    virtual std::string to_string() const override;

    APR_DECLARE_CLASS()
protected:
    Film(const Properties &props);
    virtual ~Film();

protected:
    Vector2i m_size;
    ref<ReconstructionFilter> m_filter;
};

APR_EXTERN_CLASS(Film)

} // namespace aspirin