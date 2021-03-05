#include <aspirin/film.h>
#include <aspirin/imageblock.h>
#include <aspirin/properties.h>

#include <fstream>

namespace aspirin {

template <typename Float, typename Spectrum>
class RGBFilm final : public Film<Float, Spectrum> {
public:
    APR_IMPORT_CORE_TYPES(Float)
    using Base = Film<Float, Spectrum>;
    using Base::m_size;
    using typename Base::ImageBlock;

    RGBFilm(const Properties &props) : Base(props) {
        m_storage = std::make_unique<ImageBlock>(m_size);
        m_storage->clear();
    }

    void put(const ImageBlock *block) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_storage->put(block);
    }

    void develop() override {
        //        auto bitmap = Array<Color3, 2>::from_linear_indexed(
        //            m_storage->data().shape(), [&](int i) {
        //                return m_storage->data()
        //                    .raw_data()[i]
        //                    .divide_by_filter_weight()
        //                    .to_srgb();
        //            });
        //        auto another = m_dest_file;
        //        image::write_rgb_image(another.replace_extension("jpg").string(),
        //                               bitmap);
        //        write_float_rgb_image(m_dest_file.string(), bitmap);
    }

    void set_destination_file(const fs::path &filename) override {
        m_dest_file = filename;
    }

    APR_DECLARE_CLASS()
private:
    std::unique_ptr<ImageBlock> m_storage;
    fs::path m_dest_file;
    std::mutex m_mutex;
};

APR_IMPLEMENT_CLASS_VARIANT(RGBFilm, Film)
APR_INTERNAL_PLUGIN(RGBFilm, "rgbfilm")

} // namespace aspirin