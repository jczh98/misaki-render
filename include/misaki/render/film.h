#pragma once

#include "imageblock.h"
#include "misaki/core/image.h"
#include "misaki/core/object.h"
#include "rfilter.h"

namespace misaki {

class MSK_EXPORT Film : public Object {
public:
    virtual void prepare(const std::vector<std::string> &channels) = 0;

    virtual void put(const ImageBlock *block) = 0;

    virtual void set_destination_file(const fs::path &filename);

    virtual bool destination_exists(const fs::path &basename) const = 0;

    virtual void develop() = 0;

    virtual std::shared_ptr<Image> image() = 0;

    const Eigen::Vector2i &size() const { return m_size; }

    const Eigen::Vector2i &crop_size() const { return m_size; }

    const Eigen::Vector2i &crop_offset() const { return m_crop_offset; }

    void set_crop_window(const Eigen::Vector2i &crop_offset,
                         const Eigen::Vector2i &crop_size);

    const ReconstructionFilter *filter() const { return m_filter; }

    virtual std::string to_string() const override;

    MSK_DECLARE_CLASS()
protected:
    Film(const Properties &props);
    virtual ~Film();

protected:
    Eigen::Vector2i m_size, m_crop_size, m_crop_offset;
    ref<ReconstructionFilter> m_filter;
};

} // namespace misaki