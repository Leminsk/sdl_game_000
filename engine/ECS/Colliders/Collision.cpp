#include <cmath>
#include <algorithm>
#include "../../Vector2D.hpp"
#include "Collision.hpp"
#include "Collider.hpp"
#include "HexagonCollider.hpp"
#include "CircleCollider.hpp"

// https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
bool lineIntersectCircle(const CircleCollider& cir, const Vector2D& line_start, const Vector2D& line_end) {
    float radius_2 = cir.radius * cir.radius;
    float distance_2 = Distance(line_start, line_end, false);
    if(distance_2 == 0.0f) {
        return Distance(cir.center, line_start, false) <= radius_2;
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
    float distance = Distance(cir.center, projection, false);
    return distance <= radius_2;
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

bool Collision::CircleCircle(const CircleCollider& cA, const CircleCollider& cB) {
    float distance = Distance(cA.center, cB.center, false);
    return distance <= (cA.radius + cB.radius) * (cA.radius + cB.radius);
}

bool Collision::CircleCircle(const Collider& colA, const Collider& colB) {
    if(colA.type == COLLIDER_CIRCLE && colB.type == COLLIDER_CIRCLE) {
        return CircleCircle(
            colA.entity->getComponent<CircleCollider>(), 
            colB.entity->getComponent<CircleCollider>()
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

bool Collision::ConvexPolygonCircle(const Collider& conv_pol, const CircleCollider& cir) {
    std::vector<Vector2D> hull;
    switch(conv_pol.type) {
        case COLLIDER_HEXAGON:   hull = conv_pol.entity->getComponent<  HexagonCollider>().hull; break;
        case COLLIDER_RECTANGLE: hull = conv_pol.entity->getComponent<RectangleCollider>().hull; break;
        default: return false;
    }

    bool collide = false;
    int hull_size = hull.size();

    int i;
    for(i=0; i<hull_size-1; ++i) {
        collide = collide || lineIntersectCircle(cir, hull[i], hull[i+1]);
    }
    collide = collide || lineIntersectCircle(cir, hull[i], hull[0]);
    return collide;
}