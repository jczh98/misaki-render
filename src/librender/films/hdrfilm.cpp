#include <iostream>
#include <misaki/core/image.h>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/object.h>
#include <misaki/core/properties.h>
#include <misaki/core/spectrum.h>
#include <misaki/core/string.h>
#include <misaki/render/film.h>
#include <misaki/render/imageblock.h>
#include <mutex>

namespace misaki {

class HDRFilm final : public Film {
public:
    HDRFilm(const Properties &props) : Film(props) {
        m_file_format =
            string::to_lower(props.string("file_format", "openexr"));
        std::string pixel_format =
            string::to_lower(props.string("pixel_format", "rgba"));

        m_dest_file = props.string("filename", "");
    }

    void set_destination_file(const fs::path &dest_file) override {
        m_dest_file = dest_file;
    }

    void prepare(const std::vector<std::string> &channels) override {
        for (size_t i = 1; i < channels.size(); ++i) {
            if (channels[i] == channels[i - 1])
                Throw("Film::prepare(): duplicate channel name \"%s\"",
                      channels[i]);
        }

        m_storage = new ImageBlock(m_crop_size, channels.size());
        m_storage->set_offset(m_crop_offset);
        m_storage->clear();
        m_channels = channels;
    }

    void put(const ImageBlock *block) override {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_storage->put(block);
    }

    std::shared_ptr<Image> image() override {
        const auto channel_count = m_channels.size();
        auto image    = std::make_shared<Image>(m_storage->size(), m_channels);
        bool has_aovs = channel_count != 4;
        for (int x = 0; x < m_size.x(); x++) {
            for (int y = 0; y < m_size.y(); y++) {
                uint32_t base_index = channel_count * (y * m_size.x() + x);

                Eigen::Vector3f xyz =
                    Eigen::Vector3f(m_storage->data()[base_index],
                                    m_storage->data()[base_index + 1],
                                    m_storage->data()[base_index + 2]);
                Eigen::Vector3f rgb = xyz_to_srgb(xyz);
                float weight        = m_storage->data()[base_index + 3];
                float inv_weight    = weight != 0 ? 1.f / weight : 0.f;
                    
                rgb *= inv_weight;

                image->operator()(x, y, 0) = rgb.x();
                image->operator()(x, y, 1) = rgb.y();
                image->operator()(x, y, 2) = rgb.z();
                image->operator()(x, y, 3) = 1.f;

                for (int ch = 4; ch < channel_count; ch++) {
                    image->operator()(x, y, ch) =
                        m_storage->data()[base_index + ch] * inv_weight;
                }
            }
        }
        return image;
    };

    void develop() override {
        if (m_dest_file.empty())
            Throw("Destination file not specified, cannot develop.");

        fs::path filename = m_dest_file;
        std::string proper_extension;
        if (m_file_format == "openexr")
            proper_extension = ".exr";
        else if (m_file_format == "rgbe")
            proper_extension = ".rgbe";
        else
            proper_extension = ".pfm";

        std::string extension = string::to_lower(filename.extension().string());
        if (extension != proper_extension)
            filename.replace_extension(proper_extension);

        Log(Info, "\U00002714  Developing \"{}\" ..", filename.string());

        image()->write(filename);
    }

    bool destination_exists(const fs::path &base_name) const override {
        std::string proper_extension;
        if (m_file_format == "openexr")
            proper_extension = ".exr";
        else if (m_file_format == "rgbe")
            proper_extension = ".rgbe";
        else
            proper_extension = ".pfm";

        fs::path filename = base_name;

        std::string extension = string::to_lower(filename.extension().string());
        if (extension != proper_extension)
            filename.replace_extension(proper_extension);

        return fs::exists(filename);
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "HDRFilm[" << std::endl
            << "  size = " << m_size << "," << std::endl
            << "  crop_size = " << m_crop_size << "," << std::endl
            << "  crop_offset = " << m_crop_offset << "," << std::endl
            << "  filter = " << m_filter << "," << std::endl
            << "  file_format = " << m_file_format << "," << std::endl
            << "  dest_file = \"" << m_dest_file << "\"" << std::endl
            << "]";
        return oss.str();
    }

    MSK_DECLARE_CLASS()
protected:
    std::string m_file_format;
    fs::path m_dest_file;
    ref<ImageBlock> m_storage;
    std::mutex m_mutex;
    std::vector<std::string> m_channels;
};

MSK_IMPLEMENT_CLASS(HDRFilm, Film)
MSK_REGISTER_INSTANCE(HDRFilm, "hdrfilm")

} // namespace misaki