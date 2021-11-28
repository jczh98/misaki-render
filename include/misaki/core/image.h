#pragma once

#include "fwd.h"
#include "mathutils.h"

namespace misaki {

class Channel {
public:
    Channel(const std::string &name, const Eigen::Vector2i size);

    const std::string &name() const { return m_name; }
    const Eigen::RowMatrixXf &data() const { return m_data; }

    float &at(Eigen::DenseIndex index) { return m_data(index); }
    float at(Eigen::DenseIndex index) const { return m_data(index); }

    float &at(Eigen::Vector2i index) {
        return at(index.x() + index.y() * m_data.cols());
    }
    float at(Eigen::Vector2i index) const {
        return at(index.x() + index.y() * m_data.cols());
    }

    Eigen::DenseIndex count() const { return m_data.size(); }

    Eigen::Vector2i size() const { return { m_data.cols(), m_data.rows() }; }

    void set_zero() { m_data.setZero(); }

private:
    std::string m_name;
    Eigen::RowMatrixXf m_data;
};

class MSK_EXPORT Image {
public:
    Image(const Eigen::Vector2i &size, const std::vector<std::string> channels,
          uint8_t *data = nullptr);

    const float &operator()(int x, int y, int ch) const {
        x = std::clamp(x, 0, m_size.x() - 1);
        y = std::clamp(y, 0, m_size.y() - 1);
        return m_channels[ch].at({ x, y });
    }

    float &operator()(int x, int y, int ch) {
        x = std::clamp(x, 0, m_size.x() - 1);
        y = std::clamp(y, 0, m_size.y() - 1);
        return m_channels[ch].at({ x, y });
    }


    void write(const fs::path &path);

    void read(const fs::path &path);

private:
    std::vector<Channel> m_channels;
    Eigen::Vector2i m_size;
};

} // namespace misaki