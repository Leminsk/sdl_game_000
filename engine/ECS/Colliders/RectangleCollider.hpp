#pragma once
#include <SDL2/SDL.h>
#include "../Components.hpp"

class RectangleCollider : public Component {
    public:
        SDL_FRect collider;

        TransformComponent* transform;

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            transform = &entity->getComponent<TransformComponent>();
        }

        void update() override {
            this->collider.x = transform->position.x;
            this->collider.y = transform->position.y;
            this->collider.w = transform->width * transform->scale;
            this->collider.h = transform->height * transform->scale;
        }
};