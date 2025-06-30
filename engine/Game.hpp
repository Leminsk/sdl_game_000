#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "Vector2D.hpp"
#include "ECS/ECS.hpp"

class Game {
    public:
        static std::mt19937 *RNG;
        static int SCREEN_WIDTH, SCREEN_HEIGHT;
        // camera
        static Vector2D camera_position;
        static Vector2D camera_velocity;
        static float camera_zoom;
        static Vector2D camera_focus;
        static const float DEFAULT_SPEED;

        static uint64_t FRAME_COUNT;
        static float AVERAGE_FPS;
        static float FRAME_DELTA;
        static bool isRunning;
        static SDL_Renderer *renderer;
        static SDL_Event event;

        static TTF_Font* default_font;
        static SDL_Color bg_color, default_text_color;
        static SDL_Texture *unit_tex, *building_tex;
        static const int UNIT_SIZE;
        static int UNIT_COUNTER;

        static float world_map_layout_width;
        static float world_map_layout_height;

        static int collision_mesh_1_height;
        static int collision_mesh_1_width;        
        static int collision_mesh_4_height;
        static int collision_mesh_4_width;
        static int collision_mesh_16_height;
        static int collision_mesh_16_width;
        static int collision_mesh_64_height;
        static int collision_mesh_64_width;
        // All meshes are indexed Y first, then X
        static std::vector<std::vector<uint8_t>> collision_mesh_64;
        static std::vector<std::vector<uint8_t>> collision_mesh_16;
        static std::vector<std::vector<uint8_t>> collision_mesh_4;
        static std::vector<std::vector<uint8_t>> collision_mesh_1;


        static Manager* manager;
        
        
        static bool is_client;
        static bool is_server;
        static int SERVER_STATE_SHARE_RATE;
        static int CLIENT_PING_RATE;

        Game();
        ~Game();

        void init(const char* title, int width, int height, bool fullscreen);

        void handleEvents();
        void update();
        void render();
        void clean();

        bool running() { return this->isRunning; }

    private:
        int cnt = 0;
        SDL_Window *window;

};