#pragma once

#include "bbox.h"
#include "diagnostic.h"
#include "distribution.h"
#include "frame.h"
#include "fresolver.h"
#include "mathutils.h"
#include "platform.h"
#include "spectrum.h"
#include "transform.h"
#include <Eigen/Core>
#include <filesystem>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>
#include <type_traits>

namespace misaki {

class Object;
class Class;
template <typename> class ref;

class Properties;
class Formatter;
class Logger;

using Float           = float;
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
using Spectrum        = Color3;

namespace fs = std::filesystem;

struct BSDFContext;
class BSDF;
class Emitter;
class Endpoint;
class Film;
class ImageBlock;
class Integrator;
class Medium;
class Mesh;
class ReconstructionFilter;
class Sampler;
class Scene;
class Sensor;
class PhaseFunction;
class ProjectiveCamera;
class Shape;
class Texture;
class Volume;

struct Ray;
struct RayDifferential;
struct DirectionSample;
struct PositionSample;
struct Interaction;
struct PreliminaryIntersection;
struct SurfaceInteraction;
struct MediumInteraction;
struct BSDFSample;
struct PhaseFunctionContext;

extern MSK_EXPORT FileResolver *get_file_resolver();

} // namespace misaki