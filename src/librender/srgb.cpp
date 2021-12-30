#include <misaki/render/srgb.h>
#include <misaki/core/logger.h>
#include <rgb2spec.h>
#include <tbb/tbb.h>

namespace misaki {

static RGB2Spec *model = nullptr;
static tbb::spin_mutex model_mutex;

Color3 srgb_model_fetch(const Color3 &c) {
    if (model == nullptr) {
        tbb::spin_mutex::scoped_lock sl(model_mutex);
        std::string fname = get_file_resolver()->resolve("data/srgb.coeff").string();
        Log(Info, "Loading spectral upsampling model \"data/srgb.coeff\" .. ");
        model = rgb2spec_load(fname.c_str());
        if (model == nullptr)
            Throw("Could not load sRGB-to-spectrum upsampling model "
                  "('data/srgb.coeff')");
        atexit([] { rgb2spec_free(model); });
    }

    float rgb[3] = { (float) c.r(), (float) c.g(), (float) c.b() };
    float out[3];
    rgb2spec_fetch(model, rgb, out);

    return Color3(out[0], out[1], out[2]);
}

}
