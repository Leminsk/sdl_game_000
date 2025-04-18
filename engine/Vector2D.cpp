#include <cmath>
#include "Vector2D.hpp"

Vector2D::Vector2D() {
    this-> x = 0.0f;
    this-> y = 0.0f;
}

Vector2D::Vector2D(float x, float y) {
    this->x = x;
    this->y = y;
}

Vector2D& Vector2D::Add(const Vector2D& vec) {
    this->x += vec.x;
    this->y += vec.y;
    return *this;
}
Vector2D& Vector2D::Subtract(const Vector2D& vec) {
    this->x -= vec.x;
    this->y -= vec.y;
    return *this;
}
Vector2D& Vector2D::Multiply(const Vector2D& vec) {
    this->x *= vec.x;
    this->y *= vec.y;
    return *this;
}
Vector2D& Vector2D::Divide(const Vector2D& vec) {
    this->x /= vec.x;
    this->y /= vec.y;
    return *this;
}


Vector2D& Vector2D::operator+=(const Vector2D& vec) {
    return this->Add(vec);
}
Vector2D& Vector2D::operator-=(const Vector2D& vec) {
    return this->Subtract(vec);
}
Vector2D& Vector2D::operator*=(const Vector2D& vec) {
    return this->Multiply(vec);
}
Vector2D& Vector2D::operator/=(const Vector2D& vec) {
    return this->Divide(vec);
}

Vector2D& Vector2D::Scale(const float& i) {
    this->x *= i;
    this->y *= i;
    return *this;
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
    return this->Scale( 1/this->Magnitude() );
}

float Vector2D::OriginAngle() {
    return atanf(this->y / this->x);
}

std::ostream& operator<<(std::ostream& stream, const Vector2D& vec) {
    stream << "(" << vec.x << "," << vec.y << ")";
    return stream;
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
    float x = b.y - a.y;
    float y = b.x - a.x;
    if(square_root) {
        return sqrtf((y*y) + (x*x));
    }
    return (y*y) + (x*x);
}