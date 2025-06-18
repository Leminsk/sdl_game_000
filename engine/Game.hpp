#pragma once

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "../networking/Client.hpp"
#include "../networking/Server.hpp"
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
        static int UNIT_COUNTER;

        static int collision_mesh_1_height;
        static int collision_mesh_1_width;        
        static int collision_mesh_4_height;
        static int collision_mesh_4_width;
        static int collision_mesh_16_height;
        static int collision_mesh_16_width;
        static int collision_mesh_64_height;
        static int collision_mesh_64_width;
        // All meshes are indexed Y first, then X
        static std::vector<std::vector<bool>> collision_mesh_64;
        static std::vector<std::vector<bool>> collision_mesh_16;
        static std::vector<std::vector<bool>> collision_mesh_4;
        static std::vector<std::vector<bool>> collision_mesh_1;


        static bool server_up;
        static bool client_up;
        static Client *client;
        static Server *server;

        Game();
        ~Game();

        void init(const char* title, int width, int height, bool fullscreen);

        void handleEvents();
        void update();
        void render();
        void clean();

        bool running() { return this->isRunning; }

        static void AddTile(SDL_Texture* t, int id, float width, int map_x, int map_y, const std::vector<std::vector<int>>& layout);
        static Entity& createDrone(float pos_x, float pos_y);

    private:
        int cnt = 0;
        SDL_Window *window;

};