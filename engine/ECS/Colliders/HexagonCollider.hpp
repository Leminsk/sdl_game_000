#pragma once

#include "../ECS.hpp"
#include "../TransformComponent.hpp"
#include "../../Vector2D.hpp"

class HexagonCollider : public Component {
    private:
        // this hull uses the transform position as its reference
        void setHull() {
            float lesser_height = this->radius/2;
            float cos_30_radius = this->radius * 0.8660254f;
            float x_gap = this->radius - cos_30_radius;
            float right_x = (this->transform->width * this->transform->scale) - x_gap;
            float greater_height = (this->transform->height * this->transform->scale) - lesser_height;

            this->center = Vector2D(
                this->transform->position.x + this->radius, 
                this->transform->position.y + this->radius
            );
            this->hull[0] = AddVecs(Vector2D( right_x,                                   greater_height), this->transform->position);
            this->hull[1] = AddVecs(Vector2D(  radius, this->transform->height * this->transform->scale), this->transform->position);
            this->hull[2] = AddVecs(Vector2D(   x_gap,                                   greater_height), this->transform->position);
            this->hull[3] = AddVecs(Vector2D(   x_gap,                                    lesser_height), this->transform->position);
            this->hull[4] = AddVecs(Vector2D(  radius,                                             0.0f), this->transform->position);
            this->hull[5] = AddVecs(Vector2D( right_x,                                    lesser_height), this->transform->position);
        }
        
    public:
        float radius;
        Vector2D center;
        std::vector<Vector2D> hull = std::vector<Vector2D>(6);
        /* the hexagon is inscribed in a circle which is inscribed in a rect (preferrably a square)
        not to scale
            4
           /\
          /  \
        3/    \5
        |     |
        | c.  |
        |  ^  |
        2\    /0
          \  /
           \/
           1
         */

        TransformComponent *transform;

        HexagonCollider(TransformComponent* transf) {
            this->transform = transf;
        }

        void init() override {
            this->radius = this->transform->width/2 * this->transform->scale;
            setHull();
        }

        void update(const float& frame_delta) override {
            setHull();
        }
};