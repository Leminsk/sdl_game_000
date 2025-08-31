#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include <unordered_map>
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

// generate collision mesh for path finding where each element is either the original pixel value, or a subsection.
// out_mesh will be overwritten entirely with the new mesh
void generateCollisionMesh(const int subdivisions, std::vector<std::vector<uint8_t>>& out_mesh, int& out_width, int& out_height) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
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

    int row, column, shifted_row;
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

// generate collision mesh with fewer tiles than the original map. coalesce sets of tiles into a macro tile for easier path finding;
// return true on success, return false if can't coalesce in the map dimensions or macro_size;
// `macro_size` is the amount of pixels that will be coalesced into a new macro pixel. e.g. 4 means a grid of 2x2 from the original layout will become a single pixel in out_mesh
bool generateCollisionMacroMesh(const int macro_size, std::vector<std::vector<uint8_t>>& out_mesh, int& out_width, int& out_height) {
    int factor, inc;
    switch(macro_size) {
        case 16: factor = 2; break;
        case 4:  factor = 1; break;
        case 1:
        default: factor = 0; break;
    }
    inc = (1 << factor);
    if((this->layout_height % inc != 0) || (this->layout_width % inc != 0)) {
        std::cout << "Cannot generate MacroMesh with macro_size=" << macro_size << " - Aborting...\n";
        return false;
    }
    out_height = this->layout_height >> factor;
    out_width = this->layout_width >> factor;   

    out_mesh.clear();
    out_mesh.resize(out_height, {});

    int row, column;
    std::unordered_map<uint8_t, int> tile_counter = {
        {tile_type::TILE_BASE_SPAWN, 0},
        {tile_type::TILE_IMPASSABLE, 0},
        {tile_type::TILE_NAVIGABLE, 0},
        {tile_type::TILE_PLAIN, 0},
        {tile_type::TILE_PLAYER, 0},
        {tile_type::TILE_ROUGH, 0}
    };
    std::vector<uint8_t> tiles;
    switch(inc) {
        case 4: {
            tiles = { 
                0, 0, 0, 0, 
                0, 0, 0, 0, 
                0, 0, 0, 0,
                0, 0, 0, 0
            };
            for(row=0; row<this->layout_height; row += inc) {
                out_mesh[row].reserve(out_width);
                tile_counter[tile_type::TILE_BASE_SPAWN] = 0;
                tile_counter[tile_type::TILE_IMPASSABLE] = 0;
                tile_counter[tile_type::TILE_NAVIGABLE] = 0;
                tile_counter[tile_type::TILE_PLAIN] = 0;
                tile_counter[tile_type::TILE_PLAYER] = 0;
                tile_counter[tile_type::TILE_ROUGH] = 0;
                for(column=0; column<this->layout_width; column += inc) {
                    int column1 = column+1; int column2 = column+2; int column3 = column+3;
                    tiles[ 0] = this->layout[row  ][column]; tiles[ 1] = this->layout[row  ][column1]; tiles[ 2] = this->layout[row  ][column2]; tiles[ 3] = this->layout[row  ][column3];
                    tiles[ 4] = this->layout[row+1][column]; tiles[ 5] = this->layout[row+1][column1]; tiles[ 6] = this->layout[row+1][column2]; tiles[ 7] = this->layout[row+1][column3];
                    tiles[ 8] = this->layout[row+2][column]; tiles[ 9] = this->layout[row+2][column1]; tiles[10] = this->layout[row+2][column2]; tiles[11] = this->layout[row+2][column3];
                    tiles[12] = this->layout[row+3][column]; tiles[13] = this->layout[row+3][column1]; tiles[14] = this->layout[row+3][column2]; tiles[15] = this->layout[row+3][column3];
                    for(uint8_t& t : tiles) { ++tile_counter[t]; }
                    uint8_t max_counter_type = tile_type::TILE_BASE_SPAWN;
                    int current_max = 0;
                    for(auto& [t_type, cnt_val]: tile_counter) {
                        if(cnt_val > current_max) { 
                            current_max = cnt_val;
                            max_counter_type = t_type;    
                        }
                    }
                    out_mesh[row].push_back( max_counter_type );
                }
            }
        } break;

        case 2: {
            tiles = { 
                0, 0, 
                0, 0 
            };
            for(row=0; row<this->layout_height; row += inc) {
                out_mesh[row].reserve(out_width);
                tile_counter[tile_type::TILE_BASE_SPAWN] = 0;
                tile_counter[tile_type::TILE_IMPASSABLE] = 0;
                tile_counter[tile_type::TILE_NAVIGABLE] = 0;
                tile_counter[tile_type::TILE_PLAIN] = 0;
                tile_counter[tile_type::TILE_PLAYER] = 0;
                tile_counter[tile_type::TILE_ROUGH] = 0;
                for(column=0; column<this->layout_width; column += inc) {
                    tiles[0] = this->layout[row  ][column]; tiles[1] = this->layout[row  ][column+1];
                    tiles[2] = this->layout[row+1][column]; tiles[3] = this->layout[row+1][column+1];
                    for(uint8_t& t : tiles) { ++tile_counter[t]; }
                    uint8_t max_counter_type = tile_type::TILE_BASE_SPAWN;
                    int current_max = 0;
                    for(auto& [t_type, cnt_val]: tile_counter) {
                        if(cnt_val > current_max) { 
                            current_max = cnt_val;
                            max_counter_type = t_type;    
                        }
                    }
                    out_mesh[row].push_back( max_counter_type );
                }
            }
        } break;

        case 1:
        default:
            // just copy it
            for(row=0; row<out_height; ++row) {
                out_mesh[row].reserve(out_width);
                for(column=0; column<out_width; ++column) {
                    out_mesh[row].push_back( this->layout[row][column] );
                }
            }
    }
    
    
    return true;
}

};