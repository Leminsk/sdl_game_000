#include <cmath>
#include <algorithm>
#include <vector>
#include "../../Vector2D.hpp"
#include "Collision.hpp"
#include "Collider.hpp"
#include "HexagonCollider.hpp"
#include "CircleCollider.hpp"

// https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
bool lineIntersectCircle(const CircleCollider& cir, const Vector2D& line_start, const Vector2D& line_end) {
    float radius_2 = cir.radius * cir.radius;
    float distance_2 = Distance(line_start, line_end);
    if(distance_2 == 0.0f) {
        return Distance(cir.center, line_start) <= radius_2;
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
    
    return Distance(cir.center, projection) <= radius_2;
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
    float distance_2 = Distance(cA.center, cB.center);
    return distance_2 <= (cA.radius + cB.radius) * (cA.radius + cB.radius);
}

bool Collision::CircleCircle(const Collider& colA, const Collider& colB) {
    if(colA.type == COLLIDER_CIRCLE && colB.type == COLLIDER_CIRCLE) {
        return Collision::CircleCircle(
            colA.entity->getComponent<CircleCollider>(), 
            colB.entity->getComponent<CircleCollider>()
        );
    }

    return false;
}

bool Collision::HexCircle(const Collider& hex, const Collider& cir) {
    CircleCollider c = cir.entity->getComponent<CircleCollider>();
    Vector2D cir_center = cir.entity->getComponent<CircleCollider>().center;
    Vector2D hex_center = hex.entity->getComponent<HexagonCollider>().center;
    std::vector<Vector2D> hull = hex.entity->getComponent<HexagonCollider>().hull;
    

    if(hex_center.x < cir_center.x) {
        return (
            lineIntersectCircle(c, hull[0], hull[1]) ||
            lineIntersectCircle(c, hull[4], hull[5]) ||
            lineIntersectCircle(c, hull[5], hull[0])
        );
    } else {
        return (
            lineIntersectCircle(c, hull[1], hull[2]) ||
            lineIntersectCircle(c, hull[2], hull[3]) ||
            lineIntersectCircle(c, hull[3], hull[4])
        );
    }
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

bool Collision::Collide(const Collider& colA, const Collider& colB) {

    switch(colA.type) {
        case COLLIDER_CIRCLE:
            switch(colB.type) {
                case COLLIDER_CIRCLE: return Collision::CircleCircle(colA, colB); break;
                case COLLIDER_HEXAGON: return Collision::HexCircle(colB, colA); break;
                // case COLLIDER_RECTANGLE: return Collision::RectCircle(colB, colA); break; <- TODO: IMPLEMENT
            }
            break;

        case COLLIDER_HEXAGON:
            switch(colB.type) {
                case COLLIDER_CIRCLE: return Collision::CircleCircle(colA, colB); break;
                case COLLIDER_HEXAGON: return Collision::HexCircle(colA, colB); break;
                // case COLLIDER_RECTANGLE: return Collision::RectHex(colB, colA); break; <- TODO: IMPLEMENT
            }
            break;

        case COLLIDER_RECTANGLE:
            switch(colB.type) {
                // case COLLIDER_CIRCLE: return Collision::RectCircle(colA, colB); break; <- TODO: IMPLEMENT
                // case COLLIDER_HEXAGON: return Collision::RectHex(colA, colB); break; <- TODO: IMPLEMENT
                case COLLIDER_RECTANGLE: return Collision::AABB(colA, colB); break;
            }
            break;
    }

    return false;
}