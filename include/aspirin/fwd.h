#pragma once

#include "array.h"
#include "bbox.h"
#include "distribution.h"
#include "frame.h"
#include "math_helper.h"
#include "platform.h"
#include "spectrum.h"
#include "transform.h"
#include <Eigen/Dense>
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

#define APR_IMPORT_CORE_TYPES(Float_)                                          \
    using Float           = Float_;                                            \
    using Vector2i        = Eigen::Vector2i;                                   \
    using Vector3i        = Eigen::Vector3i;                                   \
    using Vector4i        = Eigen::Vector4i;                                   \
    using Vector2u        = Eigen::Matrix<uint32_t, 2, 1>;                     \
    using Vector3u        = Eigen::Matrix<uint32_t, 3, 1>;                     \
    using Vector4u        = Eigen::Matrix<uint32_t, 4, 1>;                     \
    using Vector2         = Eigen::Matrix<Float, 2, 1>;                        \
    using Vector3         = Eigen::Matrix<Float, 3, 1>;                        \
    using Vector4         = Eigen::Matrix<Float, 4, 1>;                        \
    using Matrix3         = Eigen::Matrix<Float, 3, 3>;                        \
    using Matrix4         = Eigen::Matrix<Float, 4, 4>;                        \
    using Transform3      = Transform<Float, 3>;                               \
    using Transform4      = Transform<Float, 4>;                               \
    using BoundingBox3    = BoundingBox<Float, 3>;                             \
    using BoundingSphere3 = BoundingSphere<Float, 3>;                          \
    using Frame3          = Frame<Float>;                                      \
    using Distribution1D  = math::Distribution1D<Float>;                       \
    using Color3          = Color<Float, 3>;

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

} // namespace aspirin