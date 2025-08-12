#pragma once

#include <SDL2/SDL.h>
#include "../utils.hpp"
#include "../Colors.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TextComponent.hpp"



class MapThumbnailComponent : public Component {
private:
const float thumbnail_pixel_size = 5.0f;
const int thumbnail_gap = 4;
const int border_thickness = 2;
const int double_thickness = this->border_thickness<<1;
SDL_FRect border_rect;
SDL_Color border_color = Game::default_text_color;
TextComponent *map_title;
TextComponent *map_dimensions_subtitle;

public:
uint32_t map_width, map_height;
std::vector<std::vector<SDL_Color>> map_pixels = {};
std::string map_name = "Placeholder";
SDL_FRect map_pixel_rect = { 0.0f, 0.0f, this->thumbnail_pixel_size, this->thumbnail_pixel_size };
float origin_x, origin_y;

MapThumbnailComponent(const std::string& map_dir, const std::string& map_name, float pos_x, float pos_y) {
    getPixelsBMP(map_dir+map_name+".bmp", this->map_pixels, &this->map_width, &this->map_height);
    this->origin_x = pos_x;
    this->origin_y = pos_y;

    this->map_name = map_name;
    this->border_rect = {
        pos_x - this->border_thickness, 
        pos_y - this->border_thickness,
        static_cast<float>(( this->map_width * this->thumbnail_pixel_size) + this->double_thickness), 
        static_cast<float>((this->map_height * this->thumbnail_pixel_size) + this->double_thickness),
    };

    this->map_title = new TextComponent(
        map_name, 
        pos_x, pos_y - (this->thumbnail_gap + 32), 
        Game::default_text_color, true
    );
    this->map_dimensions_subtitle = new TextComponent(
        std::to_string(this->map_width)+" x "+std::to_string(this->map_height),
        pos_x, static_cast<float>(pos_y + (this->map_height * this->thumbnail_pixel_size) + this->thumbnail_gap),
        Game::default_text_color, true
    );
}
~MapThumbnailComponent() {}

void init() override {
    this->map_title->init();
    this->map_dimensions_subtitle->init();
}
void update() override {

}
void draw() override {
    this->map_title->draw();
    TextureManager::DrawRect(&this->border_rect, this->border_color);
    uint32_t y, x;
    for(y=0; y<this->map_height; ++y) {
        this->map_pixel_rect.y = this->origin_y + (static_cast<float>(y) * this->thumbnail_pixel_size);
        for(x=0; x<this->map_width; ++x) {
            this->map_pixel_rect.x = this->origin_x + (static_cast<float>(x) * this->thumbnail_pixel_size);
            TextureManager::DrawRect(&this->map_pixel_rect, this->map_pixels[y][x]);
        }                
    }
    this->map_dimensions_subtitle->draw(); 
}
};