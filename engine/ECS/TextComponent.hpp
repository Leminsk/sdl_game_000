#pragma once
#include <string>
#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"


class TextComponent : public Component {
    private:
        TransformComponent *transform;
        SDL_Texture *texture = NULL;
        SDL_Rect srcRect;
        SDL_FRect destRect;

    public:
        double rotation = 0;
        int rotation_tick = 1;
        bool rotating = false;
        bool fixed = false;

        const char* font_path;
        std::string content;
        SDL_Color color = Game::default_text_color;
        float w = 0.0f;
        float h = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        Vector2D offset = Vector2D(0.0f, 0.0f);
        
        

        TextComponent() = default;
        TextComponent(std::string text, bool fixed_to_screen=false) {
            this->fixed = fixed_to_screen;
            this->w = Game::SCREEN_WIDTH/3;
            this->h = Game::SCREEN_HEIGHT/16;
            setText(text);
        }
        TextComponent(
            std::string text, float pos_x, float pos_y, float width, float height, SDL_Color color=Game::default_text_color, 
            bool fixed_to_screen=false, const char* path=nullptr
        ) {
            this->x = pos_x;
            this->y = pos_y;
            this->w = width;
            this->h = height;
            this->fixed = fixed_to_screen;
            this->font_path = path;
            this->color.r = color.r; this->color.g = color.g; this->color.b = color.b; this->color.a = SDL_ALPHA_OPAQUE;
            setText(text, path);
        }
        ~TextComponent() {
            SDL_DestroyTexture(this->texture);
        }

        void setOffset(const Vector2D& v) {
            this->offset = v;
        }

        void setOffset(float x_offset, float y_offset) {
            this->offset.x = x_offset;
            this->offset.y = y_offset;
        }

        void setText(std::string text="", const char* path=nullptr) {
            if(this->texture != NULL) { SDL_DestroyTexture(this->texture); }
            if(text == "") { text = "PLACEHOLDER"; }
            this->content = text;
            int width, height;
            this->texture = TextureManager::LoadTextTexture(text.c_str(), this->color, width, height, path);
        }

        void init() override {
            this->srcRect.x = 0;
            this->srcRect.y = 0;
            if(this->fixed) {
                this->srcRect.w = Game::SCREEN_WIDTH;
                this->srcRect.h = Game::SCREEN_HEIGHT;
                // destRect doesn't have to be updated
                this->destRect.x = this->x; this->destRect.y = this->y;
                this->destRect.w = this->w; this->destRect.h = this->h;
            } else {
                this->transform = &entity->getComponent<TransformComponent>();
                this->srcRect.w = static_cast<int>(this->w);
                this->srcRect.h = static_cast<int>(this->h);
            }            
        }
        void update() override {
            if (this->rotating) {
                this->rotation = (static_cast<int>(this->rotation + this->rotation_tick) % 360);
            }
        }
        void draw() override {
            if(!this->fixed) {
                TransformComponent& camera_transform = Game::camera.getComponent<TransformComponent>();
                Vector2D screen_pos = convertWorldToScreen(this->transform->position + this->offset);
                this->destRect.x = screen_pos.x;
                this->destRect.y = screen_pos.y;
                this->destRect.w = this->w * camera_transform.scale;
                this->destRect.h = this->h * camera_transform.scale;
            }
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