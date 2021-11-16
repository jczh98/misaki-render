#pragma once

#include "bsphere.h"
#include <limits>

namespace misaki {


struct BoundingBox2f {
    BoundingBox2f() { reset(); }
    BoundingBox2f(const Eigen::Vector2f &p) {
        this->pmin = p;
        this->pmax = p;
    }
    BoundingBox2f(const Eigen::Vector2f &pmin,
                  const Eigen::Vector2f &pmax) {
        this->pmin = pmin;
        this->pmax = pmax;
    }

    void reset() {
        pmin.setConstant(std::numeric_limits<float>::infinity());
        pmax.setConstant(-std::numeric_limits<float>::infinity());
    }

    void clip(const BoundingBox2f &bbox) {
        pmin = pmin.cwiseMax(bbox.pmin);
        pmax = pmax.cwiseMin(bbox.pmax);
    }

    void expand(const Eigen::Vector2f &p) {
        pmin = pmin.cwiseMin(p);
        pmax = pmax.cwiseMax(p);
    }

    Eigen::Vector2f offset(const Eigen::Vector2f &p) const {
        Eigen::Vector2f o = p - pmin;
        if (pmax.x() > pmin.x())
            o.x() /= pmax.x() - pmin.x();
        if (pmax.y() > pmin.y())
            o.y() /= pmax.y() - pmin.y();
        return o;
    }

    void expand(const BoundingBox2f &bbox) {
        pmin = pmin.cwiseMin(bbox.pmin);
        pmax = pmax.cwiseMax(bbox.pmax);
    }

    Eigen::Vector2f center() const { return (pmin + pmax) * float(.5f); }

    Eigen::Vector2f diagonal() const { return pmax - pmin; }

    BoundingSphere2f bounding_sphere() const {
        Eigen::Vector2f c = center();
        return { c, (c - pmax).norm() };
    }

    Eigen::Vector2f pmin, pmax;
};

struct BoundingBox3f {
    BoundingBox3f() { reset(); }
    BoundingBox3f(const Eigen::Vector3f &p) {
        this->pmin = p;
        this->pmax = p;
    }
    BoundingBox3f(const Eigen::Vector3f &pmin,
                  const Eigen::Vector3f &pmax) {
        this->pmin = pmin;
        this->pmax = pmax;
    }

    void reset() {
        pmin.setConstant(std::numeric_limits<float>::infinity());
        pmax.setConstant(-std::numeric_limits<float>::infinity());
    }

    void clip(const BoundingBox3f &bbox) {
        pmin = pmin.cwiseMax(bbox.pmin);
        pmax = pmax.cwiseMin(bbox.pmax);
    }

    void expand(const Eigen::Vector3f &p) {
        pmin = pmin.cwiseMin(p);
        pmax = pmax.cwiseMax(p);
    }

    Eigen::Vector3f offset(const Eigen::Vector3f &p) const {
        Eigen::Vector3f o = p - pmin;
        if (pmax.x() > pmin.x())
            o.x() /= pmax.x() - pmin.x();
        if (pmax.y() > pmin.y())
            o.y() /= pmax.y() - pmin.y();
        if (pmax.z() > pmin.z())
            o.z() /= pmax.z() - pmin.z();
        return o;
    }

    void expand(const BoundingBox3f &bbox) {
        pmin = pmin.cwiseMin(bbox.pmin);
        pmax = pmax.cwiseMax(bbox.pmax);
    }

    Eigen::Vector3f center() const { return (pmin + pmax) * float(.5f); }

    Eigen::Vector3f diagonal() const { return pmax - pmin; }

    BoundingSphere3f bounding_sphere() const {
        Eigen::Vector3f c = center();
        return { c, (c - pmax).norm() };
    }

    Eigen::Vector3f pmin, pmax;
};

} // namespace misaki