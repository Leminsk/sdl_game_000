#pragma once

#include "../TextureManager.hpp"

#include "SpriteAnimation.hpp"
#include "Components.hpp"

#include <SDL2/SDL.h>

#include <map>

class SpriteComponent : public Component {
    private:
        TransformComponent *transform;
        SDL_Texture *texture;
        SDL_Rect srcRect, destRect;

        bool animated = false;
        int frames = 0;
        int delay = 100;

    public:
        int animationIndex = 0;
        double rotation = 0;
        int rotation_tick = 2;
        bool rotating = false;

        SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;
        std::map<const char*, SpriteAnimation> animations;

        SpriteComponent() = default;
        SpriteComponent(const char* path) {
            setTex(path);
        }
        SpriteComponent(const char* path, bool isAnimated) {
            this->animated = isAnimated;

            SpriteAnimation idle = SpriteAnimation(0, 3, 100);

            animations.emplace("idle", idle);

            play("idle");
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
            this->srcRect.y = this->animationIndex * this->transform->height;

            if (this->animated) {
                srcRect.x = srcRect.w * static_cast<int>( (SDL_GetTicks() / delay) % this->frames );
            }

            if (this->rotating) {
                this->rotation = static_cast<int>(this->rotation + this->rotation_tick) % 360;
            }

            this->destRect.x = static_cast<int>(this->transform->position.x);
            this->destRect.y = static_cast<int>(this->transform->position.y);

            this->destRect.w = this->transform->width * this->transform->scale;
            this->destRect.h = this->transform->height * this->transform->scale;
        }
        void draw() override {
            TextureManager::Draw(this->texture, this->srcRect, this->destRect, this->rotation, this->spriteFlip);
        }
        void play(const char* name) {
            this->animationIndex = this->animations[name].index;
            this->frames = this->animations[name].frames;
            this->delay = this->animations[name].delay;
        }
};