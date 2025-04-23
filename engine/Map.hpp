#pragma once
#include <vector>
#include <string>
#include <cmath>
#include "Vector2D.hpp"
#include "Game.hpp"

class Map {
    public:
        int tile_width;
        std::vector<std::vector<int>> layout;

        Map(std::string path, int dimension=1) {
            this->tile_width = dimension;
            LoadMapFile(path);
        }
        ~Map() {}

        void LoadMapFile(std::string& path);

        void LoadMapRender() {
            uint64_t row, column;
            for(row = 0; row < this->layout_height; ++row) {
                for(column = 0; column < this->layout_width; ++column) {
                    Game::AddTile(
                        this->layout[row][column], 
                        static_cast<float>(this->tile_width),
                        column, row
                    );
                }
            }
        }

        void LoadMapRender(float tile_scale) {
            uint64_t row, column;
            for(row = 0; row < this->layout_height; ++row) {
                for(column = 0; column < this->layout_width; ++column) {
                    Game::AddTile(
                        this->layout[row][column], 
                        this->tile_width * tile_scale,
                        column, row
                    );
                }
            }
        }
        
        int getTileFromWorldPos(Vector2D world_pos) {
            float x = floorf(world_pos.x / tile_width);
            float y = floorf(world_pos.y / tile_width);
            return this->layout[x][y];
        }

    private:
        uint32_t layout_width, layout_height;
};