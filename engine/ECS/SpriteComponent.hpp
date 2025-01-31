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
        ~SpriteComponent() {
            SDL_DestroyTexture(this->texture);
        }

        void setTex(const char* path) {
            this->texture = TextureManager::LoadTexture(path);
        }

        void init() override {
            this->transform = &entity->getComponent<TransformComponent>();

            this->srcRect.x = 0;
            this->srcRect.y = 0;
            this->srcRect.w = this->transform->width;
            this->srcRect.h = this->transform->height;
        }
        void update() override {
            this->destRect.x = (int)this->transform->position.x;
            this->destRect.y = (int)this->transform->position.y;

            this->destRect.w = this->transform->width * this->transform->scale;
            this->destRect.h = this->transform->height * this->transform->scale;
        }
        void draw() override {
            TextureManager::Draw(this->texture, this->srcRect, this->destRect);
        }
};