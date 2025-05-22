#include <cmath>
#include "utils.hpp"
#include "Vector2D.hpp"


Vector2D::Vector2D() {
    this-> x = 0.0f;
    this-> y = 0.0f;
}

Vector2D::Vector2D(float x, float y) {
    this->x = x;
    this->y = y;
}

std::ostream& operator<<(std::ostream& stream, const Vector2D& vec) {
    stream << "(" << vec.x << "," << vec.y << ")";
    return stream;
}
bool operator==(const Vector2D& a, const Vector2D& b) {
    return (a.x == b.x) && (a.y == b.y);
}
Vector2D operator+(const Vector2D& a, const Vector2D& b) {
    return AddVecs(a, b);
}
Vector2D operator-(const Vector2D& a, const Vector2D& b) {
    return SubVecs(a, b);
}
Vector2D operator/(const Vector2D& v, const float& s) {
    return Vector2D(v.x / s,  v.y / s);
}
Vector2D operator*(const Vector2D& v, const float& s) {
    return Vector2D(v.x * s,  v.y * s);
}
float operator*(const Vector2D& a, const Vector2D& b) {
    return DotProd(a, b);
}


Vector2D& Vector2D::Zero() {
    this->x = 0;
    this->y = 0;
    return *this;
}

float Vector2D::ScProj(Vector2D& p) {
    return DotProd(*this, p) / p.Magnitude();
}
Vector2D Vector2D::VecProj(Vector2D& p) {
    float sc_proj = this->ScProj(p);
    p.x *= sc_proj / p.Magnitude();
    p.y *= sc_proj / p.Magnitude();
    return p;
}


float Vector2D::Magnitude2() {
    return (this->x * this->x) + (this->y * this->y);
}
float Vector2D::Magnitude() {
    return sqrtf((this->x * this->x) + (this->y * this->y));
}
Vector2D& Vector2D::Normalize() {
    float mag = this->Magnitude();
    if(mag == 0) { return this->Zero(); }
    *this = *this / mag;
    return *this;
}

float Vector2D::OriginAngle() {
    return atanf(this->y / this->x);
}



std::string Vector2D::FormatDecimal(int integer_precision, int decimal_precision) {
    std::string x_str = format_decimal(this->x, integer_precision, decimal_precision);
    std::string y_str = format_decimal(this->y, integer_precision, decimal_precision);
    return '(' + x_str + ',' + y_str + ')';
}





float AngleVecs(Vector2D& a, Vector2D& b) {
    return acosf(DotProd(a, b) / (a.Magnitude() * b.Magnitude()));
}

Vector2D AddVecs(const Vector2D& a, const Vector2D& b) {
    Vector2D sum = Vector2D(a.x + b.x, a.y + b.y);
    return sum;
}
Vector2D SubVecs(const Vector2D& a, const Vector2D& b) {
    Vector2D sub = Vector2D(a.x - b.x, a.y - b.y);
    return sub;
}
float DotProd(const Vector2D& a, const Vector2D& b) {
    return (a.x * b.x) + (a.y * b.y);
}
float DotProd(float magA, float magB, float angle) {
    return magA * magB * angle;
}
Vector2D VecProj(Vector2D& a, Vector2D& p) {
    float sc_proj = a.ScProj(p);
    p.x *= sc_proj / p.Magnitude();
    p.y *= sc_proj / p.Magnitude();
    return p;
}

float Distance(const Vector2D& a, const Vector2D& b, bool square_root) {
    float x = b.x - a.x;
    float y = b.y - a.y;
    float d2 = (y*y) + (x*x);
    if(square_root) {
        return sqrtf(d2);
    }
    return d2;
}

float lerp(float a, float b, float f) {
    if(a == b) { return a; }
    return (a * (1.0f - f)) + (b*f);
}

Vector2D VecLerp(const Vector2D& a, const Vector2D& b, float f) {
    return Vector2D(lerp(a.x, b.x, f), lerp(a.y, b.y, f));
}