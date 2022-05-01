#include "particlesystem.h"
#include "particle.h"
#include "force.h"
#include "emitter.h"

#include "Windows.h"
#include "Tracy.hpp"

#include <stdlib.h> 
#include <vector>
#include <iostream>
#include <list>
#include <cmath>
#include <random>

using namespace std;

namespace {
    constexpr float Pi = 3.141592654f;
    const float Tau = 2.f * Pi;
} // namespace

void ParticleSystem::createParticles(Emitter &E, float dt) {
    float size = (float)particles.size();
   
    if (size >= maxParticles) {
        /*particles.erase(particles.begin(), particles.end()-maxParticles);*/
    } else {
        E.elapsed = E.elapsed + dt;

        if (E.elapsed >= (1 / E.pps)) {
            int amount = (int)(E.elapsed / (1 / E.pps));
            while (amount) {
                Particle p;
                p.position = E.position;

                double s1 = (rand() % (double)100 * ((double)E.spread * (double)2)) / (double)100;
                s1 = s1 - E.spread;

                float x = cos(E.angle + (float)s1);
                float y = sin(E.angle + (float)s1);

                p.velocity = {x * E.velocity, y * E.velocity};

                p.radius = E.particleSize;
                particles.push_back(p);
                amount = amount - 1;
            }
            E.elapsed = 0;
        }
    }

    
}

void ParticleSystem::update(vector<Force> forces, float dt) {
    // @TODO: Update the state of the particle system, move particles forwards, spawn new
    // particles, destroy old particles, and apply effects

    // Make sure there actually are particles
    if (particles.size() > 0) {
        // For each particle 
        for (int i = 0; i < (int)particles.size(); i++) {
            
            particles[i].lifetime = particles[i].lifetime - dt;

            if (particles[i].lifetime < 0) {
                particles.erase(particles.begin() + i);
            } else {
                // Calculate force
                // Calculate new position and velocity
                for (int j = 0; j < forces.size(); j++) {
                    float dx = forces[j].position.x - (particles[i].position.x);
                    float dy = forces[j].position.y - (particles[i].position.y);

                    float dis = sqrt(dx * dx + dy * dy);

                    if (dis < 0.5) {
                        if (dx != 0) {
                            // particles[i].velocity.x += 0.001 * forces[j].force * sign(dx) *
                            // pow(abs(dx), 0.5);
                            particles[i].velocity.x += 0.002 * forces[j].force * dx;
                        } else {
                            particles[i].velocity.x += 0.002 * forces[j].force * 0;
                        }
                        if (dy != 0) {
                            // particles[i].velocity.y += 0.001 * forces[j].force * sign(dy) *
                            // pow(abs(dy), 0.5);
                            particles[i].velocity.y += 0.002 * forces[j].force * dy;
                        } else {
                            particles[i].velocity.y += 0.002 * forces[j].force * 0;
                        }
                    } else if (dis < 1) {
                        if (dx != 0) {
                            // particles[i].velocity.x += 0.001 * forces[j].force * sign(dx) *
                            // pow(abs(dx), 0.5);
                            particles[i].velocity.x += 0.001 * forces[j].force * dx;
                        } else {
                            particles[i].velocity.x += 0.001 * forces[j].force * 0;
                        }
                        if (dy != 0) {
                            // particles[i].velocity.y += 0.001 * forces[j].force * sign(dy) *
                            // pow(abs(dy), 0.5);
                            particles[i].velocity.y += 0.001 * forces[j].force * dy;
                        } else {
                            particles[i].velocity.y += 0.001 * forces[j].force * 0;
                        }
                    }
                }
                particles[i].position.x += particles[i].velocity.x * dt;
                particles[i].position.y += particles[i].velocity.y * dt;
            }
        }
    }
}

void ParticleSystem::render(vector<Force> forces, vector<Emitter> emitters) {
    std::vector<rendering::ParticleInfo> newParticleArray;
    if (particles.size() > 0) {
        for (int i = 0; i < particles.size(); i++) {
            rendering::ParticleInfo p;
            vec2 v = {particles[i].position.x, particles[i].position.y};
            p.position = v;
            p.radius = particles[i].radius;
            newParticleArray.push_back(p);
        }
    }
    rendering::renderParticles(newParticleArray);

    std::vector<rendering::ForceInfo> newForceArray;
    if (forces.size() > 0) {
        for (int i = 0; i < forces.size(); i++) {
            rendering::ForceInfo f;
            vec2 v = {forces[i].position.x, forces[i].position.y};
            f.position = v;
            newForceArray.push_back(f);
        }
    }
    rendering::renderForces(newForceArray);

    std::vector<rendering::EmitterInfo> newEmitterArray;
    if (forces.size() > 0) {
        for (int i = 0; i < emitters.size(); i++) {
            rendering::EmitterInfo e;
            vec2 v = {emitters[i].position.x, emitters[i].position.y};
            e.position = v;
            newEmitterArray.push_back(e);
        }
    }
    rendering::renderEmitters(newEmitterArray);
}