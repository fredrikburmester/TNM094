#ifndef __EMITTER_H__
#define __EMITTER_H__

#include "util/color.h"
#include "util/rendering.h"
#include "util/vec2.h"
#include <vector>

class Emitter {
    public:
        vec2 position = {-0.5, 0};
        float spread = (float)0.1;
        float angle = (float)0;
        float velocity = (float)0.1;
        float pps = (float)2;
        float particleSize = (float)5;
        float elapsed = (float)0;

        void setPosition(vec2 position) { 
            this->position = position;
        }

        void setSpread(float spread) { 
            this->spread = spread; 
        }
};



class Explosion : public Emitter {
public: 
    Explosion(float spread, vec2 position) { 
        setPosition(position);
        setSpread(spread);
    };
};

#endif
