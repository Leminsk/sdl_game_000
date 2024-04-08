#include "Game.hpp"
#include "TextureManager.hpp"
#include "GameObject.hpp"
#include "Map.hpp"

#include "ECS.hpp"
#include "Components.hpp"

GameObject* player;
SDL_Renderer *Game::renderer = nullptr;
Map* map;

Manager manager;
auto& newPlayer(manager.addEntity());

Game::Game() {

}

Game::~Game() {

}

/**
 * title: window name
 * xpos and ypos: window center coordinates
 * width and height: window proportions in pixels
 * fullscreen: force fullscreen (true fullscreen)
*/
void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        std::cout << "Subsystems Initialized\n";

        int flags = 0;
        if (fullscreen) {
            flags = SDL_WINDOW_FULLSCREEN;
        }
        
        this->window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
        if (this->window) {
            std::cout << "Window created\n";
        }

        this->renderer = SDL_CreateRenderer(window, -1, 0);
        if (this->renderer) {
            SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
            std::cout << "Renderer created\n";
        }

        this->isRunning = true;

        player = new GameObject("assets/magenta_circle.png", 0, 0);

        map = new Map();

    } else {
        this->isRunning = false;
    }
}

void Game::handleEvents() {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch(event.type) {
        case SDL_QUIT:
            this->isRunning = false;
            break;

        default:
            break;
    }
}

void Game::update() {
    player->Update();
}

void Game::render() {
    SDL_RenderClear(this->renderer);

    map->DrawMap();
    player->Render();

    SDL_RenderPresent(this->renderer);
}

void Game::clean() {
    SDL_DestroyWindow(this->window);
    SDL_DestroyRenderer(this->renderer);
    SDL_Quit();
    std::cout << "Game cleaned\n";
}