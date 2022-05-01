#ifndef __COLOR_H__
#define __COLOR_H__

struct Color {
    constexpr Color() = default;
    constexpr Color(float r, float g, float b) : r(r), g(g), b(b) {}

    float r = 1.f;
    float g = 1.f;
    float b = 1.f;
};

#endif // __COLOR_H__