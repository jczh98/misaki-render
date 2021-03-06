#pragma once

#include "fwd.h"

namespace aspirin {

class Bitmap : public Eigen::Array<Color<float, 3>, Eigen::Dynamic,
                                   Eigen::Dynamic, Eigen::RowMajor> {
public:
    using Base = Eigen::Array<Color<float, 3>, Eigen::Dynamic, Eigen::Dynamic,
                              Eigen::RowMajor>;
    using Color3f = Color<float, 3>;
    using Vector2i = Eigen::Matrix<int, 2, 1>;

    Bitmap(const Vector2i &size = Vector2i(0, 0)) : Base(size.y(), size.x()) {}

    void write(const std::string &filename);

protected:
    void write_png(const std::string &filename);
};

} // namespace aspirin