#include <cmath>
#include <algorithm>
#include "../../Vector2D.hpp"
#include "Collision.hpp"
#include "Collider.hpp"
#include "HexagonCollider.hpp"
#include "CircleCollider.hpp"

// https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
bool lineIntersectCircle(const CircleCollider& cir, const Vector2D& line_start, const Vector2D& line_end) {
    float distance_2 = Distance(line_start, line_end, false);
    if(distance_2 == 0.0f) {
        return Distance(cir.center, line_start) <= cir.radius;
    }

    Vector2D line_vec = SubVecs(line_end, line_start);
    float t = std::max(
        0.0f, 
        std::min(
            1.0f,
            (DotProd(SubVecs(cir.center, line_start), line_vec) / distance_2)
        )
    );

    Vector2D projection = AddVecs(line_start, (SubVecs(cir.center, line_start), line_vec).Scale(t));
    float distance = Distance(cir.center, projection);
    return distance <= cir.radius;
}

bool Collision::AABB(const RectangleCollider& recA, const RectangleCollider& recB) {
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

bool Collision::AABB(const Collider& colA, const Collider& colB) {
    if(colA.type == COLLIDER_RECTANGLE && colB.type == COLLIDER_RECTANGLE) {
        return AABB(
            colA.entity->getComponent<RectangleCollider>(), 
            colB.entity->getComponent<RectangleCollider>()
        );
    }

    return false;
}

bool Collision::HexCircle(const HexagonCollider& hex, const CircleCollider& cir) {
    return (
        lineIntersectCircle(cir, hex.hull[0], hex.hull[1]) ||
        lineIntersectCircle(cir, hex.hull[1], hex.hull[2]) ||
        lineIntersectCircle(cir, hex.hull[2], hex.hull[3]) ||
        lineIntersectCircle(cir, hex.hull[3], hex.hull[4]) ||
        lineIntersectCircle(cir, hex.hull[4], hex.hull[5]) ||
        lineIntersectCircle(cir, hex.hull[5], hex.hull[0])
    );
}

