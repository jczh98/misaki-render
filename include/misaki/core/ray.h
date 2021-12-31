#pragma once

#include "fwd.h"

#include <iostream>

namespace misaki {

struct Ray {
    Eigen::Vector3f o, d;
    float mint = math::RayEpsilon<float>;
    float maxt = math::Infinity<float>;
    float time = 0.f;
    Wavelength wavelengths;
    Eigen::Vector3f d_rcp;

    Ray() {
    }

    Ray(const Eigen::Vector3f &o, const Eigen::Vector3f &d, float time, const Wavelength &wavelengths)
        : o(o), d(d), time(time), wavelengths(wavelengths) {
        update();
    }

    Ray(const Eigen::Vector3f &o, const Eigen::Vector3f &d, float mint,
        float maxt, float time, const Wavelength &wavelengths)
        : o(o), d(d), mint(mint), maxt(maxt), time(time), wavelengths(wavelengths) {
        update();
    }

    Ray(const Ray &ray, float mint, float maxt, float time)
        : o(ray.o), d(ray.d), wavelengths(ray.wavelengths), mint(mint), maxt(maxt), time(time) {
        update();
    }

    Ray(const Ray &ray)
        : o(ray.o), d(ray.d), mint(ray.mint), maxt(ray.maxt),
          time(ray.time), wavelengths(ray.wavelengths) {
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
        oss << "  wavelengths = " << wavelengths << std::endl;
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

    RayDifferential() {
    }

    RayDifferential(const Ray &ray)
        : Ray(ray), has_differentials(false) {
    }
};

} // namespace misaki
