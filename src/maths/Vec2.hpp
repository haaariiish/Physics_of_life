#pragma once  // évite les inclusions multiples

struct Vec2 {
    float x, y;

    Vec2(float x = 0.f, float y = 0.f);  // constructeur

    Vec2 operator+(const Vec2& other) const;
    Vec2 operator*(float scalar) const;
    float length() const;      // norme du vecteur
    Vec2  normalized() const;  // vecteur unitaire
};