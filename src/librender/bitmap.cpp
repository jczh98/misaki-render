#include <misaki/bitmap.h>
#include <iostream>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace misaki {

void Bitmap::write(const std::string &filename) { write_png(filename); }

void Bitmap::write_png(const std::string &filename) {
    std::string path = filename;

    auto to_srgb = [&](Color3f value) {
        Color3f ret;
        for (int i = 0; i < 3; i++) {
            if (value.coeff(i) <= 0.0031308f)
                ret.coeffRef(i) = 12.92f * value.coeff(i);
            else
                ret.coeffRef(i) =
                    (1.0f + 0.055f) * std::pow(value.coeff(i), 1.0f / 2.4f) -
                    0.055f;
        }
        return ret;
    };
    auto *rgb8   = new uint8_t[3 * cols() * rows()];
    uint8_t *dst = rgb8;
    for (int i = 0; i < rows(); ++i) {
        for (int j = 0; j < cols(); ++j) {
            Color3f tonemapped = to_srgb(coeffRef(i, j));
            dst[0] = (uint8_t) std::clamp(255.f * tonemapped[0], 0.f, 255.f);
            dst[1] = (uint8_t) std::clamp(255.f * tonemapped[1], 0.f, 255.f);
            dst[2] = (uint8_t) std::clamp(255.f * tonemapped[2], 0.f, 255.f);
            dst += 3;
        }
    }

    int ret = stbi_write_png(path.c_str(), (int) cols(), (int) rows(), 3, rgb8,
                             3 * (int) cols());
    if (ret == 0) {
        std::cout << "Bitmap::savePNG(): Could not save PNG file \"" << path
                  << "%s\"" << std::endl;
    }

    delete[] rgb8;
}

} // namespace misaki