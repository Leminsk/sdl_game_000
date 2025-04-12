#pragma once
#include "../Components.hpp"
#include "../../Vector2D.hpp"

class CircleCollider : public Component {
    public:
        Vector2D center;
        float radius;

        TransformComponent *transform;
        Vector2D prev_position;
        Vector2D trans_pos_update;

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            this->transform = &entity->getComponent<TransformComponent>();
            this->prev_position = this->transform->position;
            
            this->radius = this->transform->width/2;
            this->center = AddVecs(Vector2D(this->radius, this->radius), this->transform->position);
        }

        void update() override {
            this->trans_pos_update = SubVecs(this->transform->position, this->prev_position);
            this->center += trans_pos_update;
            this->prev_position = this->transform->position;
            // std::cout<<this->center<<'\n';
        }
};