#pragma once

#include "mathutils.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <sstream>

namespace misaki {

struct Transform3f {

    Eigen::Matrix3f m_matrix         = Eigen::Matrix3f::Identity(),
                    m_inverse_matrix = Eigen::Matrix3f::Identity();

    Transform3f() {}
    Transform3f(const Eigen::Matrix3f &m)
        : m_matrix(m), m_inverse_matrix(m.inverse()) {}
    Transform3f(const Eigen::Matrix3f &m, const Eigen::Matrix3f &inv_m)
        : m_matrix(m), m_inverse_matrix(inv_m) {}
    bool operator==(const Transform3f &t) const {
        return m_matrix == t.m_matrix && m_inverse_matrix == t.m_inverse_matrix;
    }
    bool operator!=(const Transform3f &t) const {
        return m_matrix != t.m_matrix || m_inverse_matrix != t.m_inverse_matrix;
    }
    Transform3f operator*(const Transform3f &t) const {
        return Transform3f(m_matrix * t.m_matrix,
                           t.m_inverse_matrix * m_inverse_matrix);
    }
    Eigen::Matrix3f matrix() const { return m_matrix; }
    Eigen::Matrix3f inverse_matrix() const { return m_inverse_matrix; }

    Transform3f inverse() const {
        return Transform3f(m_inverse_matrix, m_matrix);
    }

    Eigen::Vector2f translation() const {
        return m_matrix.col(2).template head<2>();
    }

    Eigen::Vector2f transform_affine_point(const Eigen::Vector2f &p) const {
        Eigen::Vector3f result;
        for (int i = 0; i < 2; i++)
            result.coeffRef(i) = p.coeff(i);
        result.coeffRef(2) = 1.f;
        result             = m_matrix * result;
        return result.template head<2>();
    }

    Eigen::Vector2f apply_vector(const Eigen::Vector2f &v) const {
        return m_matrix.template topLeftCorner<2, 2>() * v;
    }

    Eigen::Vector2f apply_point(const Eigen::Vector2f &p) const {
        Eigen::Vector3f result;
        for (int i = 0; i < 2; i++)
            result.coeffRef(i) = p.coeff(i);
        result.coeffRef(2) = 1.f;
        result             = m_matrix * result;
        return result.template head<2>() / result.coeff(2);
    }

    Eigen::Vector2f apply_normal(const Eigen::Vector2f &n) const {
        return m_inverse_matrix.template topLeftCorner<2, 2>().transpose() * n;
    }

    static Transform3f translate(const Eigen::Vector2f &v) {
        Eigen::Affine2f transform = Eigen::Affine2f::Identity();
        transform = Eigen::Translation2f(v.x(), v.y()) * transform;
        return Transform3f(transform.matrix());
    }

    static Transform3f scale(const Eigen::Vector2f &v) {
        Eigen::Affine2f transform = Eigen::Affine2f::Identity();
        transform = Eigen::DiagonalMatrix<float, 2>(v.x(), v.y()) * transform;
        return Transform3f(transform.matrix());
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << m_matrix.format(
            Eigen::IOFormat(4, 0, ", ", "\n", "", "", "[", "]"));
        return oss.str();
    }
};

struct Transform4f {
    Eigen::Matrix4f m_matrix         = Eigen::Matrix4f::Identity(),
                    m_inverse_matrix = Eigen::Matrix4f::Identity();
    Transform4f() {}
    Transform4f(const Eigen::Matrix4f &m)
        : m_matrix(m), m_inverse_matrix(m.inverse()) {}
    Transform4f(const Eigen::Matrix4f &m, const Eigen::Matrix4f &inv_m)
        : m_matrix(m), m_inverse_matrix(inv_m) {}
    bool operator==(const Transform4f &t) const {
        return m_matrix == t.m_matrix && m_inverse_matrix == t.m_inverse_matrix;
    }
    bool operator!=(const Transform4f &t) const {
        return m_matrix != t.m_matrix || m_inverse_matrix != t.m_inverse_matrix;
    }
    Transform4f operator*(const Transform4f &t) const {
        return Transform4f(m_matrix * t.m_matrix,
                           t.m_inverse_matrix * m_inverse_matrix);
    }
    Eigen::Matrix4f matrix() const { return m_matrix; }
    Eigen::Matrix4f inverse_matrix() const { return m_inverse_matrix; }

    Transform4f inverse() const {
        return Transform4f(m_inverse_matrix, m_matrix);
    }

    Eigen::Vector3f translation() const {
        return m_matrix.col(3).template head<3>();
    }

    Eigen::Vector3f transform_affine_point(const Eigen::Vector3f &p) const {
        Eigen::Vector4f result;
        for (int i = 0; i < 3; i++)
            result.coeffRef(i) = p.coeff(i);
        result.coeffRef(3) = 1.f;
        result             = m_matrix * result;
        return result.template head<3>();
    }

    Eigen::Vector3f apply_vector(const Eigen::Vector3f &v) const {
        return m_matrix.template topLeftCorner<3, 3>() * v;
    }

    Eigen::Vector3f apply_point(const Eigen::Vector3f &p) const {
        Eigen::Vector4f result;
        for (int i = 0; i < 3; i++)
            result.coeffRef(i) = p.coeff(i);
        result.coeffRef(3) = 1.f;
        result             = m_matrix * result;
        return result.template head<3>() / result.coeff(3);
    }

    Eigen::Vector3f apply_normal(const Eigen::Vector3f &n) const {
        return m_inverse_matrix.template topLeftCorner<3, 3>().transpose() * n;
    }

    Transform3f extract() const {
        Transform3f result;
        result.m_matrix = m_matrix.template topLeftCorner<3, 3>();
        result.m_inverse_matrix =
            m_inverse_matrix.template topLeftCorner<3, 3>();
        return result;
    }

    static Transform4f translate(const Eigen::Vector3f &v) {
        Eigen::Affine3f transform = Eigen::Affine3f::Identity();
        transform = Eigen::Translation3f(v.x(), v.y(), v.z()) * transform;
        return Transform4f(transform.matrix());
    }

    static Transform4f scale(const Eigen::Vector3f &v) {
        Eigen::Affine3f transform = Eigen::Affine3f::Identity();
        transform =
            Eigen::DiagonalMatrix<float, 3>(v.x(), v.y(), v.z()) * transform;
        return Transform4f(transform.matrix());
    }

    static Transform4f rotate(const Eigen::Vector3f &axis, const float &angle) {
        Eigen::Affine3f transform = Eigen::Affine3f::Identity();
        transform                 = Eigen::AngleAxisf(angle, axis) * transform;
        return Transform4f(transform.matrix());
    }

    static Transform4f lookat(const Eigen::Vector3f &origin,
                              const Eigen::Vector3f &target,
                              const Eigen::Vector3f &up) {
        auto dir    = (target - origin).normalized();
        auto left   = up.normalized().cross(dir).normalized();
        auto new_up = dir.cross(left).normalized();
        Eigen::Matrix4f result;
        result << left, new_up, dir, origin, 0, 0, 0, 1;
        return Transform4f(result);
    }

    static Transform4f perspective(float fov, float near_, float far_) {
        float recip = 1.0f / (far_ - near_),
              cot   = 1.0f / std::tan(math::deg_to_rag(fov / 2.0f));
        Eigen::Matrix4f perspective;
        perspective << cot, 0, 0, 0, 0, cot, 0, 0, 0, 0, far_ * recip,
            -near_ * far_ * recip, 0, 0, 1, 0;
        return Transform4f(perspective);
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << m_matrix.format(
            Eigen::IOFormat(4, 0, ", ", "\n", "", "", "[", "]"));
        return oss.str();
    }
};

MSK_INLINE std::ostream &operator<<(std::ostream &os, const Transform3f &t) {
    os << t.matrix().format(
        Eigen::IOFormat(4, 0, ", ", "\n", "", "", "[", "]"));
    return os;
}

MSK_INLINE std::ostream &operator<<(std::ostream &os, const Transform4f &t) {
    os << t.matrix().format(
        Eigen::IOFormat(4, 0, ", ", "\n", "", "", "[", "]"));
    return os;
}

} // namespace misaki