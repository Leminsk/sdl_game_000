#pragma once

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "Vector2D.hpp"
#include "ECS/MainColors.hpp"
#include "ECS/ECS.hpp"

class Game {
    public:
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
        static std::vector<std::vector<bool>> collision_mesh_64;
        static std::vector<std::vector<bool>> collision_mesh_16;
        static std::vector<std::vector<bool>> collision_mesh_4;
        static std::vector<std::vector<bool>> collision_mesh_1;


        static Manager* manager;

        static MainColors PLAYER_COLOR;
        
        static bool is_client;
        static bool is_server;
        static int SERVER_STATE_SHARE_RATE;
        static int CLIENT_PING_RATE;
        static bool update_server;
        static const int PACKET_SIZE; // the ideal max size in bytes. 10 bytes or so over it is fine
        static int64_t PING_MS; // this client's ping on the server
        static uint32_t PLAYER_CLIENT_ID; // this client's ID on the server

        // drones which have been selected and have had their moveToPoint invoked on this Client. Their paths should then be sent to the server
        static std::vector<Entity*> moved_drones;

        Game();
        ~Game();

        void init(const char* title, int width, int height, bool fullscreen);

        void handleEvents();
        void update();
        void render();
        void clean();

        bool running() { return this->isRunning; }

        static void AddTile(SDL_Texture* t, int id, float width, int map_x, int map_y, const std::vector<std::vector<int>>& layout);
        static Entity& createDrone(float pos_x, float pos_y, MainColors c);
        static Entity& createSimpleUIText(std::string id, int pos_x=0, int pos_y=0, int width=100, int height=10, std::string text="UI_TEXT");

    private:
        int cnt = 0;
        SDL_Window *window;

};