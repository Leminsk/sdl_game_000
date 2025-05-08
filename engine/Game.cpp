#include <vector>
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

auto& mountain_tile(manager->addEntity());

enum groupLabels : size_t {
    groupMap,
    groupMovables,
    groupEnemies,
    groupStationaries
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

        player.addComponent<TransformComponent>(0.0f, 0.0f, 64.0f, 64.0f, 5.0);
        player.addComponent<SpriteComponent>("assets/green_circle.png");
        player.addComponent<KeyboardController>();
        player.addComponent<Collider>("player", COLLIDER_CIRCLE);
        player.addComponent<Wireframe>();
        player.addGroup(groupMovables);


        hexagon_wall.addComponent<TransformComponent>(0.0f, 400.0f, 200.0f, 200.0f, 1.0);
        hexagon_wall.addComponent<SpriteComponent>("assets/green_circle2.png");
        hexagon_wall.addComponent<Collider>("hexagon_wall", COLLIDER_HEXAGON);
        hexagon_wall.addComponent<Wireframe>();
        hexagon_wall.addGroup(groupStationaries);

        circle_wall.addComponent<TransformComponent>(600.0f, 0.0f, 200.0f, 200.0f, 1.0);
        circle_wall.addComponent<SpriteComponent>("assets/magenta_circle.png");
        circle_wall.addComponent<Collider>("circle_wall", COLLIDER_CIRCLE);
        circle_wall.addComponent<Wireframe>();
        circle_wall.addGroup(groupStationaries);


        mountain_tile.addComponent<TransformComponent>(150.0f, 150.0f, 100.0f, 100.0f, 1.0);
        mountain_tile.addComponent<SpriteComponent>("assets/tiles/mountain.png");
        mountain_tile.addComponent<Collider>("mountain_tile", COLLIDER_RECTANGLE);
        mountain_tile.addComponent<Wireframe>();
        mountain_tile.addGroup(groupStationaries);

    } else {
        this->isRunning = false;
    }
}

auto& stationaries(manager->getGroup(groupStationaries));
auto& movables(manager->getGroup(groupMovables));
auto& enemies(manager->getGroup(groupEnemies));

void Game::handleEvents() {
    
    while( SDL_PollEvent(&Game::event) ) {
        if(Game::event.type == SDL_QUIT) {
            this->isRunning = false;
        }
    }


    const uint8_t *keystates = SDL_GetKeyboardState(NULL);

    if(keystates[SDL_SCANCODE_ESCAPE]) { 
        this->isRunning = false; 
    } else {
        Vector2D *camera_v = &Game::camera.getComponent<TransformComponent>().velocity;

        if(keystates[SDL_SCANCODE_UP   ]) { camera_v->y =  -1.0f; }
        if(keystates[SDL_SCANCODE_DOWN ]) { camera_v->y =   1.0f; }
        if(keystates[SDL_SCANCODE_LEFT ]) { camera_v->x =  -1.0f; }
        if(keystates[SDL_SCANCODE_RIGHT]) { camera_v->x =   1.0f; }

        if(!keystates[SDL_SCANCODE_UP  ] && !keystates[SDL_SCANCODE_DOWN ]) { camera_v->y = 0.0f; }
        if(!keystates[SDL_SCANCODE_LEFT] && !keystates[SDL_SCANCODE_RIGHT]) { camera_v->x = 0.0f; }
    }    
}

void Game::update() {
    Collider *player_collider = &player.getComponent<Collider>();
    manager->refresh();
    
    // TODO: iterate all stationaries here in a "smart" manner
    float distance_2;
    Collider *s_col;
    for(auto& s : stationaries) {
        s_col = &s->getComponent<Collider>();
        distance_2 = Distance(player_collider->getCenter(), s_col->getCenter());
        if(distance_2 <= 100000.0f) {
            player_collider->transform->position = Collision::Collide(*player_collider, *s_col);
        }
    }

    manager->update();
}



void Game::render() {
    SDL_RenderClear(this->renderer);
    for(auto& s : stationaries) { s->draw(); }
    for(auto& m : movables) { m->draw(); }
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