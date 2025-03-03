#include "Game.hpp"
#include "TextureManager.hpp"
#include "Map.hpp"
#include "ECS/Components.hpp"
#include "Vector2D.hpp"
#include "Collision.hpp"

Map* map;
Manager manager;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;

std::vector<ColliderComponent*> Game::colliders;

auto& player(manager.addEntity());
auto& wall(manager.addEntity());

auto& tile0(manager.addEntity());
auto& tile1(manager.addEntity());
auto& tile2(manager.addEntity());

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
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
        std::cout << "Subsystems Initialized\n";

        int flags = 0;
        if (fullscreen) {
            flags = SDL_WINDOW_FULLSCREEN;
        }
        
        this->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
        if (this->window) {
            std::cout << "Window created\n";
        }

        this->renderer = SDL_CreateRenderer(window, -1, 0);
        if (this->renderer) {
            SDL_SetRenderDrawColor(this->renderer, 255, 255, 255, 255);
            std::cout << "Renderer created\n";
        }

        this->isRunning = true;

        map = new Map();

        tile0.addComponent<TileComponent>(200, 200, 32, 32, 0);
        tile1.addComponent<TileComponent>(250, 250, 32, 32, 1);
        tile1.addComponent<ColliderComponent>("dirt");
        tile2.addComponent<TileComponent>(150, 150, 32, 32, 2);
        tile2.addComponent<ColliderComponent>("grass");

        

        player.addComponent<TransformComponent>(2);
        player.addComponent<SpriteComponent>("assets/green_circle.png");
        player.addComponent<KeyboardController>();
        player.addComponent<ColliderComponent>("player");


        wall.addComponent<TransformComponent>(300.0f, 300.0f, 200, 200, 1);
        wall.addComponent<SpriteComponent>("assets/magenta_circle.png");
        wall.addComponent<ColliderComponent>("wall");

    } else {
        this->isRunning = false;
    }
}

void Game::handleEvents() {
    

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
    Vector2D prev_player_pos = player.getComponent<TransformComponent>().position;
    manager.refresh();
    manager.update();

    // TODO: iterate all colliders here in a "smart" manner

    if(Collision::AABB(
        player.getComponent<ColliderComponent>().collider,
        wall.getComponent<ColliderComponent>().collider
    )) {
        player.getComponent<TransformComponent>().position = prev_player_pos;
        std::cout << "WALL HIT!\n";
    }
}

void Game::render() {
    SDL_RenderClear(this->renderer);

    // map->DrawMap();
    manager.draw();

    SDL_RenderPresent(this->renderer);
}

void Game::clean() {
    SDL_DestroyWindow(this->window);
    SDL_DestroyRenderer(this->renderer);
    SDL_Quit();
    std::cout << "Game cleaned\n";
}