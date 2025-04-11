#pragma once
#include <string>
#include <SDL2/SDL.h>
#include "Components.hpp"

class ColliderComponent : public Component {
    public:
        SDL_FRect collider;
        std::string tag;

        TransformComponent* transform;

        ColliderComponent(std::string t) {
            this->tag = t;
        }

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            transform = &entity->getComponent<TransformComponent>();

            Game::colliders.push_back(this);
        }

        void update() override {
            this->collider.x = transform->position.x;
            this->collider.y = transform->position.y;
            this->collider.w = transform->width * transform->scale;
            this->collider.h = transform->height * transform->scale;
        }
};