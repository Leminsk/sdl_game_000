#pragma once
#include <SDL2/SDL.h>
#include "Collider.hpp"
#include "HexagonCollider.hpp"
#include "RectangleCollider.hpp"
#include "CircleCollider.hpp"

class Collision {
    public:
        static bool AABB(const RectangleCollider& recA, const RectangleCollider& recB);
        static bool AABB(const Collider& colA, const Collider& colB);
        static bool CircleCircle(const CircleCollider& cA, const CircleCollider& cB);
        static bool CircleCircle(const Collider& colA, const Collider& colB);
        static bool HexCircle(const HexagonCollider& hex, const CircleCollider& cir);
        static bool ConvexPolygonCircle(const Collider& conv_pol, const CircleCollider& cir);
};