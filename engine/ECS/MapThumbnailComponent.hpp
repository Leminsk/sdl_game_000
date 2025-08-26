#pragma once

#include <SDL2/SDL.h>
#include "../utils.hpp"
#include "../Colors.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TextComponent.hpp"



class MapThumbnailComponent : public Component {
private:
float pixel_width = 1.0f;
float pixel_height = 1.0f;
bool use_texture = true;
SDL_Color border_color = Game::default_text_color;
SDL_Texture *map_texture = nullptr;

void setObjects(float pos_x, float pos_y, float w, float h, const std::string& name) {
    this->origin_x = pos_x;
    this->origin_y = pos_y;
    this->map_name = name;
    this->map_title = new TextComponent(map_name, pos_x, pos_y, Game::default_text_color, true);
    this->border_rect = {
        pos_x,
        pos_y + this->text_height + this->thumbnail_gap,
        w + this->double_thickness,
        h + this->double_thickness
    };
    this->map_rect = {
        this->border_rect.x + this->border_thickness, 
        this->border_rect.y + this->border_thickness,
        w, h
    };
    this->map_dimensions_subtitle = new TextComponent(
        std::to_string(this->map_width)+" x "+std::to_string(this->map_height),
        pos_x, 
        this->border_rect.y + this->border_rect.h + this->thumbnail_gap,
        Game::default_text_color, true
    );
}

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
bool selected = false;

MapThumbnailComponent(const std::string& map_dir, const std::string& map_name, float pos_x, float pos_y, float scale=1.0f) {
    const std::string file_path = map_dir+map_name+".bmp";
    if(getBMPProperties(file_path, &this->map_width, &this->map_height)) {
        this->map_texture = TextureManager::LoadTexture(file_path.c_str());
        const float scaled_size = this->thumbnail_side_size * scale;
        setObjects(pos_x, pos_y, scaled_size, scaled_size, map_name);
    } else {
        throw std::runtime_error("Failed to get BMP properties for MapThumbnail: " + file_path);
    }    
}
MapThumbnailComponent(const std::string& map_dir, const std::string& map_name, float pos_x, float pos_y, float width, float height) {
    const std::string file_path = map_dir+map_name+".bmp";
    if(getBMPPixels(file_path, this->map_pixels, &this->map_width, &this->map_height)) {
        this->use_texture = false;
        setObjects(pos_x, pos_y, width, height, map_name);
        this->pixel_width = width / static_cast<float>(this->map_width);
        this->pixel_height = height / static_cast<float>(this->map_height);
    } else {
        throw std::runtime_error("Failed to get BMP pixels for MapThumbnail: " + file_path);
    }
}
~MapThumbnailComponent() {
    if(this->map_title) {
        delete this->map_title;
        this->map_title = nullptr;
    }
    if(this->map_dimensions_subtitle) {
        delete this->map_dimensions_subtitle;
        this->map_dimensions_subtitle = nullptr;
    }
    if(this->map_texture) {
        SDL_DestroyTexture(this->map_texture);
        this->map_texture = nullptr;
    }
}

void init() override {
    this->map_title->init();
    this->map_dimensions_subtitle->init();
}
void update() override {

}
void draw() override {
    if(this->selected) {
        this->border_color = COLORS_MAGENTA;
        this->map_title->setColor(COLORS_MAGENTA);
        this->map_dimensions_subtitle->setColor(COLORS_MAGENTA);
    } else {
        this->border_color = Game::default_text_color;
        this->map_title->setColor(Game::default_text_color);
        this->map_dimensions_subtitle->setColor(Game::default_text_color);
    }
    this->map_title->draw();
    TextureManager::DrawRect(&this->border_rect, this->border_color);

    if(this->use_texture) {
        TextureManager::Draw(this->map_texture, NULL, &this->map_rect);
    } else {
        SDL_FRect pixel_rect = { 0.0f, 0.0f, this->pixel_width, this->pixel_height };
        int y, x;
        for(y=0; y<this->map_height; ++y) {
            pixel_rect.y = this->map_rect.y + (y * this->pixel_height);
            for(x=0; x<this->map_width; ++x) {
                pixel_rect.x = this->map_rect.x + (x * this->pixel_width);
                TextureManager::DrawRect(&pixel_rect, this->map_pixels[y][x]);
            }
        }
    }
    
    this->map_dimensions_subtitle->draw(); 
}
};