#include "Game.hpp"
#include "Vector2D.hpp"
#include "Map.hpp"
#include "ECS/ECS.hpp"
#include "ECS/Components.hpp"
#include "ECS/Colliders/Collider.hpp"
#include "ECS/Colliders/Collision.hpp"

Map* map;
Manager* manager = new Manager();

int Game::SCREEN_HEIGHT;
int Game::SCREEN_WIDTH;
bool Game::isRunning = false;
float Game::frame_delta = 0.0f;

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
Entity& Game::camera(manager->addEntity());



auto& player(manager->addEntity());
auto& hexagon_wall(manager->addEntity());
auto& circle_wall(manager->addEntity());

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
        this->SCREEN_WIDTH = width;
        this->SCREEN_HEIGHT = height;

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
        
        Game::camera.addComponent<TransformComponent>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 1.0f);

        map = new Map("assets/test.bmp");

        player.addComponent<TransformComponent>(0.0f, 0.0f, 32.0f, 32.0f, 5.0);
        player.addComponent<SpriteComponent>("assets/green_circle.png");
        player.addComponent<KeyboardController>();
        player.addComponent<Collider>("player", COLLIDER_RECTANGLE);
        player.addComponent<Wireframe>();
        player.addGroup(groupPlayers);


        hexagon_wall.addComponent<TransformComponent>(0.0f, 400.0f, 200.0f, 200.0f, 1.0);
        hexagon_wall.addComponent<SpriteComponent>("assets/green_circle2.png");
        hexagon_wall.addComponent<Collider>("hexagon_wall", COLLIDER_RECTANGLE);
        hexagon_wall.addComponent<Wireframe>();
        hexagon_wall.addGroup(groupColliders);

        circle_wall.addComponent<TransformComponent>(600.0f, 0.0f, 100.0f, 100.0f, 1.0);
        circle_wall.addComponent<SpriteComponent>("assets/magenta_circle.png");
        circle_wall.addComponent<Collider>("circle_wall", COLLIDER_RECTANGLE);
        circle_wall.addComponent<Wireframe>();
        circle_wall.addGroup(groupColliders);

    } else {
        this->isRunning = false;
    }
}

void Game::handleEvents() {
    

    SDL_PollEvent(&Game::event);
    
    const SDL_Keycode key = Game::event.key.keysym.sym;
    Vector2D *camera_v = &Game::camera.getComponent<TransformComponent>().velocity;

    switch(Game::event.type) {
        case SDL_QUIT:
            this->isRunning = false;
            break;

        case SDL_KEYDOWN:
            if(key ==    SDLK_UP) { camera_v->y = -1.0f; }
            if(key ==  SDLK_DOWN) { camera_v->y =  1.0f; }
            if(key ==  SDLK_LEFT) { camera_v->x = -1.0f; }
            if(key == SDLK_RIGHT) { camera_v->x =  1.0f; }
            break;

        case SDL_KEYUP:
            if(key ==    SDLK_UP || key ==  SDLK_DOWN) { camera_v->y = 0.0f; }
            if(key ==  SDLK_LEFT || key == SDLK_RIGHT) { camera_v->x = 0.0f; }
            break;

        default:
            break;
    }
}

void Game::update() {
    Vector2D prev_player_pos = player.getComponent<TransformComponent>().position;
    Vector2D prev_player_vel = player.getComponent<TransformComponent>().velocity;
    manager->refresh();
    manager->update();

    // TODO: iterate all colliders here in a "smart" manner

    float distance_2;
    // for(auto& c : Game::colliders) {
    //     if(c->identifier != "player") {
    //         distance_2 = Distance(player.getComponent<TransformComponent>().position, c->getCenter());
    //         if(distance_2 <= 1000000.0f && Collision::Collide(player.getComponent<Collider>(), *c)) {
    //             std::cout << "collide!\n";
    //             player.getComponent<TransformComponent>().position = prev_player_pos;
    //         }
    //     }
    // }


    Vector2D potential_pos = AddVecs(prev_player_pos, prev_player_vel);

    // if(Collision::HexCircle(
    //     hexagon_wall.getComponent<HexagonCollider>(),
    //     player.getComponent<CircleCollider>()
    // )) {
    //     player.getComponent<TransformComponent>().position = prev_player_pos;
    //     std::cout << "player HIT hexagon_wall\n";
    // }

    // if(Collision::CircleCircle(
    //     player.getComponent<Collider>(),
    //     circle_wall.getComponent<Collider>()
    // )) {
    //     player.getComponent<TransformComponent>().position = prev_player_pos;
    //     std::cout << "player HIT circle_wall\n";
    // }
}

auto& map_assets(manager->getGroup(groupColliders));
auto& players(manager->getGroup(groupPlayers));
auto& enemies(manager->getGroup(groupEnemies));

void Game::render() {
    SDL_RenderClear(this->renderer);
    for(auto& t : map_assets) { t->draw(); }
    for(auto& p : players) { p->draw(); }
    for(auto& e : enemies) { e->draw(); }
    SDL_RenderPresent(this->renderer);
}

void Game::clean() {
    delete manager;
    delete map;

    SDL_DestroyWindow(this->window);
    SDL_DestroyRenderer(this->renderer);
    this->window = NULL;
    this->renderer = NULL;
    IMG_Quit();
    SDL_Quit();
    std::cout << "Game cleaned\n";
}

void Game::AddTile(int id, float width, int map_x, int map_y) {
    auto& tile(manager->addEntity());
    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id);
    tile.addGroup(groupMap);
}