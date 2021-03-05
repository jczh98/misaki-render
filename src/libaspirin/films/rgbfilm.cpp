#include <aspirin/film.h>
#include <aspirin/imageblock.h>
#include <aspirin/imageio.h>
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
        auto to_srgb = [&](Float value) {
            if (value <= 0.0031308f)
                return 12.92f * value;
            else
                return (1.0f + 0.055f) * std::pow(value, 1.0f / 2.4f) - 0.055f;
        };
        auto bitmap = Array<Color3, 2>::from_linear_indexed(
            m_storage->data().shape(), [&](int i) {
                Color4 rgba = m_storage->data().raw_data()[i];
                Color3 rgb;
                if (rgba.w() != 0)
                    rgb = rgba.template head<3>() / rgba.w();
                else
                    rgb = Color3::Zero();
                return Color3(to_srgb(rgb.x()), to_srgb(rgb.y()),
                              to_srgb(rgb.z()));
            });
        auto another = m_dest_file;
        write_float_rgb_image(m_dest_file.string(), bitmap);
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