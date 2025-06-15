#pragma once
#include <chrono>
#include <vector>
#include <string>
#include <cmath>
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

        SDL_Texture* dirt_texture     = TextureManager::LoadTexture("assets/tiles/dirt.png");
        SDL_Texture* mountain_texture = TextureManager::LoadTexture("assets/tiles/mountain.png");
        SDL_Texture* water_texture    = TextureManager::LoadTexture("assets/tiles/water.png");
        SDL_Texture* grass_texture    = TextureManager::LoadTexture("assets/tiles/grass.png");

        Map(std::string path, int dimension=1) {
            this->tile_width = dimension;
            this->loaded = LoadMapFile(path);
        }
        ~Map() {}

        bool LoadMapFile(std::string& path);

        void LoadMapRender(float tile_scale=1.0f) {
            uint64_t row, column;
            SDL_Texture** tex = nullptr;
            for(row = 0; row < this->layout_height; ++row) {
                for(column = 0; column < this->layout_width; ++column) {
                    switch(this->layout[row][column]) {
                        case TILE_ROUGH:  tex = &dirt_texture;     break;
                        case TILE_IMPASSABLE:  tex = &mountain_texture; break;
                        case TILE_NAVIGABLE:  tex = &water_texture;    break;
                        default: tex = &grass_texture;
                    }
                    Game::AddTile(
                        *tex, this->layout[row][column], 
                        this->tile_width * tile_scale,
                        column, row,
                        this->layout
                    );
                }
            }
        }
        
        int getTileFromWorldPos(Vector2D world_pos) {
            float x = floorf(world_pos.x / tile_width);
            float y = floorf(world_pos.y / tile_width);
            return this->layout[x][y];
        }

        // generate collision mesh for path finding where true values are IMPASSABLE tiles.
        // out_mesh will be overwritten entirely with the new mesh
        void generateCollisionMesh(const int subdivisions, std::vector<std::vector<bool>>& out_mesh, int& out_width, int& out_height) {
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
                    out_mesh[row].push_back( this->layout[ shifted_row ][ column>>factor ] == TILE_IMPASSABLE );
                }
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Collision Mesh Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
        }

};