#pragma once
#include <SDL2/SDL.h>
#include "Vector2D.hpp"

class ColliderComponent;
class HexagonCollider;
class CircleCollider;

class Collision {
    public:
        static bool AABB(const SDL_FRect& recA, const SDL_FRect& recB);
        static bool AABB(const ColliderComponent& colA, const ColliderComponent& colB);
        static bool HexCircle(HexagonCollider& hex, CircleCollider& cir);
};