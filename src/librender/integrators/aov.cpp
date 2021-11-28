#include <fstream>
#include <misaki/core/logger.h>
#include <misaki/core/manager.h>
#include <misaki/core/properties.h>
#include <misaki/core/utils.h>
#include <misaki/render/bsdf.h>
#include <misaki/render/emitter.h>
#include <misaki/render/film.h>
#include <misaki/render/integrator.h>
#include <misaki/render/interaction.h>
#include <misaki/render/mesh.h>
#include <misaki/render/records.h>
#include <misaki/render/scene.h>
#include <misaki/render/sensor.h>
#include <tbb/parallel_for.h>

namespace misaki {

class AOVIntegrator final : public MonteCarloIntegrator {
public:
    enum class Type {
        Depth,
        Position,
        UV,
        GeometricNormal,
        ShadingNormal,
        IntegratorRGBA
    };
    AOVIntegrator(const Properties &props) : MonteCarloIntegrator(props) {
        std::vector<std::string> tokens =
            string::tokenize(props.string("aovs"));

        for (const std::string &token : tokens) {
            std::vector<std::string> item = string::tokenize(token, ":");

            if (item.size() != 2 || item[0].empty() || item[1].empty())
                Log(Warn,
                    "Invalid AOV specification: require <name>:<type> pair");

            if (item[1] == "depth") {
                m_aov_types.push_back(Type::Depth);
                m_aov_names.push_back(item[0]);
            } else if (item[1] == "position") {
                m_aov_types.push_back(Type::Position);
                m_aov_names.push_back(item[0] + ".X");
                m_aov_names.push_back(item[0] + ".Y");
                m_aov_names.push_back(item[0] + ".Z");
            } else if (item[1] == "uv") {
                m_aov_types.push_back(Type::UV);
                m_aov_names.push_back(item[0] + ".U");
                m_aov_names.push_back(item[0] + ".V");
            } else if (item[1] == "geo_normal") {
                m_aov_types.push_back(Type::GeometricNormal);
                m_aov_names.push_back(item[0] + ".X");
                m_aov_names.push_back(item[0] + ".Y");
                m_aov_names.push_back(item[0] + ".Z");
            } else if (item[1] == "sh_normal") {
                m_aov_types.push_back(Type::ShadingNormal);
                m_aov_names.push_back(item[0] + ".X");
                m_aov_names.push_back(item[0] + ".Y");
                m_aov_names.push_back(item[0] + ".Z");
            } else {
                Throw("Invalid AOV type \"{}\"!", item[1]);
            }
        }

        for (auto &kv : props.objects()) {
            SamplingIntegrator *integrator =
                dynamic_cast<SamplingIntegrator *>(kv.second.get());
            if (!integrator)
                Throw("Child objects must be of type 'SamplingIntegrator'!");
            m_aov_types.push_back(Type::IntegratorRGBA);
            std::vector<std::string> aovs = integrator->aov_names();
            for (auto name : aovs)
                m_aov_names.push_back(kv.first + "." + name);
            m_integrators.push_back({ integrator, aovs.size() });
            m_aov_names.push_back(kv.first + ".R");
            m_aov_names.push_back(kv.first + ".G");
            m_aov_names.push_back(kv.first + ".B");
            m_aov_names.push_back(kv.first + ".A");
        }

        if (m_aov_names.empty())
            Log(Warn, "No AOVs were specified!");
    }

    virtual Spectrum sample(const Scene *scene, Sampler *sampler,
                            const RayDifferential &ray, const Medium *medium,
                            float *aovs) const override {
        SceneInteraction si = scene->ray_intersect(ray);

        Spectrum result;
        size_t ctr = 0;

        for (int i = 0; i < m_aov_types.size(); i++) {
            switch (m_aov_types[i]) {
                case Type::Depth:
                    *aovs++ = si.t == math::Infinity<float> ? 0.f : si.t;
                    break;

                case Type::Position:
                    *aovs++ = si.p.x();
                    *aovs++ = si.p.y();
                    *aovs++ = si.p.z();
                    break;

                case Type::UV:
                    *aovs++ = si.uv.x();
                    *aovs++ = si.uv.y();
                    break;

                case Type::GeometricNormal:
                    *aovs++ = si.n.x();
                    *aovs++ = si.n.y();
                    *aovs++ = si.n.z();
                    break;

                case Type::ShadingNormal:
                    *aovs++ = si.sh_frame.n.x();
                    *aovs++ = si.sh_frame.n.y();
                    *aovs++ = si.sh_frame.n.z();
                    break;

                case Type::IntegratorRGBA: {
                    Spectrum rgb = m_integrators[ctr].first->sample(
                        scene, sampler, ray, medium, aovs);
                    aovs += m_integrators[ctr].second;

                    *aovs++ = rgb.r();
                    *aovs++ = rgb.g();
                    *aovs++ = rgb.b();
                    *aovs++ = 1.f;

                    if (ctr == 0)
                        result = rgb;

                    ctr++;
                } break;
            }
        }
        return result;
    }

    std::vector<std::string> aov_names() const override { return m_aov_names; }

    MSK_DECLARE_CLASS()
private:
    std::vector<Type> m_aov_types;
    std::vector<std::string> m_aov_names;
    std::vector<std::pair<ref<SamplingIntegrator>, size_t>> m_integrators;
};

MSK_IMPLEMENT_CLASS(AOVIntegrator, MonteCarloIntegrator)
MSK_REGISTER_INSTANCE(AOVIntegrator, "aov")

} // namespace misaki