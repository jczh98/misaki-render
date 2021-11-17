#pragma once

#include <Eigen/Core>

namespace misaki {

struct BoundingSphere2f {
    Eigen::Vector2f center;
    float radius;
    BoundingSphere2f()
        : center(Eigen::Vector2f::Constant(0.f)), radius(0.f) {}
    BoundingSphere2f(const Eigen::Vector2f &center, const float &radius)
        : center(center), radius(radius) {}

    bool operator==(const BoundingSphere2f &bsphere) const {
        return center == bsphere.center && radius == bsphere.radius;
    }

    bool operator!=(const BoundingSphere2f &bsphere) const {
        return center != bsphere.center || radius != bsphere.radius;
    }

    bool empty() const { return radius <= 0.f; }
    void expand(const Eigen::Vector2f &p) {
        radius = std::max<float>(radius, (p - center).norm());
    }
    bool contains(const Eigen::Vector2f &p, bool strict = false) const {
        if (strict)
            return (p - center).squaredNorm() < radius * radius;
        else
            return (p - center).squaredNorm() <= radius * radius;
    }
};

struct BoundingSphere3f {
    Eigen::Vector3f center;
    float radius;
    BoundingSphere3f()
        : center(Eigen::Vector3f::Constant(0.f)), radius(0.f) {}
    BoundingSphere3f(const Eigen::Vector3f &center, const float &radius)
        : center(center), radius(radius) {}

    bool operator==(const BoundingSphere3f &bsphere) const {
        return center == bsphere.center && radius == bsphere.radius;
    }

    bool operator!=(const BoundingSphere3f &bsphere) const {
        return center != bsphere.center || radius != bsphere.radius;
    }

    bool empty() const { return radius <= 0.f; }
    void expand(const Eigen::Vector3f &p) {
        radius = std::max<float>(radius, (p - center).norm());
    }
    bool contains(const Eigen::Vector3f &p, bool strict = false) const {
        if (strict)
            return (p - center).squaredNorm() < radius * radius;
        else
            return (p - center).squaredNorm() <= radius * radius;
    }
};

} // namespace misaki