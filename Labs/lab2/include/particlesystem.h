#ifndef __PARTICLESYSTEM_H__
#define __PARTICLESYSTEM_H__

#include "util/color.h"
#include "util/rendering.h"
#include "util/vec2.h"
#include <vector>

#include "particle.h"
#include "force.h"
#include "emitter.h"

class ParticleSystem {
public:
    float elapsed = 0;  // seconds
    float maxParticles = 100;
    /* std::vector<Emitter> emitters;
     std::vector<Force> forces;*/
    std::vector<Particle> particles;

    void createParticles(Emitter &E, float dt);

    void update(std::vector<Force> forces, float dt);
    void render(std::vector<Force> forces, std::vector<Emitter> emitters);
};

#endif // __PARTICLESYSTEM_H__
