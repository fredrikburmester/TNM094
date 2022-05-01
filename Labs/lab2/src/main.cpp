#include "Tracy.hpp"
#include "particlesystem.h"
#include "util/rendering.h"

#include <algorithm>
#include <iterator>
#include <random>

#include "particle.h"
#include "force.h"
#include "emitter.h"

#include "Windows.h"

#include <iostream>

int main(int, char**) {
    rendering::createWindow();

    ParticleSystem particleSystem;

    std::vector<Emitter> emitters;
    std::vector<Force> forces;

    Emitter E1;
    E1.position = {-0.5, 0.2};

    emitters.push_back(E1);

    Explosion Ex1(6.3, vec2{0.5,0});

    //std::cout << Ex1.spread << " " << Ex1.position.x << ", " << Ex1.position.y << std::endl;

    emitters.push_back(Ex1);

    Force F1;
    F1.position = {-0.6,0};
    F1.force = 1;
    forces.push_back(F1);

    Force F2;
    F2.position = {0.5, 0.5};
    F2.force = 1;
    forces.push_back(F2);

    float speed = 1.0f;
    float angle = 0;
    float spread = 0;

    bool isRunning = true;
    while (isRunning) {
        float dt = rendering::beginFrame();
        dt = dt * speed;
        {
            ui::GuiScope ui; 

            ui::text("I'm text!");
            ui::sliderFloat("Angle E1", emitters[0].angle, 0.001f, 6.2831f);
            ui::sliderFloat("Spread E1", emitters[0].spread, 0.001f, 6.2831f);
            ui::sliderFloat("Particles/s E1", emitters[0].pps, 0.1f, 10.0f);
            ui::sliderFloat("Velocity E1", emitters[0].velocity, 0.1f, 1.0f);

            ui::sliderFloat("Force F1", forces[0].force, 1.0f, 10.0f);

            /*ui::sliderFloat("Angle E2", E2.angle, 0.001f, 6.2831f);
            ui::sliderFloat("Spread E2", E2.spread, 0.001f, 6.2831f);*/

            ui::sliderFloat("Max particles", particleSystem.maxParticles, 10, 10000);

            //ui::sliderFloat("Speed", speed, 0.01f, 10.0f);

            if (ui::button("Close application")) {
                isRunning = false;
            }
        }

        for (int i = 0; i < emitters.size(); i++) {
            particleSystem.createParticles(emitters[i], dt);
        }

        particleSystem.update(forces, dt);
        particleSystem.render(forces, emitters);

        // Particle generation and rendering example. Remove in your implementation.
        //particleRenderingExample();

        isRunning &= rendering::endFrame();
    }

    rendering::destroyWindow();

    return EXIT_SUCCESS;
}
