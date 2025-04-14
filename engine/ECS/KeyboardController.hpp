#pragma once

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
                    case SDLK_r:
                        this->sprite->rotating = true;
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
                    case SDLK_r:
                        this->sprite->rotating = false;
                        break;
                    case SDLK_ESCAPE:
                        Game::isRunning = false;
                        break;
                    default:
                        break;
                }
            }
        }
};