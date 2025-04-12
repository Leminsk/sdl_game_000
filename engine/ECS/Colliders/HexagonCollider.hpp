#pragma once
#include "../Components.hpp"
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
        |  C  |
        |     |
        2\    /0
          \  /
           \/
           1
         */

        TransformComponent *transform;
        Vector2D prev_position;
        // Vector2D trans_pos_update;

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            this->transform = &entity->getComponent<TransformComponent>();
            std::cout << "position:" << this->transform->position << '\n';
            // this->prev_position = this->transform->position;
            
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

        void update() override {
            // this->trans_pos_update = SubVecs(this->transform->position, this->prev_position);
            // std::cout << "this->trans_pos_update: " << this->trans_pos_update << '\n';
            // this->trans_pos_update.Scale(this->transform->scale);
            // std::cout << "this->trans_pos_update: " << this->trans_pos_update << '\n';
            // std::cout << "this->transform->scale: " << this->transform->scale << '\n';
            // this->center += trans_pos_update;
            // this->hull[0] += trans_pos_update;
            // this->hull[1] += trans_pos_update;
            // this->hull[2] += trans_pos_update;
            // this->hull[3] += trans_pos_update;
            // this->hull[4] += trans_pos_update;
            // this->hull[5] += trans_pos_update;
            // this->prev_position = this->transform->position;
            // std::cout<<"              "<<this->center<<'\n';
        }
};