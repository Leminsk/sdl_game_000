#include <cmath>
#include "Collision.hpp"
#include "ECS/ColliderComponent.hpp"
#include "ECS/HexagonCollider.hpp"
#include "ECS/CircleCollider.hpp"

bool Collision::AABB(const SDL_FRect& recA, const SDL_FRect& recB) {
    if(
        recA.x + recA.w >= recB.x &&
        recB.x + recB.w >= recA.x &&
        recA.y + recA.h >= recB.y &&
        recB.y + recB.h >= recA.y
    ) {
        return true;
    }

    return false;
}

bool Collision::AABB(const ColliderComponent& colA, const ColliderComponent& colB) {
    if(AABB(colA.collider, colB.collider)) {
        std::cout << colA.tag << " hit: " << colB.tag << '\n';
        return true;
    }
    return false;
}

bool Collision::HexCircle(HexagonCollider& hex, CircleCollider& cir) {
    Vector2D hex_to_circle = Vector2D(hex.center.x - cir.center.x, hex.center.y - cir.center.y);
    float magnitude = sqrt((hex_to_circle.x*hex_to_circle.x) + (hex_to_circle.y*hex_to_circle.y));
    Vector2D hex_to_circle_normalized = Vector2D(hex_to_circle.x / magnitude, hex_to_circle.y / magnitude);

    Vector2D curr_vec;
    float max = -1;
    float curr_proj;

    for(int i=0; i<6; ++i) {
        curr_vec.x = hex.hull[i].x - hex.center.x;
        curr_vec.y = hex.hull[i].y - hex.center.y;
        curr_proj = (hex_to_circle_normalized.x * curr_vec.x) + (hex_to_circle_normalized.y * curr_vec.y);
        if(max < curr_proj) { max = curr_proj; }
    }

    if((magnitude - max - cir.radius) > 0 && magnitude > 0) {
        return true;
    }
    return false;
}