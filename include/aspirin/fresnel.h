#pragma once

#include "fwd.h"

namespace aspirin {

inline Float schlick_weight(Float cos_theta) {
  return math::power<5>(std::clamp<float>(1 - cos_theta, 0, 1)); 
}

Vector3 reflect(const Vector3 &wi) {
  return Vector3(-wi.x(), -wi.y(), wi.z());
}

Vector3 reflect(const Vector3 &wi, const Vector3 &n) {
  return Vector3(n) * 2.f * math::dot(wi, n) - wi;
}

Vector3 refract(const Vector3 &wi, Float cos_theta_t, Float eta_ti) {
  return Vector3(-eta_ti * wi.x(), -eta_ti * wi.y(), cos_theta_t);
}

Vector3 refract(const Vector3 &wi, const Vector3 &m, Float cos_theta_t, Float eta_ti) {
  return m * (math::dot(wi, m) * eta_ti + cos_theta_t) - wi * eta_ti;
}

// return F(fresnel), cos_theta_t, eta_it, eta_ti
std::tuple<Float, Float, Float, Float> fresnel(Float cos_theta_i, Float eta) {
  Float eta_it, eta_ti;
  if (cos_theta_i >= 0.f) {
    eta_it = eta;
    eta_ti = 1.f / eta;
  } else {
    eta_it = 1.f / eta;
    eta_ti = eta;
  }
  Float cos_theta_t_sqr = 1.f - eta_ti * eta_ti * (1.f - cos_theta_i * cos_theta_i);
  Float cos_theta_i_abs = std::abs(cos_theta_i);
  Float cos_theta_t_abs = math::safe_sqrt(cos_theta_t_sqr);

  Float a_s = (cos_theta_i_abs - eta_it * cos_theta_t_abs) / (cos_theta_i_abs + eta_it * cos_theta_t_abs);
  Float a_p = (cos_theta_t_abs - eta_it * cos_theta_i_abs) / (cos_theta_t_abs + eta_it * cos_theta_i_abs);
  Float r;
  if (eta == 1.f || cos_theta_i_abs == 0.f)
    r = eta == 1.f ? 0.f : 1.f;
  else
    r = 0.5f * (a_s * a_s + a_p * a_p);
  Float cos_theta_t = cos_theta_t_abs * std::copysign(1.f, -cos_theta_i);
  return {r, cos_theta_t, eta_it, eta_ti};
}

Color3 fresnel_conductor(Float cos_theta_i, const Color3 &eta, const Color3 &k) {
  Float cos_theta_i_2 = cos_theta_i * cos_theta_i,
        sin_theta_i_2 = 1.f - cos_theta_i_2,
        sin_theta_i_4 = sin_theta_i_2 * sin_theta_i_2;

  Color3 eta_r = eta,
           eta_i = k;

  Color3 temp_1 = eta_r * eta_r - eta_i * eta_i - sin_theta_i_2,
           a_2_pb_2 = (temp_1 * temp_1 + 4.f * eta_i * eta_i * eta_r * eta_r).sqrt(),
           a = (.5f * (a_2_pb_2 + temp_1)).sqrt();

  Color3 term_1 = a_2_pb_2 + cos_theta_i_2,
           term_2 = 2.f * cos_theta_i * a;

  Color3 r_s = (term_1 - term_2) / (term_1 + term_2);

  Color3 term_3 = a_2_pb_2 * cos_theta_i_2 + sin_theta_i_4,
           term_4 = term_2 * sin_theta_i_2;

  Color3 r_p = r_s * (term_3 - term_4) / (term_3 + term_4);

  return .5f * (r_s + r_p);
}

// Computes the diffuse unpolarized Fresnel reflectance of a dielectric material (sometimes referred to as "Fdr").
// https://github.com/mitsuba-renderer/mitsuba2/blob/master/include/mitsuba/render/fresnel.h
Float fresnel_diffuse_reflectance(Float eta) {
  /* Fast mode: the following code approximates the diffuse Frensel reflectance
     for the eta<1 and eta>1 cases. An evaluation of the accuracy led to the
     following scheme, which cherry-picks fits from two papers where they are
     best. */
  Float result(0.f);
  bool eta_l_1 = (eta < 1.f);
  /* Fit by Egan and Hilgeman (1973). Works reasonably well for
     "normal" IOR values (<2).
     Max rel. error in 1.0 - 1.5 : 0.1%
     Max rel. error in 1.5 - 2   : 0.6%
     Max rel. error in 2.0 - 5   : 9.5%
  */
  if (eta_l_1) {
    result = -1.4399f * (eta * eta) + 0.7099f * eta + 0.6681f + 0.0636f / eta;
    return result;
  }
  /* Fit by d'Eon and Irving (2011)
     Maintains a good accuracy even for unrealistic IOR values.
     Max rel. error in 1.0 - 2.0   : 0.1%
     Max rel. error in 2.0 - 10.0  : 0.2%  */
  Float inv_eta = 1.f / eta,
        inv_eta_2 = inv_eta * inv_eta,
        inv_eta_3 = inv_eta_2 * inv_eta,
        inv_eta_4 = inv_eta_3 * inv_eta,
        inv_eta_5 = inv_eta_4 * inv_eta;
  if (!eta_l_1) {
    result = 0.919317f - 3.4793f * inv_eta + 6.75335f * inv_eta_2 - 7.80989f * inv_eta_3 + 4.98554f * inv_eta_4 - 1.36881f * inv_eta_5;
  }

  return result;
}

}  // namespace aspirin