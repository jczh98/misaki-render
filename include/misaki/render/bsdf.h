#pragma once

#include "interaction.h"

namespace misaki::render {

enum class TransportMode : uint32_t {
	Radiance, // Camera to light
	Importance, // Light to camera
	TransportMode = 2
}

}

