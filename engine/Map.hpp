#pragma once
#include "Game.hpp"

class Map {
    public:
        Map();
        ~Map();

        void LoadMap(std::string path);

    private:
        std::vector<std::vector<int>> layout;
        int layout_width, layout_height;
};