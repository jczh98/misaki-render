#pragma once

#include "fwd.h"

namespace misaki {

struct Ray {
    Eigen::Vector3f o, d;
    float mint = math::RayEpsilon<float>;
    float maxt = math::Infinity<float>;
    float time = 0.f;
    Wavelength wavelengths; 
    Eigen::Vector3f d_rcp;

    Ray() {}

    Ray(const Eigen::Vector3f &o, const Eigen::Vector3f &d, float time)
        : o(o), d(d), time(time) {
        update();
    }

    Ray(const Eigen::Vector3f &o, const Eigen::Vector3f &d, float mint,
        float maxt, float time)
        : o(o), d(d), mint(mint), maxt(maxt), time(time) {
        update();
    }

    Ray(const Ray &ray, float mint, float maxt, float time)
        : o(ray.o), d(ray.d), mint(mint), maxt(maxt), time(time) {
        update();
    }

    Eigen::Vector3f operator()(float t) const { return o + d * t; }

    void update() { d_rcp = d.cwiseInverse(); }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "Ray[" << std::endl;
        oss << "  o = " << string::indent(o, 6) << "," << std::endl;
        oss << "  d = " << string::indent(d, 6) << "," << std::endl;
        oss << "  mint = " << mint << "," << std::endl;
        oss << "  maxt = " << maxt << "," << std::endl;
        oss << "  time = " << time << std::endl;
        oss << "]";
        return oss.str();
    }
};

struct RayDifferential : public Ray {
    Eigen::Vector3f o_x, o_y, d_x, d_y;
    bool has_differentials = false;

    void scale_differential(float amount) {
        o_x = (o_x - this->o) * amount + this->o;
        o_y = (o_y - this->o) * amount + this->o;
        d_x = (d_x - this->d) * amount + this->d;
        d_y = (d_y - this->d) * amount + this->d;
    }
    RayDifferential() {}
    RayDifferential(const Ray &ray) : Ray(ray), has_differentials(false) {}
};

namespace ray {

inline Ray spawn(const Ray &ray, float mint, float maxt) {
    return Ray(ray.o, ray.d, mint, maxt, 0.f);
}

template <bool Offset = true>
inline Ray spawn(const Eigen::Vector3f &p, const Eigen::Vector3f &d) {
    if constexpr (Offset) {

        return Ray(p, d,
                   (1.f + p.cwiseAbs().maxCoeff()) * math::RayEpsilon<float>,
                   math::Infinity<float>, 0.f);
    } else {
        return Ray(p, d, 0, math::Infinity<float>, 0.f);
    }
}

} // namespace ray

} // namespace misaki