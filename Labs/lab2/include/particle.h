#include "util/color.h"
#include "util/rendering.h"
#include "util/vec2.h"
#include <vector>

#pragma once
class Particle {
	public: 
        vec2 position = {0, 0};
        vec2 velocity = {0, 0};
        vec2 acceleration = {0, 0};
        float lifetime = (float)60; // in s
		float radius = (float)1;
};
