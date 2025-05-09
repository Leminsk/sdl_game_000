#pragma once

#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"


class TextComponent : public Component {
    private:
        TransformComponent *transform;
        TransformComponent *camera_transform;
        SDL_Texture *texture;
        SDL_Rect srcRect;
        SDL_FRect destRect;

    public:
        double rotation = 0;
        int rotation_tick = 1;
        bool rotating = false;
        bool fixed = false;

        const char* font_path;
        const char* content;
        SDL_Color color = Game::default_text_color;
        float w, h;
        Vector2D offset = Vector2D(0.0f, 0.0f);
        
        

        TextComponent() = default;
        TextComponent(const char* text, bool fixed_to_screen=false) {
            this->fixed = fixed_to_screen;
            setText(text);
        }
        TextComponent(const char* text, Uint8 r, Uint8 g, Uint8 b, bool fixed_to_screen=false, const char* path=nullptr) {
            this->fixed = fixed_to_screen;
            this->font_path = path;
            this->color.r = r; this->color.g = g; this->color.b = b; this->color.a = SDL_ALPHA_OPAQUE;
            setText(text, path);
        }
        ~TextComponent() {
            SDL_DestroyTexture(this->texture);
        }

        void setOffset(float x_offset, float y_offset) {
            this->offset.x = x_offset;
            this->offset.y = y_offset;
        }

        void setText(const char* text, const char* path=nullptr) {
            this->content = text;
            int width, height;
            this->texture = TextureManager::LoadTextTexture(text, this->color, width, height, path);
            w = static_cast<float>(width);
            h = static_cast<float>(height);
        }

        void init() override {
            this->transform = &entity->getComponent<TransformComponent>();

            this->srcRect.x = 0;
            this->srcRect.y = 0;
            if(this->fixed) {
                this->srcRect.w = Game::SCREEN_WIDTH;
                this->srcRect.h = Game::SCREEN_HEIGHT;
                this->destRect.x = 0;
                this->destRect.y = 0;
                this->destRect.w = Game::SCREEN_WIDTH/4;
                this->destRect.h = Game::SCREEN_HEIGHT/16;
            } else {
                this->srcRect.w = static_cast<int>(this->w);
                this->srcRect.h = static_cast<int>(this->h);
            }
            
        }
        void update() override {
            if (this->rotating) {
                this->rotation = (static_cast<int>(this->rotation + this->rotation_tick) % 360);
            }

            if(!this->fixed) {
                camera_transform = &Game::camera.getComponent<TransformComponent>();
                Vector2D screen_pos = applyZoom(
                    *camera_transform, 
                    convertWorldToScreen(
                        camera_transform->position, 
                        AddVecs( this->transform->position, this->offset )
                    )
                );
                this->destRect.x = screen_pos.x;
                this->destRect.y = screen_pos.y;
                this->destRect.w = w * camera_transform->scale;
                this->destRect.h = h * camera_transform->scale;
            }
        }
        void draw() override {
            // similar to AABB collision, but the screen has position fixed to (0,0) as well as width and height fixed to the window's dimensions
            if(
                Game::SCREEN_WIDTH >= this->destRect.x &&
                this->destRect.x + this->destRect.w >= 0.0f &&
                Game::SCREEN_HEIGHT >= this->destRect.y &&
                this->destRect.y + this->destRect.h >= 0.0f
            ) {
                /*
                    destRect is the SCREEN coordinates
                    TransformComponent's position is the WORLD coordinates
                */
                TextureManager::DrawText(this->color, this->texture, this->srcRect, this->destRect, this->rotation, SDL_FLIP_NONE);
            }
        }
};