#pragma once

#include "../ECS.hpp"
#include "../TransformComponent.hpp"
#include "../../Vector2D.hpp"

class HexagonCollider : public Component {
    public:
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
        Vector2D prev_position;

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            this->transform = &entity->getComponent<TransformComponent>();
            
            float radius = this->transform->width/2;
            float lesser_height = radius/2;
            float cos_30_radius = radius * 0.8660254f;
            float x_gap = radius - cos_30_radius;
            float right_x = this->transform->width - x_gap;
            float greater_height = this->transform->height - lesser_height;

            Vector2D offset = this->transform->position;
            offset.x /= 2;
            offset.y /= 2;

            this->center = AddVecs(Vector2D(radius, radius), offset).Scale(this->transform->scale);
            this->hull[0] = AddVecs(Vector2D(right_x,          greater_height), offset).Scale(this->transform->scale);
            this->hull[1] = AddVecs(Vector2D( radius, this->transform->height), offset).Scale(this->transform->scale);
            this->hull[2] = AddVecs(Vector2D(  x_gap,          greater_height), offset).Scale(this->transform->scale);
            this->hull[3] = AddVecs(Vector2D(  x_gap,           lesser_height), offset).Scale(this->transform->scale);
            this->hull[4] = AddVecs(Vector2D( radius,                    0.0f), offset).Scale(this->transform->scale);
            this->hull[5] = AddVecs(Vector2D(right_x,           lesser_height), offset).Scale(this->transform->scale);
        }
};