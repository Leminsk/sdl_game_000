#pragma once
#include <vector>
#include <string>
#include <cmath>
#include "Vector2D.hpp"
#include "Game.hpp"
#include "TextureManager.hpp"

class Map {
    public:
        int tile_width;
        std::vector<std::vector<int>> layout;

        SDL_Texture* dirt_texture     = TextureManager::LoadTexture("assets/tiles/dirt.png");
        SDL_Texture* mountain_texture = TextureManager::LoadTexture("assets/tiles/mountain.png");
        SDL_Texture* water_texture    = TextureManager::LoadTexture("assets/tiles/water.png");
        SDL_Texture* grass_texture    = TextureManager::LoadTexture("assets/tiles/grass.png");

        Map(std::string path, int dimension=1) {
            this->tile_width = dimension;
            LoadMapFile(path);
        }
        ~Map() {}

        void LoadMapFile(std::string& path);

        void LoadMapRender(float tile_scale=1.0f) {
            uint64_t row, column;
            SDL_Texture* tex = NULL;
            for(row = 0; row < this->layout_height; ++row) {
                for(column = 0; column < this->layout_width; ++column) {
                    switch(this->layout[row][column]) {
                        case 1:  tex = dirt_texture;     break;
                        case 2:  tex = mountain_texture; break;
                        case 3:  tex = water_texture;    break;
                        default: tex = grass_texture;    break;
                    }
                    Game::AddTile(
                        tex, this->layout[row][column], 
                        this->tile_width * tile_scale,
                        column, row
                    );
                }
            }
            free(tex);
        }
        
        int getTileFromWorldPos(Vector2D world_pos) {
            float x = floorf(world_pos.x / tile_width);
            float y = floorf(world_pos.y / tile_width);
            return this->layout[x][y];
        }

    private:
        uint32_t layout_width, layout_height;
};