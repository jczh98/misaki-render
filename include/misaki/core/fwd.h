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

using Distribution1D = math::Distribution1D<float>;
using Color3         = Color<float, 3>;
using Color4         = Color<float, 4>;
using Spectrum       = Color3;

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
struct SceneInteraction;
struct MediumInteraction;
struct BSDFSample;
struct PhaseFunctionContext;

extern MSK_EXPORT FileResolver *get_file_resolver();

} // namespace misaki