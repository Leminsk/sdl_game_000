#pragma once

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "ECS/ECS.hpp"

class Game {
    public:
        static int SCREEN_WIDTH, SCREEN_HEIGHT;
        static float frame_delta;
        static bool isRunning;
        static SDL_Renderer *renderer;
        static SDL_Event event;
        static Entity& camera;
        static TTF_Font* default_font;
        static SDL_Color bg_color, default_text_color;
        static SDL_Texture *unit_tex, *building_tex;
        static const int UNIT_SIZE;

        Game();
        ~Game();

        void init(const char* title, int width, int height, bool fullscreen);

        void handleEvents();
        void update();
        void render();
        void clean();

        bool running() { return this->isRunning; }

        static void AddTile(SDL_Texture* t, int id, float width, int map_x, int map_y, const std::vector<std::vector<int>>& layout);

    private:
        int cnt = 0;
        SDL_Window *window;

};