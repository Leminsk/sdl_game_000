#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>

#include "ECS/Colliders/Collider.hpp"

class Collider;

class Game {
    public:
        Game();
        ~Game();
        void init(const char* title, int width, int height, bool fullscreen);

        void handleEvents(const float& frame_delta);
        void update(const float& frame_delta);
        void render(const float& frame_delta);
        void clean();

        bool running() { return this->isRunning; }

        static void AddTile(int id, float x, float y);
        static SDL_Renderer *renderer;
        static SDL_Event event;
        static std::vector<Collider*> colliders;
        static bool isRunning;
        static SDL_FRect camera;

    private:
        int cnt = 0;
        SDL_Window *window;

};