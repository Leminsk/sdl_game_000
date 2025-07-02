#pragma once
#include <string>
#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"

// Text with background surrounded by a border
class TextBoxComponent : public Component {
    private:
        TransformComponent *transform;
        SDL_Texture *texture = NULL;
        SDL_Rect srcRect;
        SDL_FRect destRect;
        SDL_FRect bgRect;
        SDL_FRect borderRect;

    public:
        double rotation = 0;
        int rotation_tick = 1;
        bool rotating = false;

        int border_thickness;

        const char* font_path;
        std::string content;
        SDL_Color text_color, bg_color, border_color;
        SDL_Color mouse_down_text_color, mouse_down_bg_color, mouse_down_border_color;
        bool mouse_down = false;
        float w = 0.0f; float inner_w;
        float h = 0.0f; float inner_h;
        float x = 0.0f; float inner_x;
        float y = 0.0f; float inner_y;
        
        
        
        TextBoxComponent(
            const std::string& text, 
            float pos_x, float pos_y, float width, float height, 
            const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c,
            int border_thickness, const char* path=nullptr
        ) {
            this->border_thickness = border_thickness;
            this->x = pos_x; this->y = pos_y;
            this->w = width; this->h = height;
            this->inner_x = pos_x + border_thickness;
            this->inner_y = pos_y + border_thickness;
            this->inner_w = width - (border_thickness + border_thickness);
            this->inner_h = height - (border_thickness + border_thickness);

            this->text_color = t_c; 
            this->mouse_down_text_color = { static_cast<uint8_t>(t_c.r>>1), static_cast<uint8_t>(t_c.g>>1), static_cast<uint8_t>(t_c.b>>1), SDL_ALPHA_OPAQUE };
            this->bg_color = bg_c; 
            this->mouse_down_bg_color = { static_cast<uint8_t>(bg_c.r>>1), static_cast<uint8_t>(bg_c.g>>1), static_cast<uint8_t>(bg_c.b>>1), SDL_ALPHA_OPAQUE };
            this->border_color = b_c; 
            this->mouse_down_border_color = { static_cast<uint8_t>(b_c.r>>1), static_cast<uint8_t>(b_c.g>>1), static_cast<uint8_t>(b_c.b>>1), SDL_ALPHA_OPAQUE };

            this->font_path = path;            
            setText(text, path);
        }
        ~TextBoxComponent() {
            SDL_DestroyTexture(this->texture);
        }

        void setText(std::string text="", const char* path=nullptr) {
            if(this->texture != NULL) { SDL_DestroyTexture(this->texture); }
            if(text == "") { text = "PLACEHOLDER"; }
            this->content = text;
            int width, height;
            this->texture = TextureManager::LoadTextTexture(text.c_str(), this->text_color, width, height, path);
        }
        void setRenderRects(float x, float y, float width, float height) {
            this->x = x; this->y = y;
            this->w = width; this->h = height;
            this->inner_x = x + this->border_thickness; this->inner_y = y + this->border_thickness;
            this->inner_w = width - (this->border_thickness + this->border_thickness); this->inner_h = height - (this->border_thickness + this->border_thickness);

            this->srcRect.x = 0; this->srcRect.y = 0;
            this->srcRect.w = Game::SCREEN_WIDTH; this->srcRect.h = Game::SCREEN_HEIGHT;
            
            this->destRect.x = this->inner_x + 1; this->destRect.y = this->inner_y + 1;
            this->destRect.w = this->inner_w - 2; this->destRect.h = this->inner_h - 2;

            this->bgRect.x = this->inner_x; this->bgRect.y = this->inner_y;
            this->bgRect.w = this->inner_w; this->bgRect.h = this->inner_h;

            this->borderRect.x = this->x; this->borderRect.y = this->y;
            this->borderRect.w = this->w; this->borderRect.h = this->h;
        }

        void init() override {
            this->srcRect.x = 0; this->srcRect.y = 0;
            this->srcRect.w = Game::SCREEN_WIDTH; this->srcRect.h = Game::SCREEN_HEIGHT;
            
            this->destRect.x = this->inner_x + 1; this->destRect.y = this->inner_y + 1;
            this->destRect.w = this->inner_w - 2; this->destRect.h = this->inner_h - 2;

            this->bgRect.x = this->inner_x; this->bgRect.y = this->inner_y;
            this->bgRect.w = this->inner_w; this->bgRect.h = this->inner_h;

            this->borderRect.x = this->x; this->borderRect.y = this->y;
            this->borderRect.w = this->w; this->borderRect.h = this->h;
        }
        void update() override {
            if (this->rotating) {
                this->rotation = (static_cast<int>(this->rotation + this->rotation_tick) % 360);
            }
        }
        void draw() override {
            // UI elements are shown on screen, thus drawn by default
            if(this->mouse_down) {
                TextureManager::DrawRect(&this->borderRect, this->mouse_down_border_color);
                TextureManager::DrawRect(&this->bgRect, this->mouse_down_bg_color);  // drawing over the background is faster than making a special routine for it
                // srcRect is the entire screen, destRect is the inner portion of TextBox
                TextureManager::DrawText(this->mouse_down_text_color, this->texture, this->srcRect, this->destRect, this->rotation, SDL_FLIP_NONE);
            } else {
                TextureManager::DrawRect(&this->borderRect, this->border_color);
                TextureManager::DrawRect(&this->bgRect, this->bg_color);
                TextureManager::DrawText(this->text_color, this->texture, this->srcRect, this->destRect, this->rotation, SDL_FLIP_NONE);
            }
            
        }
};