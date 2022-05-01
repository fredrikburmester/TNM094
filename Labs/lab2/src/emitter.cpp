#include "Emitter.h"
#include "Tracy.hpp"
#include <cmath>
#include <random>
#include <utility>
#include "Particle.h"

#include "Windows.h"


namespace {
constexpr float Pi = 3.141592654f;
const float Tau = 2.f * Pi;
}  // namespace

//void Emitter::addParticle(ParticleSystem psys) {
//    while(true) {
//        vec2 s = {0, 0};
//        psys.createParticle(this->position, this->spread, s, 1.0);
//        Sleep(1);
//    }
//}