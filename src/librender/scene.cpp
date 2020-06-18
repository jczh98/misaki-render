#include <misaki/render/properties.h>
#include <misaki/render/scene.h>
#include <misaki/render/logger.h>

namespace misaki::render {

Scene::Scene(const Properties &props) {
	for (auto& kv : props.components()) {
		auto camera = std::dynamic_pointer_cast<Camera>(kv.second);
		auto integrator = std::dynamic_pointer_cast<Integrator>(kv.second);
		if (camera) {
			if (m_camera) Throw("Can only have one camera.");
			m_camera = camera;
		} else if (integrator) {
			if (m_integrator) Throw("Can only have one integrator.");
			m_integrator = integrator;
		}
	}
	if (!m_integrator) {
		Log(Warn, "No integrator found! Instantiating a path tracer..");
		m_integrator = PluginManager::instance()->create_comp<Integrator>(Properties("path"));
	}
}

MSK_REGISTER_CLASS(Scene)

}  // namespace misaki::render