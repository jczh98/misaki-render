#pragma once

#include "math.h"
#include <Eigen/Dense>
#include <sstream>

namespace aspirin {

template <typename Float, int Size> struct Transform {
  using Matrix = Eigen::Matrix<Float, Size, Size>;
  using Affine = Eigen::Transform<Float, Size - 1, Eigen::Affine>;
  using Vector = Eigen::Matrix<Float, Size - 1, 1>;

  Matrix m_matrix = Matrix::Identity(), m_inverse_matrix = Matrix::Identity();
  Transform() {}
  Transform(const Matrix &m) : m_matrix(m), m_inverse_matrix(m.inverse()) {}
  Transform(const Matrix &m, const Matrix &inv_m)
      : m_matrix(m), m_inverse_matrix(inv_m) {}
  bool operator==(const Transform<Float, Size> &t) const {
    return m_matrix == t.m_matrix && m_inverse_matrix == t.m_inverse_matrix;
  }
  bool operator!=(const Transform<Float, Size> &t) const {
    return m_matrix != t.m_matrix || m_inverse_matrix != t.m_inverse_matrix;
  }
  Transform operator*(const Transform &t) const {
    return Transform(m_matrix * t.m_matrix,
                     t.m_inverse_matrix * m_inverse_matrix);
  }
  Matrix matrix() const { return m_matrix; }
  Matrix inverse_matrix() const { return m_inverse_matrix; }

  Transform inverse() const { return Transform(m_inverse_matrix, m_matrix); }

  Vector translation() const {
    return m_matrix.col(Size - 1).template head<Size - 1>();
  }

  Vector transform_affine_point(const Vector &p) const {
    Eigen::Matrix<Float, Size, 1> result;
    for (int i = 0; i < Size - 1; i++)
      result.coeffRef(i) = p.coeff(i);
    result.coeffRef(Size - 1) = 1.f;
    result = m_matrix * result;
    return result.template head<Size - 1>();
  }

  Vector apply_vector(const Vector &v) const {
    return m_matrix.template topLeftCorner<Size - 1, Size - 1>() *
           Vector::Base(v);
  }

  Vector apply_point(const Vector &p) const {
    Eigen::Matrix<Float, Size, 1> result;
    for (int i = 0; i < Size - 1; i++)
      result.coeffRef(i) = p.coeff(i);
    result.coeffRef(Size - 1) = 1.f;
    result = m_matrix * Eigen::Matrix<Float, Size, 1>::Base(result);
    return result.template head<Size - 1>() / result.coeff(Size - 1);
  }

  Vector apply_normal(const Vector &n) const {
    return m_inverse_matrix.template topLeftCorner<Size - 1, Size - 1>()
               .transpose() *
           Vector::Base(n);
  }

  Transform<Float, Size - 1> extract() const {
    Transform<Float, Size - 1> result;
    result.m_matrix = m_matrix.template topLeftCorner<Size - 1, Size - 1>();
    result.m_inverse_matrix =
        m_inverse_matrix.template topLeftCorner<Size - 1, Size - 1>();
    return result;
  }

  static Transform translate(const Vector &v) {
    Affine transform;
    transform.setIdentity();
    transform =
        Eigen::Translation<Float, Size - 1>(v.x(), v.y(), v.z()) * transform;
    return Transform(transform.matrix());
  }

  static Transform scale(const Vector &v) {
    Affine transform;
    transform.setIdentity();
    transform = Eigen::DiagonalMatrix<Float, Size - 1>(v) * transform;
    return Transform(transform.matrix());
  }

  template <int N = Size, std::enable_if_t<N == 4, int> = 0>
  static Transform rotate(const Vector &axis, const Float &angle) {
    Affine transform;
    transform.setIdentity();
    transform = Eigen::AngleAxis<Float>(angle, axis) * transform;
    return Transform(transform.matrix());
  }

  template <int N = Size, std::enable_if_t<N == 4, int> = 0>
  static Transform lookat(const Eigen::Matrix<Float, 3, 1> &origin,
                          const Eigen::Matrix<Float, 3, 1> &target,
                          const Eigen::Matrix<Float, 3, 1> &up) {
    auto dir = (target - origin).normalized();
    auto left = up.normalized().cross(dir).normalized();
    auto new_up = dir.cross(left).normalized();
    Eigen::Matrix4f result;
    result << left, new_up, dir, origin, 0, 0, 0, 1;
    return Transform(result);
  }

  template <int N = Size, std::enable_if_t<N == 4, int> = 0>
  static Transform perspective(Float fov, Float near_, Float far_) {
    float recip = 1.0f / (far_ - near_),
          cot = 1.0f / std::tan(math::deg_to_rag(fov / 2.0f));
    Eigen::Matrix4f perspective;
    perspective << cot, 0, 0, 0, 0, cot, 0, 0, 0, 0, far_ * recip,
        -near_ * far_ * recip, 0, 0, 1, 0;
    return Transform(perspective);
  }

  std::string to_string() const {
    std::ostringstream oss;
    oss << m_matrix.format(Eigen::IOFormat(4, 0, ", ", "\n", "", "", "[", "]"));
    return oss.str();
  }
};

template <typename Float, int Size>
std::ostream &operator<<(std::ostream &os, const Transform<Float, Size> &t) {
  os << t.matrix().format(Eigen::IOFormat(4, 0, ", ", "\n", "", "", "[", "]"));
  return os;
}

} // namespace nekoba