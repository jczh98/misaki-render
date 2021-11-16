#pragma once

#include "bitmap.h"
#include "imageblock.h"
#include "object.h"
#include "rfilter.h"

namespace misaki {

class MSK_EXPORT Film : public Object {
public:
    virtual void put(const ImageBlock *block);
    virtual void set_destination_file(const fs::path &filename);
    virtual void develop();
    virtual std::shared_ptr<Bitmap> bitmap() = 0;
    const Eigen::Vector2i &size() const { return m_size; }
    const ReconstructionFilter *filter() const { return m_filter; }

    virtual std::string to_string() const override;

    MSK_DECLARE_CLASS()
protected:
    Film(const Properties &props);
    virtual ~Film();

protected:
    Eigen::Vector2i m_size;
    ref<ReconstructionFilter> m_filter;
};

} // namespace misaki