#pragma once
#include <SDL2/SDL.h>
#include "../../Vector2D.hpp"
#include "Collider.hpp"
#include "HexagonCollider.hpp"
#include "RectangleCollider.hpp"
#include "CircleCollider.hpp"

class Collision {
    public:
        static bool pointInRect(float px, float py, float rx, float ry, float rw, float rh);
        static bool AABB(const RectangleCollider& recA, const RectangleCollider& recB);
        static bool AABB(const Collider& colA, const Collider& colB);
        static bool pointInHex(const Vector2D& p, const HexagonCollider& hex);
        static bool ConvexPolygonCircle(const Collider& conv_pol, const CircleCollider& cir);
        static Vector2D CollideStatic(const Collider& moving_col, const Collider& col, const float& distance2, const Vector2D& prev_pos);
        static std::vector<Vector2D> CollideDynamic(const Collider& a, const Collider& b, const float& distance2);
};