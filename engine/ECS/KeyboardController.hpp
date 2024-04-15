#pragma once

#include "../Game.hpp"
#include "ECS.hpp"
#include "Components.hpp"

class KeyboardController : public Component {
    public:
        TransformComponent *transform;

        void init() override {
            this->transform = &entity->getComponent<TransformComponent>();
        }

        void update() override {
            if (Game::event.type == SDL_KEYDOWN) {
                switch (Game::event.key.keysym.sym) {
                    case SDLK_w:
                        this->transform->velocity.y = -1;
                        break;
                    case SDLK_a:
                        this->transform->velocity.x = -1;
                        break;
                    case SDLK_s:
                        this->transform->velocity.y = 1;
                        break;
                    case SDLK_d:
                        this->transform->velocity.x = 1;
                        break;
                    default:
                        break;
                }
            }

            if (Game::event.type == SDL_KEYUP) {
                switch (Game::event.key.keysym.sym) {
                    case SDLK_w:
                        this->transform->velocity.y = 0;
                        break;
                    case SDLK_a:
                        this->transform->velocity.x = 0;
                        break;
                    case SDLK_s:
                        this->transform->velocity.y = 0;
                        break;
                    case SDLK_d:
                        this->transform->velocity.x = 0;
                        break;
                    default:
                        break;
                }
            }
        }
};