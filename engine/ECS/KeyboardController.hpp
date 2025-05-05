#pragma once

#include <SDL2/SDL.h>
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "SpriteComponent.hpp"


class KeyboardController : public Component {
    public:
        TransformComponent *transform;
        SpriteComponent *sprite;

        void init() override {
            this->transform = &entity->getComponent<TransformComponent>();
            this->sprite = &entity->getComponent<SpriteComponent>(); // gonna use this later to control sprite animations with this->sprite->play("some_animation")
        }

        void update() override {
            const uint8_t *keystates = SDL_GetKeyboardState(NULL);
            if(keystates[SDL_SCANCODE_W]) { this->transform->velocity.y = -1.0f; }
            if(keystates[SDL_SCANCODE_S]) { this->transform->velocity.y =  1.0f; }
            if(keystates[SDL_SCANCODE_A]) { this->transform->velocity.x = -1.0f; }
            if(keystates[SDL_SCANCODE_D]) { this->transform->velocity.x =  1.0f; }

            if(keystates[SDL_SCANCODE_R]) { this->sprite->rotating = true; }


            if(!keystates[SDL_SCANCODE_W] && !keystates[SDL_SCANCODE_S]) { this->transform->velocity.y = 0.0f; }
            if(!keystates[SDL_SCANCODE_A] && !keystates[SDL_SCANCODE_D]) { this->transform->velocity.x = 0.0f; }

            if(!keystates[SDL_SCANCODE_R]) { this->sprite->rotating = false; }
        }
};