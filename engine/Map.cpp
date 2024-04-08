#include "Map.hpp"
#include "TextureManager.hpp"

int lvl1[20][25] = {
    {0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

Map::Map() {
    this->water = TextureManager::LoadTexture("assets/tiles/water.png");
    this->grass = TextureManager::LoadTexture("assets/tiles/grass.png");
    this->dirt = TextureManager::LoadTexture("assets/tiles/dirt.png");
    
    LoadMap(lvl1);

    this->src.x = 0;
    this->src.y = 0;
    this->dest.x = 0;
    this->dest.y = 0;

    this->src.h = 32;
    this->src.w = 32;
    this->dest.h = 32;
    this->dest.w = 32;
}

void Map::LoadMap(int arr[20][25]) {
    for (int row = 0; row < 20; ++row) {
        for (int column = 0; column < 25; ++column) {
            this->layout[row][column] = arr[row][column];
        }
    }
}

void Map::DrawMap() {
    int type = 0;

    for (int row = 0; row < 20; ++row) {
        for (int column = 0; column < 25; ++column) {
            type = this->layout[row][column];

            this->dest.x = column * 32;
            this->dest.y = row * 32;

            switch (type) {
                case 0:
                    TextureManager::Draw(this->water, this->src, this->dest);
                    break;
                case 1:
                    TextureManager::Draw(this->grass, this->src, this->dest);
                    break;
                case 2:
                    TextureManager::Draw(this->dirt, this->src, this->dest);
                    break;
                default:
                    break;
            }
        }
    }
}