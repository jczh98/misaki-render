#include <misaki/core/image.h>
#include <misaki/core/logger.h>
#include <OpenImageIO/imageio.h>

namespace misaki {

Channel::Channel(const std::string &name, const Eigen::Vector2i size):
    m_name(name) {
    m_data.resize(size.y(), size.x());
}

Image::Image(const Eigen::Vector2i &size,
             const std::vector<std::string> channels, uint8_t *data)
    : m_size(size) {
    for (int i = 0; i < channels.size(); i++) {
        m_channels.emplace_back(Channel(channels[i], size));
    }
}

void Image::write(const fs::path &path) {
    const std::string filename = path.string();
    std::unique_ptr<OIIO::ImageOutput> out =
        OIIO::ImageOutput::create(filename);
    if (!out) {
        Log(Warn, "Cannot create OIIO {}.", OIIO::geterror());
        return;
    }
    std::vector<float> pixels(m_size.x() * m_size.y() * m_channels.size());
    for (int i = 0; i < m_channels.size(); i++) {
        for (Eigen::DenseIndex j = 0; j < m_channels[i].count(); j++) {
            pixels[j * m_channels.size() + i] = m_channels[i].at(j);
        }
    }
    OIIO::ImageSpec spec(m_size.x(), m_size.y(), m_channels.size(),
                         OIIO::TypeDesc::FLOAT);
    spec.channelnames.clear();
    for (auto channel : m_channels) {
        spec.channelnames.push_back(channel.name());
    }
    out->open(filename, spec);
    out->write_image(OIIO::TypeDesc::FLOAT, pixels.data());
    out->close();
}

void Image::read(const fs::path &path) {}

} // namespace misaki