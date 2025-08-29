#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include "utils.hpp"
#include "Vector2D.hpp"
#include "Game.hpp"
#include "TextureManager.hpp"
#include "Colors.hpp"
#include "ECS/TileTypes.hpp"

class Map {
public:
int tile_width; // in pixels
bool loaded;
std::vector<std::vector<int>> layout;
std::vector<std::vector<SDL_Color>> map_pixels;
uint32_t layout_width, layout_height;
float world_layout_width, world_layout_height;

SDL_Texture* plain_terrain_texture;
SDL_Texture* rough_terrain_texture;
SDL_Texture* mountain_texture;
SDL_Texture* water_bg_texture;
SDL_Texture* water_fg_texture;

Map(
    const std::vector<std::vector<SDL_Color>>& pixels, 
    SDL_Texture* plain, SDL_Texture* rough, SDL_Texture* mountain, SDL_Texture* water_bg, SDL_Texture* water_fg,
    int dimension=1
) {
    this->plain_terrain_texture = plain;
    this->rough_terrain_texture = rough;
    this->mountain_texture = mountain;
    this->water_bg_texture = water_bg;
    this->water_fg_texture = water_fg;
    this->tile_width = dimension;

    this->map_pixels = pixels;
    this->layout_height = pixels.size();
    this->layout_width = pixels[0].size();

    this->world_layout_height = this->layout_height * this->tile_width;
    this->world_layout_width = this->layout_width * this->tile_width;
    
    this->layout.resize(this->layout_height);
    uint32_t x, y;    tile_type t;    SDL_Color c;
    for(y=0; y<this->layout_height; ++y) {
        this->layout[y].resize(this->layout_width);
        for(x=0; x<this->layout_width; ++x) {
            c = this->map_pixels[y][x];
            if(isSameColor(c, COLORS_SPAWN)) { // base spawn
                t = tile_type::TILE_BASE_SPAWN;
            } else if(isSameColor(c, COLORS_NAVIGABLE)) { // navigable terrain
                t = tile_type::TILE_NAVIGABLE;
            } else if(isSameColor(c, COLORS_IMPASSABLE)) { // impassable terrain
                t = tile_type::TILE_IMPASSABLE;
            } else if(isSameColor(c, COLORS_ROUGH)) { // rough terrain
                t = tile_type::TILE_ROUGH;
            } else if(isSameColor(c, COLORS_PLAIN)) { // plain terrain
                t = tile_type::TILE_PLAIN;
            } else { // selected player spawn
                t = tile_type::TILE_PLAYER;
            }
            this->layout[y][x] = t;
        }
    }
    this->loaded = true;
}

Map(
    std::string path, 
    SDL_Texture* plain, SDL_Texture* rough, SDL_Texture* mountain, SDL_Texture* water_bg, SDL_Texture* water_fg,
    int dimension=1
) {
    this->plain_terrain_texture = plain;
    this->rough_terrain_texture = rough;
    this->mountain_texture = mountain;
    this->water_bg_texture = water_bg;
    this->water_fg_texture = water_fg;
    this->tile_width = dimension;
    this->loaded = LoadMapFile(path);
}

~Map() {}

bool LoadMapFile(std::string& path);

int getTileFromWorldPos(Vector2D world_pos) {
    uint32_t x = static_cast<uint32_t>(floorf(world_pos.x / tile_width));
    uint32_t y = static_cast<uint32_t>(floorf(world_pos.y / tile_width));
    return this->layout[y][x];
}

Vector2D getWorldPosFromTileCoord(uint32_t x, uint32_t y) {
    return Vector2D(
        (x * this->tile_width) + this->tile_width/2,
        (y * this->tile_width) + this->tile_width/2
    );
}

// generate collision mesh for path finding where true values are IMPASSABLE tiles.
// out_mesh will be overwritten entirely with the new mesh
void generateCollisionMesh(const int subdivisions, std::vector<std::vector<uint8_t>>& out_mesh, int& out_width, int& out_height) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    int row, column, shifted_row;
    int factor;
    switch(subdivisions) {
        case 64: factor = 3; break;
        case 16: factor = 2; break;
        case 4:  factor = 1; break;
        case 1: 
        default: factor = 0; break;
    }
    out_height = this->layout_height << factor;
    out_width = this->layout_width << factor;

    out_mesh.clear();
    out_mesh.resize(out_height, {});
    for(row=0; row<out_height; ++row) {
        out_mesh[row].reserve(out_width);
        shifted_row = row>>factor;
        for(column=0; column<out_width; ++column) {
            out_mesh[row].push_back( this->layout[ shifted_row ][ column>>factor ] );
        }
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Collision Mesh Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
}

};