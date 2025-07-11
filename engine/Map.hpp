#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
#include "utils.hpp"
#include "Vector2D.hpp"
#include "Game.hpp"
#include "TextureManager.hpp"
#include "ECS/TileTypes.hpp"

class Map {
    public:
        int tile_width; // in pixels
        bool loaded;
        std::vector<std::vector<int>> layout;
        uint32_t layout_width, layout_height;
        float world_layout_width, world_layout_height;

        SDL_Texture* plain_terrain_texture;
        SDL_Texture* rough_terrain_texture;
        SDL_Texture* mountain_texture;
        SDL_Texture* water_bg_texture;
        SDL_Texture* water_fg_texture;

        Map(
            std::string path, 
            SDL_Texture* plain, 
            SDL_Texture* rough,
            SDL_Texture* mountain,
            SDL_Texture* water_bg,
            SDL_Texture* water_fg,
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
            float x = floorf(world_pos.x / tile_width);
            float y = floorf(world_pos.y / tile_width);
            return this->layout[x][y];
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