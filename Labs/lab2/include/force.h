#ifndef __FORCE_H__
#define __FORCE_H__

#include "util/color.h"
#include "util/rendering.h"
#include "util/vec2.h"
#include <vector>

class Force {
    public:
        vec2 position = {0, 0};
        float force = (float)0.001; 
};

class BlackHole : public Force {
    public:
        vec2 position = {0, 0};
        float force = (float)0.01;
};

class DistantStar : public Force {
    public:
        vec2 position = {0, 0};
        float force = (float)0.0000001;
};

#endif