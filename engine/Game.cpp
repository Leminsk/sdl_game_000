#include <vector>
#include <random>
#include "Game.hpp"
#include "SceneTypes.hpp"
#include "Scene.hpp"

std::mt19937* Game::RNG;

Manager* Game::manager;
const int Game::UNIT_SIZE = 32;
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

SDL_Color Game::bg_color{ 50, 5, 10, SDL_ALPHA_OPAQUE };
TTF_Font *Game::default_font;
SDL_Color Game::default_text_color{ 249, 211, 0, SDL_ALPHA_OPAQUE };

SDL_Renderer *Game::renderer = nullptr;

SDL_Texture *Game::unit_tex, *Game::building_tex;

float Game::world_map_layout_width;
float Game::world_map_layout_height;

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


int Game::SERVER_STATE_SHARE_RATE;
int Game::CLIENT_PING_RATE;

Scene* scene;

Game::Game() {

}

Game::~Game() {

}



/**
 * title: window name
 * width and height: window proportions in pixels
 * fullscreen: force fullscreen (true fullscreen)
*/
void Game::init(const char* title, int width, int height, bool fullscreen) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL could not initialize. SDL Error: %s\n", SDL_GetError());
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

    uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    
    Game::window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (Game::window) {
        std::cout << "Window created\n";
    }

    Game::renderer = SDL_CreateRenderer(window, -1, 0);
    if (Game::renderer) {
        SDL_SetRenderDrawColor(Game::renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
        std::cout << "Renderer created\n";
    }

    Game::isRunning = true;
    
    Game::camera_position = Vector2D(0,0);
    Game::camera_velocity = Vector2D(0,0);
    Game::camera_zoom = 1.0f;

    // white helps with color modulation
    Game::unit_tex     = TextureManager::LoadTexture("assets/white_circle.png");
    Game::building_tex = TextureManager::LoadTexture("assets/green_hexagon.png");

    Game::manager = new Manager();

    scene = new Scene();
    scene->setScene(SceneType::MATCH_GAME);
}

void loadTransition() {
    // scene->setLoadingScreen();
    // Game::manager->clearEntities();
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
    scene->~Scene();
    Game::manager = nullptr;
    
    TTF_CloseFont(Game::default_font);
    Game::default_font = nullptr;

    SDL_DestroyWindow(Game::window);
    SDL_DestroyRenderer(Game::renderer);
    Game::window = nullptr;
    Game::renderer = nullptr;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    std::cout << "Game cleaned\n";
}
