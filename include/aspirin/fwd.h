#pragma once

#include "bbox.h"
#include "diagnostic.h"
#include "distribution.h"
#include "frame.h"
#include "fresolver.h"
#include "math_helper.h"
#include "platform.h"
#include "spectrum.h"
#include "transform.h"
#include <Eigen/Core>
#include <filesystem>
#include <mutex>
#include <set>
#include <tuple>
#include <type_traits>

namespace aspirin {

class Object;
class Class;
template <typename> class ref;

class Properties;
class Formatter;
class Logger;

template <typename Float_> struct CoreAliases {
    using Float           = Float_;
    using Vector2i        = Eigen::Vector2i;
    using Vector3i        = Eigen::Vector3i;
    using Vector4i        = Eigen::Vector4i;
    using Vector2u        = Eigen::Matrix<uint32_t, 2, 1>;
    using Vector3u        = Eigen::Matrix<uint32_t, 3, 1>;
    using Vector4u        = Eigen::Matrix<uint32_t, 4, 1>;
    using Vector2         = Eigen::Matrix<Float, 2, 1>;
    using Vector3         = Eigen::Matrix<Float, 3, 1>;
    using Vector4         = Eigen::Matrix<Float, 4, 1>;
    using Matrix3         = Eigen::Matrix<Float, 3, 3>;
    using Matrix4         = Eigen::Matrix<Float, 4, 4>;
    using Transform3      = Transform<Float, 3>;
    using Transform4      = Transform<Float, 4>;
    using BoundingBox3    = BoundingBox<Float, 3>;
    using BoundingSphere3 = BoundingSphere<Float, 3>;
    using Frame3          = Frame<Float>;
    using Distribution1D  = math::Distribution1D<Float>;
    using Color3          = Color<Float, 3>;
    using Color4          = Color<Float, 4>;
};

#define APR_IMPORT_CORE_TYPES(Float_)                                          \
    using CoreAliases     = aspirin::CoreAliases<Float_>;                      \
    using Vector2i        = typename CoreAliases::Vector2i;                    \
    using Vector3i        = typename CoreAliases::Vector3i;                    \
    using Vector4i        = typename CoreAliases::Vector4i;                    \
    using Vector2u        = typename CoreAliases::Vector2u;                    \
    using Vector3u        = typename CoreAliases::Vector3u;                    \
    using Vector4u        = typename CoreAliases::Vector4u;                    \
    using Vector2         = typename CoreAliases::Vector2;                     \
    using Vector3         = typename CoreAliases::Vector3;                     \
    using Vector4         = typename CoreAliases::Vector4;                     \
    using Matrix3         = typename CoreAliases::Matrix3;                     \
    using Matrix4         = typename CoreAliases::Matrix4;                     \
    using Transform3      = typename CoreAliases::Transform3;                  \
    using Transform4      = typename CoreAliases::Transform4;                  \
    using BoundingBox3    = typename CoreAliases::BoundingBox3;                \
    using BoundingSphere3 = typename CoreAliases::BoundingSphere3;             \
    using Frame3          = typename CoreAliases::Frame3;                      \
    using Distribution1D  = typename CoreAliases::Distribution1D;              \
    using Color3          = typename CoreAliases::Color3;                      \
    using Color4          = typename CoreAliases::Color4;

namespace fs = std::filesystem;

struct BSDFContext;
template <typename Float, typename Spectrum> class BSDF;
template <typename Float, typename Spectrum> class Emitter;
template <typename Float, typename Spectrum> class Endpoint;
template <typename Float, typename Spectrum> class Film;
template <typename Float, typename Spectrum> class ImageBlock;
template <typename Float, typename Spectrum> class Integrator;
template <typename Float, typename Spectrum> class Medium;
template <typename Float, typename Spectrum> class Mesh;
template <typename Float, typename Spectrum> class ReconstructionFilter;
template <typename Float, typename Spectrum> class Sampler;
template <typename Float, typename Spectrum> class Scene;
template <typename Float, typename Spectrum> class Sensor;
template <typename Float, typename Spectrum> class PhaseFunction;
template <typename Float, typename Spectrum> class ProjectiveCamera;
template <typename Float, typename Spectrum> class Shape;
template <typename Float, typename Spectrum> class Texture;
template <typename Float, typename Spectrum> class Volume;

template <typename Float, typename Spectrum> struct DirectionSample;
template <typename Float, typename Spectrum> struct PositionSample;
template <typename Float, typename Spectrum> struct Interaction;
template <typename Float, typename Spectrum> struct PreliminaryIntersection;
template <typename Float, typename Spectrum> struct SurfaceInteraction;
template <typename Float, typename Spectrum> struct MediumInteraction;
template <typename Float, typename Spectrum> struct BSDFSample;
template <typename Float, typename Spectrum> struct PhaseFunctionContext;

#define MTS_IMPORT_TYPES()                                                     \
    APR_IMPORT_CORE_TYPES(Float)                                               \
    using Scene      = aspirin::Scene<Float, Spectrum>;                        \
    using Sampler    = aspirin::Sampler<Float, Spectrum>;                      \
    using Shape      = aspirin::Shape<Float, Spectrum>;                        \
    using Mesh       = aspirin::Mesh<Float, Spectrum>;                         \
    using Integrator = aspirin::Integrator<Float, Spectrum>;                   \
    using BSDF       = aspirin::BSDF<Float, Spectrum>;                         \
    using Emitter    = aspirin::Emitter<Float, Spectrum>;

extern APR_EXPORT FileResolver *get_file_resolver();

} // namespace aspirin