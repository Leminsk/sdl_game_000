#pragma once
#include <string>
#include <vector>
#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"

// Text with background surrounded by a border
class TextBoxComponent : public Component {
    private:
        SDL_Texture *texture = NULL;
        std::vector<SDL_Texture*> textures = {};
        std::vector<int> width_per_line = {};
        SDL_Rect srcRect;
        bool single_line = true;
        int lines_amount = 1;
        int line_thickness;

        const int v_line_gap = 2;
        const int h_line_gap = 2;
        const int v_char_height = 32;
        const int h_char_width = 16;

        void setProportions(int pos_x, int pos_y, const std::vector<std::string>& text_lines, int border_thickness) {
            const int double_thickness = border_thickness<<1;
            int max_char_on_line = 0;
            for(std::string line : text_lines) {
                if(line.size() > max_char_on_line) { max_char_on_line = line.size(); }
            }
            const int total_width = (max_char_on_line * this->h_char_width) + (this->h_line_gap<<1) + double_thickness;
            const int total_height = (text_lines.size() * (this->v_char_height + this->v_line_gap)) + this->v_line_gap + double_thickness;

            this->border_thickness = border_thickness;

            this->x = pos_x < 0 ? ((Game::SCREEN_WIDTH - total_width) + pos_x) : pos_x; 
            this->y = pos_y < 0 ? ((Game::SCREEN_HEIGHT - total_height) + pos_y) : pos_y;
            this->w = total_width; 
            this->h = total_height;

            this->inner_x = this->x + border_thickness;
            this->inner_y = this->y + border_thickness;
            this->inner_w = total_width - double_thickness;
            this->inner_h = total_height - double_thickness;
        }

        void setColors(const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c) {
            this->text_color = t_c;
            this->bg_color = bg_c;
            this->border_color = b_c;
            this->mouse_down_text_color   = { static_cast<uint8_t>( t_c.r>>1), static_cast<uint8_t>( t_c.g>>1), static_cast<uint8_t>( t_c.b>>1), SDL_ALPHA_OPAQUE };
            this->mouse_down_bg_color     = { static_cast<uint8_t>(bg_c.r>>1), static_cast<uint8_t>(bg_c.g>>1), static_cast<uint8_t>(bg_c.b>>1), SDL_ALPHA_OPAQUE };
            this->mouse_down_border_color = { static_cast<uint8_t>( b_c.r>>1), static_cast<uint8_t>( b_c.g>>1), static_cast<uint8_t>( b_c.b>>1), SDL_ALPHA_OPAQUE };
        }

    public:
        SDL_FRect destRect;
        SDL_FRect bgRect;
        SDL_FRect borderRect;

        double rotation = 0;
        int rotation_tick = 1;
        bool rotating = false;

        int border_thickness;

        const char* font_path;
        SDL_Color text_color, bg_color, border_color;
        SDL_Color mouse_down_text_color, mouse_down_bg_color, mouse_down_border_color;
        bool mouse_down = false;
        float w = 0.0f; float inner_w;
        float h = 0.0f; float inner_h;
        float x = 0.0f; float inner_x;
        float y = 0.0f; float inner_y;
        
        
        TextBoxComponent(
            const std::string& text, 
            float pos_x, float pos_y,
            const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c,
            int border_thickness=3, const char* path=nullptr
        ) {
            setProportions(pos_x, pos_y, { text }, border_thickness);
            setColors(t_c, bg_c, b_c);
            this->font_path = path;            
            setText(text, path);
        }
        TextBoxComponent(
            const std::vector<std::string>& text_lines, 
            float pos_x, float pos_y,
            const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c,
            int border_thickness=3, const char* path=nullptr
        ) {
            setProportions(pos_x, pos_y, text_lines, border_thickness);
            setColors(t_c, bg_c, b_c);
            this->font_path = path;            
            this->single_line = false;
            this->lines_amount = text_lines.size();
            setMultiText(text_lines, path);
        }
        ~TextBoxComponent() {
            SDL_DestroyTexture(this->texture);
            for(SDL_Texture* t : this->textures) {
                if(t != NULL) { SDL_DestroyTexture(t); }
            }
        }

        void setText(std::string text="", const char* path=nullptr) {
            if(this->texture != NULL) { SDL_DestroyTexture(this->texture); }
            if(text == "") { text = "PLACEHOLDER"; }
            int width, height;
            this->texture = TextureManager::LoadTextTexture(text.c_str(), this->text_color, width, height, path);
        }
        void setMultiText(const std::vector<std::string>& lines, const char* path=nullptr) {
            for(SDL_Texture* t : this->textures) {
                if(t != NULL) { SDL_DestroyTexture(t); }
            }
            this->width_per_line.clear();
            this->width_per_line.shrink_to_fit();
            this->textures.clear();
            this->textures.shrink_to_fit();
            for(std::string line : lines) {
                this->width_per_line.push_back(line.size() * this->h_char_width);
                int width, height;
                this->textures.push_back( TextureManager::LoadTextTexture(line.c_str(), this->text_color, width, height, path) );
            }
        }
        void setRenderRects(float x, float y, float width, float height) {
            this->x = x; this->y = y;
            this->w = width; this->h = height;
            this->inner_x = x + this->border_thickness; this->inner_y = y + this->border_thickness;
            this->inner_w = width - (this->border_thickness + this->border_thickness); this->inner_h = height - (this->border_thickness + this->border_thickness);

            this->srcRect.x = 0; this->srcRect.y = 0;
            this->srcRect.w = Game::SCREEN_WIDTH; this->srcRect.h = Game::SCREEN_HEIGHT;

            this->borderRect.x = this->x; this->borderRect.y = this->y;
            this->borderRect.w = this->w; this->borderRect.h = this->h;

            this->bgRect.x = this->inner_x; this->bgRect.y = this->inner_y;
            this->bgRect.w = this->inner_w; this->bgRect.h = this->inner_h;

            // always used for 1 line at a time
            this->destRect.x = this->inner_x + this->v_line_gap; 
            this->destRect.y = this->inner_y + this->v_line_gap;
            this->destRect.w = this->inner_w - (this->v_line_gap<<1);
            if(this->single_line) {
                this->destRect.h = this->inner_h - (this->v_line_gap<<1);
            } else {           
                                                         // top gap
                this->line_thickness = ((this->inner_h - this->v_line_gap) / this->lines_amount);
                this->destRect.h = this->line_thickness - this->v_line_gap; // bottom/inner gaps
            }
        }

        void init() override {
            this->srcRect.x = 0; this->srcRect.y = 0;
            this->srcRect.w = Game::SCREEN_WIDTH; this->srcRect.h = Game::SCREEN_HEIGHT;

            this->borderRect.x = this->x; this->borderRect.y = this->y;
            this->borderRect.w = this->w; this->borderRect.h = this->h;

            this->bgRect.x = this->inner_x; this->bgRect.y = this->inner_y;
            this->bgRect.w = this->inner_w; this->bgRect.h = this->inner_h;
            
            // always used for 1 line at a time
            this->destRect.x = this->inner_x + this->v_line_gap; 
            this->destRect.y = this->inner_y + this->v_line_gap;
            this->destRect.w = this->inner_w - (this->v_line_gap<<1);
            
            if(this->single_line) {
                this->destRect.h = this->inner_h - (this->v_line_gap<<1);
            } else {           
                                                         // top gap
                this->line_thickness = ((this->inner_h - this->v_line_gap) / this->lines_amount);
                this->destRect.h = this->line_thickness - this->v_line_gap; // bottom/inner gaps
            }
            
            
        }
        void update() override {
            if (this->rotating) {
                this->rotation = (static_cast<int>(this->rotation + this->rotation_tick) % 360);
            }
        }
        void draw() override {
            // UI elements are shown on screen, thus drawn by default
            SDL_Color color1 = this->border_color;
            SDL_Color color2 = this->bg_color;
            SDL_Color color3 = this->text_color;
            if(this->mouse_down) { 
                color1 = this->mouse_down_border_color;
                color2 = this->mouse_down_bg_color;
                color3 = this->mouse_down_text_color;
            }

            TextureManager::DrawRect(&this->borderRect, color1);
            TextureManager::DrawRect(&this->bgRect, color2);  // drawing over the background is faster than making a special routine for it

            if(this->single_line) {
                // srcRect is the entire screen, destRect is the inner portion of TextBox
                TextureManager::DrawText(color3, this->texture, &this->srcRect, &this->destRect, this->rotation, SDL_FLIP_NONE);
            } else {
                for(int i=0; i<this->lines_amount; ++i) {
                    this->destRect.w = this->width_per_line[i];
                    TextureManager::DrawText(color3, this->textures[i], &this->srcRect, &this->destRect, this->rotation, SDL_FLIP_NONE);
                    this->destRect.y += this->line_thickness;
                }
                this->destRect.y = this->inner_y + this->v_line_gap;
            }
        }
};