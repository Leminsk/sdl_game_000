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
            float* vy = &this->transform->velocity.y;
            float* vx = &this->transform->velocity.x;
            if(keystates[SDL_SCANCODE_W]) { *vy = std::max(*vy - 0.2f, -2.0f); }
            if(keystates[SDL_SCANCODE_S]) { *vy = std::min(*vy + 0.2f,  2.0f); }
            if(keystates[SDL_SCANCODE_A]) { *vx = std::max(*vx - 0.2f, -2.0f); }
            if(keystates[SDL_SCANCODE_D]) { *vx = std::min(*vx + 0.2f,  2.0f);; }

            if(keystates[SDL_SCANCODE_R]) { this->sprite->rotating = true; }


            if(!keystates[SDL_SCANCODE_W] && !keystates[SDL_SCANCODE_S]) { *vy = 0.0f; }
            if(!keystates[SDL_SCANCODE_A] && !keystates[SDL_SCANCODE_D]) { *vx = 0.0f; }

            if(!keystates[SDL_SCANCODE_R]) { this->sprite->rotating = false; }
        }
};