#include <vector>
#include <random>
#include <map>
#include "Game.hpp"
#include "SceneTypes.hpp"
#include "Scene.hpp"
#include "Colors.hpp"

int Game::MAX_FPS;
int Game::MAX_FRAME_DELAY;
std::mt19937* Game::RNG;

Manager* Game::manager;
const int Game::UNIT_SIZE = 32;
const int Game::DOUBLE_UNIT_SIZE = Game::UNIT_SIZE<<1;
int Game::SCREEN_HEIGHT;
int Game::SCREEN_WIDTH;

Vector2D Game::camera_position;
Vector2D Game::camera_velocity;
Vector2D Game::camera_focus;
const float Game::DEFAULT_SPEED = 100.0f; // pixels per second
float Game::camera_zoom;

bool Game::isRunning = false;
uint64_t Game::FRAME_COUNT;
float Game::AVERAGE_FPS;
float Game::FRAME_DELTA = 0.0f;
int Game::UNIT_COUNTER = 0;

SDL_Color Game::default_bg_color = COLORS_ROUGH;
TTF_Font *Game::default_font;
SDL_Color Game::default_text_color{ 249, 211, 0, SDL_ALPHA_OPAQUE };

SDL_Renderer *Game::renderer = nullptr;
SDL_Window *Game::window = nullptr;

SDL_Texture *Game::unit_tex = nullptr;
SDL_Texture *Game::building_tex = nullptr;

float Game::world_map_layout_width;
float Game::world_map_layout_height;

int Game::collision_mesh_macro_16_height;
int Game::collision_mesh_macro_16_width;
int Game::collision_mesh_macro_4_height;
int Game::collision_mesh_macro_4_width;
int Game::collision_mesh_1_height;
int Game::collision_mesh_1_width;        
int Game::collision_mesh_4_height;
int Game::collision_mesh_4_width;        
int Game::collision_mesh_16_height;
int Game::collision_mesh_16_width;
int Game::collision_mesh_64_height;
int Game::collision_mesh_64_width;
std::vector<std::vector<uint8_t>> Game::collision_mesh_64;
std::vector<std::vector<uint8_t>> Game::collision_mesh_16;
std::vector<std::vector<uint8_t>> Game::collision_mesh_4;
std::vector<std::vector<uint8_t>> Game::collision_mesh_1;
std::vector<std::vector<uint8_t>> Game::collision_mesh_macro_4;
std::vector<std::vector<uint8_t>> Game::collision_mesh_macro_16;


int Game::SERVER_STATE_SHARE_RATE;
int Game::CLIENT_PING_RATE;
std::map<std::string, std::string> Game::USERS_IP;

Scene* scene;

Game::Game() {

}

Game::~Game() {

}



/**
 * title: window name
 * width and height: window proportions in pixels
 * fullscreen: force fullscreen (true fullscreen)
 * max_fps: maximum frames to be rendered per second
 * server_broadcast_rate: the amount of frames inbetween sending broadcast TCP packages
 * users_ip: map of user_name to its IP string
 * rng_generator: base random function pre-seeded to generate further RNG values
*/
void Game::init(
    const char* title, 
    int width, int height, bool fullscreen,
    int max_fps, 
    int server_broadcast_rate, 
    std::map<std::string, std::string>& users_ip,
    std::mt19937* rng_generator
) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL could not initialize. SDL Error: %s\n", SDL_GetError());
        Game::isRunning = false;
        return;
    }

    const int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if(IMG_Init(img_flags) != img_flags) {
        SDL_Log("SDL_image could not initialize. SDL_image error %s\n", IMG_GetError());
        Game::isRunning = false;
        return;
    }

    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize. SDL_mixer error: %s\n", Mix_GetError());
        Game::isRunning = false;
        return;
    }

    if(TTF_Init() != 0) {
        SDL_Log("SDL_ttf could not initialize. SDL_ttf Error: %s\n", SDL_GetError());
        Game::isRunning = false;
        return;
    }





    std::cout << "Subsystems Initialized\n";
    Game::default_font = TTF_OpenFont("assets/fonts/FSEX302-alt.ttf", 16); // ideal size is 16 for this font but multiples of 8 work alright
    Game::SCREEN_WIDTH = width;
    Game::SCREEN_HEIGHT = height;
    Game::camera_focus = Vector2D(Game::SCREEN_WIDTH>>1, Game::SCREEN_HEIGHT>>1);

    uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_GRABBED;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    Game::window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (Game::window) {
        SDL_Log("Window created\n");
    }

    Game::renderer = SDL_CreateRenderer(Game::window, -1, SDL_RENDERER_ACCELERATED);
    if (Game::renderer) {
        SDL_SetRenderDrawColor(Game::renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
        SDL_Log("Renderer created\n");
    }

    SDL_RendererInfo info;
    if(SDL_GetRendererInfo(Game::renderer, &info) == 0) {
        SDL_Log("Renderer Name: %s\n", info.name);
        std::cout << "Texture Formats Supported: ";
        for(uint32_t i=0; i<info.num_texture_formats; ++i) {
            std::cout << SDL_GetPixelFormatName(info.texture_formats[i]) << ' ';
        }
        std::cout << "\nMax Texture Size: " << info.max_texture_width << "x" << info.max_texture_height << '\n';
        if(info.flags & SDL_RENDERER_ACCELERATED) {
            SDL_Log("Renderer is hardware accelerated.\n");
        } else {
            SDL_Log("Renderer is software (CPU blitting)!\n");
        }

        if(info.flags & SDL_RENDERER_PRESENTVSYNC) {
            SDL_Log("Renderer supports VSYNC.\n");
        }        
    } else {
        SDL_Log("SDL_GetRendererInfo failed: %s\n", SDL_GetError());
    }

    Game::isRunning = true;
    
    Game::camera_position = Vector2D(0,0);
    Game::camera_velocity = Vector2D(0,0);
    Game::camera_zoom = 1.0f;

    // Game::LIMIT_FPS = true;
    Game::MAX_FPS = max_fps;
    Game::MAX_FRAME_DELAY = 1000.0f / max_fps;
    Game::SERVER_STATE_SHARE_RATE = max_fps / server_broadcast_rate;
    Game::CLIENT_PING_RATE = max_fps * 3;
    Game::USERS_IP = users_ip;
    Game::RNG = rng_generator;

    Game::manager = new Manager();

    scene = new Scene();
    scene->setScene(SceneType::MAIN_MENU);
}




void Game::handleEvents() {
    scene->handleEventsPrePoll();
    scene->handleEventsPollEvent();
    scene->handleEventsPostPoll();
}

void Game::update() {
    scene->update();
}



void Game::render() {
    SDL_RenderClear(Game::renderer);
    scene->render();
    SDL_RenderPresent(Game::renderer);
}

void Game::clean() {
    scene->clean();
    delete scene;
    SDL_DestroyTexture(Game::unit_tex);
    SDL_DestroyTexture(Game::building_tex);
    Game::unit_tex = nullptr;
    Game::building_tex = nullptr;
    delete Game::manager;
    Game::manager = nullptr;
    
    TTF_CloseFont(Game::default_font);
    Game::default_font = nullptr;

    SDL_DestroyWindow(Game::window);
    SDL_DestroyRenderer(Game::renderer);
    Game::window = nullptr;
    Game::renderer = nullptr;
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    std::cout << "Game cleaned\n";
}
