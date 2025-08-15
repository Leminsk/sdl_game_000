#pragma once

#include <SDL2/SDL.h>
#include "../utils.hpp"
#include "../Colors.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TextComponent.hpp"



class MapThumbnailComponent : public Component {
private:
SDL_Color border_color = Game::default_text_color;
SDL_Texture *map_texture;


public:
const int text_height = 32;
const float thumbnail_side_size = 200.0f;
const int thumbnail_gap = 4;
const int border_thickness = 2;
const int double_thickness = this->border_thickness<<1;
uint32_t map_width, map_height;
std::vector<std::vector<SDL_Color>> map_pixels = {};
std::string map_name = "Placeholder";
SDL_FRect map_rect;
SDL_FRect border_rect;
float origin_x, origin_y;
TextComponent *map_title;
TextComponent *map_dimensions_subtitle;

MapThumbnailComponent(const std::string& map_dir, const std::string& map_name, float pos_x, float pos_y) {
    const std::string file_path = map_dir+map_name+".bmp";
    this->map_texture = TextureManager::LoadTexture(file_path.c_str());
    getBMPProperties(file_path, &this->map_width, &this->map_height);
    this->origin_x = pos_x;
    this->origin_y = pos_y;
    this->map_name = map_name;
    this->map_title = new TextComponent(map_name, pos_x, pos_y, Game::default_text_color, true);
    this->border_rect = {
        pos_x,
        pos_y + this->text_height + this->thumbnail_gap,
        this->thumbnail_side_size + this->double_thickness,
        this->thumbnail_side_size + this->double_thickness
    };
    this->map_rect = {
        this->border_rect.x + this->border_thickness, 
        this->border_rect.y + this->border_thickness,
        this->thumbnail_side_size, this->thumbnail_side_size
    };
    this->map_dimensions_subtitle = new TextComponent(
        std::to_string(this->map_width)+" x "+std::to_string(this->map_height),
        pos_x, 
        this->border_rect.y + this->border_rect.h + this->thumbnail_gap,
        Game::default_text_color, true
    );
}
~MapThumbnailComponent() {
    SDL_DestroyTexture(this->map_texture);
}

void init() override {
    this->map_title->init();
    this->map_dimensions_subtitle->init();
}
void update() override {

}
void draw() override {
    this->map_title->draw();
    TextureManager::DrawRect(&this->border_rect, this->border_color);
    TextureManager::Draw(this->map_texture, NULL, &this->map_rect);
    this->map_dimensions_subtitle->draw(); 
}
};