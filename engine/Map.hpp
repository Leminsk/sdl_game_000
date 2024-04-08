#pragma once
#include "Game.hpp"

class Map {
    public:
        Map();
        ~Map();

        void LoadMap(int arr[20][25]);
        void DrawMap();

    private:
        SDL_Rect src, dest;

        SDL_Texture *water;
        SDL_Texture *grass;
        SDL_Texture *dirt;

        int layout[20][25];

};