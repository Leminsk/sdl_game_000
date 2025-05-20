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

    Vector2D line_vec = line_end - line_start;
    float t = std::max(
        0.0f, 
        std::min(
            1.0f,
            (DotProd((cir.center - line_start), line_vec) / distance_2)
        )
    );

    Vector2D projection = line_start + (line_vec * t);
    
    return Distance(cir.center, projection) <= radius_2;
}

Vector2D projectionLineIntersectCircle(const CircleCollider& cir, const Vector2D& line_start, const Vector2D& line_end) {
    float radius_2 = cir.radius * cir.radius;
    float distance_2 = Distance(line_start, line_end);
    if(distance_2 == 0.0f) { return line_start; }

    Vector2D line_vec = line_end - line_start;
    float t = std::max(
        0.0f, 
        std::min(
            1.0f,
            (DotProd((cir.center - line_start), line_vec) / distance_2)
        )
    );

    // projection on line
    return line_start + (line_vec * t);
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

// push circle back if overlaping with nearest_point; otherwise return same position
Vector2D resolveCircleOverlap(const Vector2D& nearest_point, const CircleCollider& cir) {
    Vector2D ray_to_nearest = nearest_point - cir.center;
    float overlap = (cir.radius * cir.radius) - ray_to_nearest.Magnitude2();
    if(overlap > 0) {
        overlap = cir.radius - ray_to_nearest.Magnitude();
        Vector2D new_pos = cir.center - (ray_to_nearest.Normalize() * overlap);
        // offset from center to transform (x,y) pos
        new_pos.x -= cir.transform->width/2;
        new_pos.y -= cir.transform->height/2;
        return new_pos;
    }
    return cir.transform->position;
}

Vector2D CircleRect(const CircleCollider& cir, const RectangleCollider& rect) {
    // https://www.youtube.com/watch?v=D2a5fHX-Qrs
    Vector2D nearest_point = Vector2D(
        std::max( rect.x, std::min(cir.center.x, rect.x + rect.w) ),
        std::max( rect.y, std::min(cir.center.y, rect.y + rect.h) )
    );
    return resolveCircleOverlap(nearest_point, cir);
}


Vector2D CircleCir(const CircleCollider& a, const CircleCollider& b) {
    // https://gamedev.stackexchange.com/questions/71941/calculate-point-of-circle-circle-collision-between-frames
    float distance_2 = Distance(a.center, b.center);
    if(distance_2 <= (a.radius + b.radius) * (a.radius + b.radius)) {
        Vector2D nearest_point = VecLerp(b.center, a.center, b.radius/(a.radius + b.radius));
        return resolveCircleOverlap(nearest_point, a);
    }
    return a.transform->position;
}

Vector2D CircleHex(const CircleCollider& cir, const HexagonCollider& hex) {
    std::vector<Vector2D> hull = hex.hull;
    std::vector<Vector2D> nearest_points = std::vector<Vector2D>(3);

    // hexagon colliders are aligned with the axis
    if(hex.center.x < cir.center.x) {
        nearest_points[0] = projectionLineIntersectCircle(cir, hull[4], hull[5]);
        nearest_points[1] = projectionLineIntersectCircle(cir, hull[5], hull[0]);
        nearest_points[2] = projectionLineIntersectCircle(cir, hull[0], hull[1]);
    } else {
        nearest_points[0] = projectionLineIntersectCircle(cir, hull[1], hull[2]);
        nearest_points[1] = projectionLineIntersectCircle(cir, hull[2], hull[3]);
        nearest_points[2] = projectionLineIntersectCircle(cir, hull[3], hull[4]);
    }

    int index = 0;
    float min_distance = Distance(nearest_points[0], cir.center);
    float temp_dist = Distance(nearest_points[1], cir.center);
    if(min_distance > temp_dist) { min_distance = temp_dist; index = 1; }
    temp_dist = Distance(nearest_points[2], cir.center);
    if(min_distance > temp_dist) { min_distance = temp_dist; index = 2; }
    
    return resolveCircleOverlap(nearest_points[index], cir);
}

bool HexCircle(const Collider& hex, const Collider& cir) {
    CircleCollider c = cir.entity->getComponent<CircleCollider>();
    Vector2D cir_center = cir.entity->getComponent<CircleCollider>().center;
    Vector2D hex_center = hex.entity->getComponent<HexagonCollider>().center;
    std::vector<Vector2D> hull = hex.entity->getComponent<HexagonCollider>().hull;
    

    if(hex_center.x < cir_center.x) {
        return (
            lineIntersectCircle(c, hull[4], hull[5]) ||
            lineIntersectCircle(c, hull[5], hull[0]) ||
            lineIntersectCircle(c, hull[0], hull[1])
        );
    } else {
        return (
            lineIntersectCircle(c, hull[1], hull[2]) ||
            lineIntersectCircle(c, hull[2], hull[3]) ||
            lineIntersectCircle(c, hull[3], hull[4])
        );
    }
}

// returns the new position the moving object should be at
Vector2D Collision::Collide(const Collider& moving_col, const Collider& col) {
    switch(moving_col.type) {
        case COLLIDER_CIRCLE:
            switch(col.type) {
                case COLLIDER_CIRCLE: return CircleCir(
                    moving_col.entity->getComponent<CircleCollider>(),
                    col.entity->getComponent<CircleCollider>()
                ); break;
                case COLLIDER_HEXAGON: return CircleHex(
                    moving_col.entity->getComponent<CircleCollider>(),
                    col.entity->getComponent<HexagonCollider>()
                ); break;
                case COLLIDER_RECTANGLE: return CircleRect(
                    moving_col.entity->getComponent<CircleCollider>(), 
                    col.entity->getComponent<RectangleCollider>()
                ); break;
            }
            break;

        case COLLIDER_HEXAGON:
            switch(col.type) {
                // case COLLIDER_CIRCLE: return Collision::CircleCircle(moving_col, col); break;
                // case COLLIDER_HEXAGON: return Collision::HexCircle(moving_col, col); break;
                // case COLLIDER_RECTANGLE: return Collision::HexRect(moving_col, col); break; <- TODO: IMPLEMENT
            }
            break;

        case COLLIDER_RECTANGLE:
            switch(col.type) {
                // case COLLIDER_CIRCLE: return Collision::RectCircle(moving_col, col); break; <- TODO: IMPLEMENT
                // case COLLIDER_HEXAGON: return Collision::RectHex(moving_col, col); break; <- TODO: IMPLEMENT
                // case COLLIDER_RECTANGLE: return Collision::AABB(moving_col, colB); break;
            }
            break;
    }

    return moving_col.transform->position;
}