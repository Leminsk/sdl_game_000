#pragma once

#include "../TextureManager.hpp"

#include "Components.hpp"
#include <SDL2/SDL.h>

class SpriteComponent : public Component {
    private:
        TransformComponent *transform;
        SDL_Texture *texture;
        SDL_Rect srcRect, destRect;

    public:
        SpriteComponent() = default;
        SpriteComponent(const char* path) {
            setTex(path);
        }

        void setTex(const char* path) {
            texture = TextureManager::LoadTexture(path);
        }

        void init() override {
            this->transform = &entity->getComponent<TransformComponent>();

            this->srcRect.x = 0;
            this->srcRect.y = 0;
            this->srcRect.w = 32;
            this->srcRect.h = 32;

            this->destRect.w = this->srcRect.w * 2;
            this->destRect.h = this->srcRect.h * 2;
        }
        void update() override {
            this->destRect.x = (int)this->transform->position.x;
            this->destRect.y = (int)this->transform->position.y;
        }
        void draw() override {
            TextureManager::Draw(this->texture, this->srcRect, this->destRect);
        }
};