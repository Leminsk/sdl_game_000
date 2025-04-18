#include "Game.hpp"
#include "ECS/ECS.hpp"
#include "ECS/Components.hpp"
#include "ECS/Colliders/Collider.hpp"
#include "ECS/Colliders/Collision.hpp"
#include "Map.hpp"
#include "Vector2D.hpp"


Map* map;
Manager manager;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
SDL_FRect Game::camera = { 0.0f, 0.0f, 800.0f, 600.0f };

std::vector<Collider*> Game::colliders;

bool Game::isRunning = false;

auto& player(manager.addEntity());
auto& hexagon_wall(manager.addEntity());
auto& circle_wall(manager.addEntity());

enum groupLabels : size_t {
    groupMap,
    groupPlayers,
    groupEnemies,
    groupColliders
};

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
            SDL_SetRenderDrawColor(this->renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
            std::cout << "Renderer created\n";
        }

        this->isRunning = true;

        map = new Map();
        map->LoadMap("assets/test.bmp");

        player.addComponent<TransformComponent>(10.0f, 10.0f, 32.0f, 32.0f, 2.0);
        player.addComponent<SpriteComponent>("assets/green_circle.png");
        player.addComponent<KeyboardController>();
        player.addComponent<Collider>(COLLIDER_CIRCLE, Game::colliders);
        player.addComponent<Wireframe>();
        player.addGroup(groupPlayers);


        hexagon_wall.addComponent<TransformComponent>(300.0f, 300.0f, 200.0f, 200.0f, 1.0);
        hexagon_wall.addComponent<SpriteComponent>("assets/green_circle2.png");
        hexagon_wall.addComponent<Collider>(COLLIDER_HEXAGON, Game::colliders);
        hexagon_wall.addComponent<Wireframe>();
        hexagon_wall.addGroup(groupMap);

        circle_wall.addComponent<TransformComponent>(500.0f, 30.0f, 100.0f, 100.0f, 1.0);
        circle_wall.addComponent<SpriteComponent>("assets/magenta_circle.png");
        circle_wall.addComponent<Collider>(COLLIDER_CIRCLE, Game::colliders);
        circle_wall.addComponent<Wireframe>();
        circle_wall.addGroup(groupMap);

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

    if(Collision::HexCircle(
        hexagon_wall.getComponent<HexagonCollider>(),
        player.getComponent<CircleCollider>()
    )) {
        player.getComponent<TransformComponent>().position = prev_player_pos;
        std::cout << "player HIT hexagon_wall\n";
    }

    if(Collision::CircleCircle(
        player.getComponent<Collider>(),
        circle_wall.getComponent<Collider>()
    )) {
        player.getComponent<TransformComponent>().position = prev_player_pos;
        std::cout << "player HIT circle_wall\n";
    }
}

auto& tiles(manager.getGroup(groupMap));
auto& players(manager.getGroup(groupPlayers));
auto& enemies(manager.getGroup(groupEnemies));

void Game::render() {
    SDL_RenderClear(this->renderer);
    for(auto& t : tiles) { t->draw(); }
    for(auto& p : players) { p->draw(); }
    for(auto& e : enemies) { e->draw(); }
    SDL_RenderPresent(this->renderer);
}

void Game::clean() {
    SDL_DestroyWindow(this->window);
    SDL_DestroyRenderer(this->renderer);
    this->window = NULL;
    this->renderer = NULL;
    IMG_Quit();
    SDL_Quit();
    std::cout << "Game cleaned\n";
}

void Game::AddTile(int id, float x, float y) {
    auto& tile(manager.addEntity());
    tile.addComponent<TileComponent>(x, y, 32.0f, 32.0f, id);
    tile.addGroup(groupMap);
}