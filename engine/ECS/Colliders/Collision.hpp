#pragma once
#include <SDL2/SDL.h>
#include "Collider.hpp"
#include "HexagonCollider.hpp"
#include "CircleCollider.hpp"

class Collision {
    public:
        static bool AABB(const RectangleCollider& recA, const RectangleCollider& recB);
        static bool AABB(const Collider& colA, const Collider& colB);
        static bool HexCircle(HexagonCollider& hex, CircleCollider& cir);
};