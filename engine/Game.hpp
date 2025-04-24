#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>

class Game {
    public:
        static float frame_delta;
        static bool isRunning;
        static SDL_FRect camera;
        static float camera_zoom;
        static SDL_Renderer *renderer;
        static SDL_Event event;        

        Game();
        ~Game();

        void init(const char* title, int width, int height, bool fullscreen);

        void handleEvents();
        void update();
        void render();
        void clean();

        bool running() { return this->isRunning; }

        static void AddTile(int id, float width, int map_x, int map_y);

    private:
        int cnt = 0;
        SDL_Window *window;

};