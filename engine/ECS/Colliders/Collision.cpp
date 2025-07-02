#include <cmath>
#include <algorithm>
#include <vector>
#include "../../utils.hpp"
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

// get the closest point on a line to a circle
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

bool Collision::pointInRect(float px, float py, float rx, float ry, float rw, float rh) {
    return (
        rx <= px && px <= (rx + rw) &&
        ry <= py && py <= (ry + rh)
    );
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
    if(colA.type == ColliderType::RECTANGLE && colB.type == ColliderType::RECTANGLE) {
        return AABB(
            colA.entity->getComponent<RectangleCollider>(), 
            colB.entity->getComponent<RectangleCollider>()
        );
    }

    return false;
}

// https://stackoverflow.com/questions/62432809/sat-polygon-circle-collision-resolve-the-intersection-in-the-direction-of-velo
void foo() {

}

// https://youtu.be/eED4bSkYCB8?t=789
// Overwrites velocities of BOTH colliders
void dynamicCircleVSdynamicCircle(const CircleCollider& a, const CircleCollider& b, const float& distance2) {
    if(distance2 <= (a.radius + b.radius) * (a.radius + b.radius)) {
        // TODO: mass momentum later maybe probably not
        float ma = 1;
        float mb = 1;
        Vector2D va_minus_vb = a.transform->velocity - b.transform->velocity;
        Vector2D vb_minus_va = b.transform->velocity - a.transform->velocity;
        Vector2D ca_minus_cb = a.center - b.center;
        Vector2D cb_minus_ca = b.center - a.center;
        float mass_coef_va = 2*mb / (ma+mb);
        float mass_coef_vb = 2*ma / (ma+mb);
        a.transform->velocity = a.transform->velocity - ((ca_minus_cb) * (mass_coef_va * DotProd(va_minus_vb, ca_minus_cb) / distance2));
        b.transform->velocity = b.transform->velocity - ((cb_minus_ca) * (mass_coef_vb * DotProd(vb_minus_va, cb_minus_ca) / distance2));
    }
}

/*
OLC: https://youtu.be/8JJ-4JgR7Dg?t=3136
given a point, and its direction vector, return true if it would collide with RectangleCollider. contact_point is the point of collision, contact_normal points outside the rectangle
t_hit_near is the "time" "when" the ray would hit the contact point
*/
bool rayVSRect(const Vector2D& ray_origin, const Vector2D& ray_dir, const RectangleCollider& target, Vector2D& contact_point, Vector2D contact_normal, float& t_hit_near) {
    contact_normal = Vector2D(0,0);
    contact_point = Vector2D(0,0);

    float invdir_x = 1.0f / ray_dir.x;
    float invdir_y = 1.0f / ray_dir.y;

    Vector2D t_near = (target.transform->position - ray_origin);
    t_near.x *= invdir_x;
    t_near.y *= invdir_y;
    Vector2D t_far = (target.transform->position - ray_origin);
    t_far.x *= invdir_x;
    t_far.y *= invdir_y;

    if(std::isnan(t_near.y) || std::isnan(t_near.x)) { return false; }
    if(std::isnan(t_far.y ) || std::isnan(t_far.x )) { return false; }
    
    if(t_near.x > t_far.x) { std::swap(t_near.x, t_far.x); }
    if(t_near.y > t_far.y) { std::swap(t_near.y, t_far.y); }
    if(t_near.x > t_far.y || t_near.y > t_far.x) { return false; }

    t_hit_near = std::max(t_near.x, t_near.y);
    float t_hit_far = std::min(t_far.x, t_far.y);
    if(t_hit_far < 0) { return false; }

    contact_point = ray_origin + (ray_dir * t_hit_near);

    if(t_near.x > t_near.y) {
        if(invdir_x < 0) { contact_normal = Vector2D(  1.0f,  0.0f); }
        else {             contact_normal = Vector2D( -1.0f,  0.0f); }
    }
    else if(t_near.x < t_near.y) {
        if(invdir_y < 0) { contact_normal = Vector2D(  0.0f,  1.0f); }
        else {             contact_normal = Vector2D(  0.0f, -1.0f); }
    }

    return true;
}

// assumes RectangleCollider is fixed/stationary
// returns translation movement vector indicating the direction the Rectangle thinks the Circle should be moved
// e.g. returns Vector(-1.0, 0.0) means "move the circle to the left by 1 unit".
Vector2D resolveCircleVSRect(const CircleCollider& cir, const RectangleCollider& rect, const float& distance2, const Vector2D& prev_pos) {
    Vector2D nearest_point = Vector2D(
        std::max( rect.x, std::min(cir.center.x, rect.x + rect.w) ),
        std::max( rect.y, std::min(cir.center.y, rect.y + rect.h) )
    );

    if(nearest_point == cir.center) {
        // circle center is inside the rect, use old position to see where it would have collided
        nearest_point = Vector2D(
            std::max( rect.x, std::min(prev_pos.x, rect.x + rect.w) ),
            std::max( rect.y, std::min(prev_pos.y, rect.y + rect.h) )
        );
        Vector2D out_ray = nearest_point - cir.center;
        if(nearest_point == cir.center) {
            // circle is ON the edge, move it out by radius (away from the rectangle center)
            return (cir.center - rect.center).Normalize() * cir.radius;
        }
        // move it out by how far away it is from the edge plus its radius (towards the its old position)
        return out_ray + (out_ray.Normalize() * cir.radius);
    }
    
    Vector2D ray_to_nearest = nearest_point - cir.center;
    float overlap = (cir.radius * cir.radius) - ray_to_nearest.Magnitude2();
    if(overlap > 0) {
        // check if rect has adjacent rects that would prevent direct collision
        bool nearest_on_top    = (nearest_point.y ==            rect.y);
        bool nearest_on_bottom = (nearest_point.y == (rect.y + rect.h));
        bool nearest_on_left   = (nearest_point.x ==            rect.x);
        bool nearest_on_right  = (nearest_point.x == (rect.x + rect.w));
        if(
            (nearest_on_top && readBit(rect.adjacent_rectangles, 1)) ||
            (nearest_on_left && readBit(rect.adjacent_rectangles, 3)) ||
            (nearest_on_right && readBit(rect.adjacent_rectangles, 4)) ||
            (nearest_on_bottom && readBit(rect.adjacent_rectangles, 6))
        ) {
            return Vector2D(0,0);
        }
        overlap = cir.radius - ray_to_nearest.Magnitude();
        // move it out by the amount of overlap (towards the circle center)
        return ray_to_nearest.Normalize() * -overlap;
    }
    // no need to move it
    return Vector2D(0,0);
}


// treats both CircleColliders as dynamic
// returns two translation vectors, index 0 for a, index 1 for b
std::vector<Vector2D> resolveCircleVSCircle(const CircleCollider& a, const CircleCollider& b, const float& distance2) {
    std::vector<Vector2D> translations = {Vector2D(0,0), Vector2D(0,0)};
    if(distance2 < (a.radius + b.radius)*(a.radius + b.radius)) {
        Vector2D ray_a_to_b = (b.center - a.center).Normalize();
        float half_overlap = ((a.radius + b.radius) - std::sqrt(distance2)) / 2;
        translations[0] = ray_a_to_b * (-half_overlap);
        translations[1] = ray_a_to_b * half_overlap;        
    }
    return translations;
}

bool Collision::ConvexPolygonCircle(const Collider& conv_pol, const CircleCollider& cir) {
    std::vector<Vector2D> hull;
    switch(conv_pol.type) {
        case ColliderType::HEXAGON:   hull = conv_pol.entity->getComponent<  HexagonCollider>().hull; break;
        case ColliderType::RECTANGLE: hull = conv_pol.entity->getComponent<RectangleCollider>().hull; break;
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

    Vector2D ray_to_nearest = nearest_points[index] - cir.center;
    float overlap = (cir.radius * cir.radius) - ray_to_nearest.Magnitude2();
    if(overlap > 0) {
        overlap = cir.radius - ray_to_nearest.Magnitude();
        // move it out by the amount of overlap (towards the circle center)
        return ray_to_nearest.Normalize() * -overlap;
    }
    // no need to move it
    return Vector2D(0,0);
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

// returns a 2D translation vector to move the moving_col transform
// assumes moving_col can only be moved if it's a CircleCollider
Vector2D Collision::CollideStatic(const Collider& moving_col, const Collider& col, const float& distance2, const Vector2D& prev_pos) {
    if(moving_col.type == ColliderType::CIRCLE) {
        if(col.type == ColliderType::HEXAGON) {
            return CircleHex(
                moving_col.entity->getComponent<CircleCollider>(),
                col.entity->getComponent<HexagonCollider>()
            );
        }
        if(col.type == ColliderType::RECTANGLE) {
            return resolveCircleVSRect(
                moving_col.entity->getComponent<CircleCollider>(), 
                col.entity->getComponent<RectangleCollider>(),
                distance2, prev_pos
            ); 
        }
    }

    return Vector2D(0,0);
}

// returns two 2D translation vectors to move both colliders' transforms (index 0 for a, index 1 for b)
// assumes both are CircleColliders
std::vector<Vector2D> Collision::CollideDynamic(const Collider& a, const Collider& b, const float& distance2) {
    if(a.type == ColliderType::CIRCLE && b.type == ColliderType::CIRCLE) {
        return resolveCircleVSCircle(
            a.entity->getComponent<CircleCollider>(), 
            b.entity->getComponent<CircleCollider>(), 
            distance2
        );
    }
    return {Vector2D(0,0), Vector2D(0,0)};
}