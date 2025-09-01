#pragma once

#include <iostream>
#include <string>

// Used to store things like position and velocity. 
class Vector2D {
    public:
        float x, y;

        Vector2D();
        Vector2D(float x, float y);

        friend std::ostream& operator<<(std::ostream& stream, const Vector2D& vec);
        friend bool operator==(const Vector2D& a, const Vector2D& b);
        friend bool operator!=(const Vector2D& a, const Vector2D& b);
        friend Vector2D operator+(const Vector2D& a, const Vector2D& b);
        friend Vector2D operator-(const Vector2D& a, const Vector2D& b);
        friend Vector2D operator/(const Vector2D& v, const float& s);
        friend Vector2D operator*(const Vector2D& v, const float& s);
        friend float operator*(const Vector2D& a, const Vector2D& b);        
        
        Vector2D& Zero();

        float ScProj(Vector2D& p);
        Vector2D VecProj(Vector2D& p);

        float Magnitude2();
        float Magnitude();
        Vector2D& Normalize();
        float OriginAngle();

        std::string FormatDecimal(int integer_precision, int decimal_precision);
};

float AngleVecs(const Vector2D& a, const Vector2D& b);
Vector2D AddVecs(const Vector2D& a, const Vector2D& b);
Vector2D SubVecs(const Vector2D& a, const Vector2D& b);
float DotProd(const Vector2D& a, const Vector2D& b);
float DotProd(float magA, float magB, float angle);
Vector2D VecProj(Vector2D& a, Vector2D& p);
// distance is returned as squared by default
float Distance(const Vector2D& a, const Vector2D& b, bool square_root=false);

float lerp(float a, float b, float f);
Vector2D VecLerp(const Vector2D& a, const Vector2D& b, float f);