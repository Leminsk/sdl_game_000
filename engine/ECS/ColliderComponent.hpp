#pragma once
#include <string>
#include <SDL2/SDL.h>
#include "Components.hpp"

class ColliderComponent : public Component {
    public:
        SDL_Rect collider;
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
            this->collider.x = static_cast<int>(transform->position.x);
            this->collider.y = static_cast<int>(transform->position.y);
            this->collider.w = transform->width * transform->scale;
            this->collider.h = transform->height * transform->scale;
        }
};