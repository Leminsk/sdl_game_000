#pragma once
#include "Game.hpp"

class Map {
    public:
        Map();
        ~Map();

        void LoadMap(std::string path);
        std::vector<std::vector<int>> layout;

    private:
        int layout_width, layout_height;
};