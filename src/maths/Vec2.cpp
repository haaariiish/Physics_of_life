#include "Vec2.hpp"
#include <cmath>

Vec2::Vec2(float x, float y) : x(x), y(y) {}

Vec2 Vec2::operator+(const Vec2& o) const {
    return {x + o.x, y + o.y};
}

Vec2 Vec2::operator*(float s) const {
    return {x * s, y * s};
}

float Vec2::length() const {
    return std::sqrt(x*x + y*y);
}

Vec2 Vec2::normalized() const {
    float l = length();
    return (l > 0) ? Vec2{x/l, y/l} : Vec2{};
}