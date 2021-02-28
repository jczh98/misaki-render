#pragma once

#include "component.h"
#include "imageblock.h"
#include "rfilter.h"

namespace aspirin {

template <typename Spectrum> class APR_EXPORT Film : public Component {
public:
    using ReconstructionFilter = ReconstructionFilter<Spectrum>;

    Film(const Properties &props);
    virtual void put(const ImageBlock *block);
    virtual void set_destination_file(const fs::path &filename);
    virtual void develop();
    const Vector2i &size() const { return m_size; }
    const ReconstructionFilter *filter() const { return m_filter.get(); }
    virtual std::string to_string() const override;

protected:
    Vector2i m_size;
    std::shared_ptr<ReconstructionFilter> m_filter;
};

} // namespace aspirin