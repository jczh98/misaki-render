#include <aspirin/film.h>
#include <aspirin/imageblock.h>
#include <aspirin/properties.h>

#include <fstream>

namespace aspirin {

class RGBFilm final : public Film {
public:
    RGBFilm(const Properties &props) : Film(props) {
        m_storage = new ImageBlock(m_size, filter());
        m_storage->clear();
    }

    void put(const ImageBlock *block) override {
        tbb::spin_mutex::scoped_lock lock(m_mutex);
        m_storage->put(block);
    }

    std::shared_ptr<Bitmap> bitmap() override {
        auto divide_by_filter_weight = [&](Color4 v) {
            Color3 rgb;
            if (v.w() != 0)
                rgb = v.template head<3>() / v.w();
            else
                rgb = Color3::Zero();
            return rgb;
        };
        auto result = std::make_shared<Bitmap>(m_size);
        for (int y = 0; y < m_size.y(); ++y)
            for (int x = 0; x < m_size.x(); ++x)
                result->coeffRef(y, x) = divide_by_filter_weight(
                    m_storage->data().coeff(y + m_storage->border_size(),
                                            x + m_storage->border_size()));
        return result;
    }

    void develop() override {
        fs::path filename = m_dest_file;
        bitmap()->write(filename.string());
    }

    void set_destination_file(const fs::path &filename) override {
        m_dest_file = filename;
    }

    APR_DECLARE_CLASS()
private:
    ref<ImageBlock> m_storage;
    fs::path m_dest_file;
    tbb::spin_mutex m_mutex;
};

APR_IMPLEMENT_CLASS(RGBFilm, Film)
APR_INTERNAL_PLUGIN(RGBFilm, "rgbfilm")

} // namespace aspirin