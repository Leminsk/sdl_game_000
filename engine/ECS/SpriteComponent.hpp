#pragma once

#include <map>
#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "SpriteAnimation.hpp"


class SpriteComponent : public Component {
    private:
        TransformComponent *transform;
        SDL_Texture *texture;
        SDL_Rect srcRect;
        SDL_FRect destRect;

        bool animated = false;
        int frames = 0;
        int delay = 100;

    public:
        int animationIndex = 0;
        double rotation = 0;
        int rotation_tick = 1;
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
            this->srcRect.w = static_cast<int>(this->transform->width);
            this->srcRect.h = static_cast<int>(this->transform->height);
        }
        void update() override {
            this->srcRect.y = this->animationIndex * static_cast<int>(this->transform->height);

            if (this->animated) {
                srcRect.x = srcRect.w * static_cast<int>( (SDL_GetTicks() / delay) % this->frames );
            }

            if (this->rotating) {
                this->rotation = (static_cast<int>(this->rotation + this->rotation_tick) % 360);
            }

            Vector2D screen_pos = convertWorldToScreen(
                Game::camera.getComponent<TransformComponent>().position, 
                this->transform->position
            );
            this->destRect.x = screen_pos.x;
            this->destRect.y = screen_pos.y;
            this->destRect.w = this->transform->width;
            this->destRect.h = this->transform->height;
        }
        void draw() override {
            /*
                I THINK that for our purposes, the destRect is the SCREEN coordinates
                while whatever TransformComponent has as its position was the game WORLD coordinates.
                So ideally there must be some sort of transformation/translation layer when passing one to the other (WORLD -> SCREEN)
            */ 
            TextureManager::Draw(this->texture, this->srcRect, this->destRect, this->rotation, this->spriteFlip);
        }
        void play(const char* name) {
            this->animationIndex = this->animations[name].index;
            this->frames = this->animations[name].frames;
            this->delay = this->animations[name].delay;
        }
};