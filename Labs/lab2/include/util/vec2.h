#ifndef __VEC2_H__
#define __VEC2_H__

#include <cmath>

struct vec2 {
    float x = 0.f;
    float y = 0.f;

    constexpr vec2() noexcept = default;
    constexpr vec2(float x, float y) noexcept : x(x), y(y) {}

    inline constexpr vec2 operator+(const vec2& rhs) const noexcept {
        return vec2{ x + rhs.x, y + rhs.y };
    }

    inline constexpr vec2 operator+(float rhs) const noexcept {
        return vec2{ x + rhs, y + rhs };
    }
    
    inline constexpr vec2 operator-(const vec2& rhs) const noexcept {
        return vec2{ x - rhs.x, y - rhs.y };
    }

    inline constexpr vec2 operator-(float rhs) const noexcept {
        return vec2{ x - rhs, y - rhs };
    }

    inline constexpr vec2 operator*(const vec2& rhs) const noexcept {
        return vec2{ x * rhs.x, y * rhs.y };
    }

    inline constexpr vec2 operator*(float rhs) const noexcept {
        return vec2{ x * rhs, y * rhs };
    }

    inline constexpr vec2 operator/(const vec2& rhs) const noexcept {
        return vec2{ x / rhs.x, y / rhs.y };
    }
    
    inline constexpr vec2 operator/(float rhs) const noexcept {
        return vec2{ x / rhs, y / rhs };
    }

    inline constexpr vec2& operator+=(const vec2& rhs) noexcept {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    inline constexpr vec2& operator+=(float rhs) noexcept {
        x += rhs;
        y += rhs;
        return *this;
    }

    inline constexpr vec2& operator-=(const vec2& rhs) noexcept {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    inline constexpr vec2& operator-=(float rhs) noexcept {
        x -= rhs;
        y -= rhs;
        return *this;
    }

    inline constexpr vec2& operator*=(const vec2& rhs) noexcept {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    inline constexpr vec2& operator*=(float rhs) noexcept {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    inline constexpr vec2& operator/=(const vec2& rhs) noexcept {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }

    inline constexpr vec2& operator/=(float rhs) noexcept {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    inline float length() const {
        return std::sqrt(x * x + y * y);
    }

    inline vec2 normalized() const {
        const float len = length();
        return vec2(x / len, y / len);
    }
};

#endif // __VEC2_H__