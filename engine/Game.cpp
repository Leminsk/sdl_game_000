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
SDL_Color Game::bg_color{ 220, 220, 220, SDL_ALPHA_OPAQUE };
TTF_Font *Game::default_font;
SDL_Color Game::default_text_color{ 0, 0, 0, SDL_ALPHA_OPAQUE };

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
Entity& Game::camera(manager->addEntity());

auto& player(manager->addEntity());
auto& hexagon_wall(manager->addEntity());
auto& circle_wall(manager->addEntity());

auto& mountain_tile(manager->addEntity());
auto& corner_tile(manager->addEntity());
auto& center_tile(manager->addEntity());

enum groupLabels : size_t {
    groupMap,
    groupMovables,
    groupInerts,
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
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("SDL could not initialize. SDL Error: %s\n", SDL_GetError());
        this->isRunning = false;
        return;
    }

    if(TTF_Init() != 0) {
        SDL_Log("SDL_ttf could not initialize. SDL_ttf Error: %s\n", SDL_GetError());
        this->isRunning = false;
        return;
    }





    std::cout << "Subsystems Initialized\n";
    Game::default_font = TTF_OpenFont("assets/fixedsys-2-monospaced.ttf", 12);
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
    Game::camera.addComponent<TextComponent>("", true);

    map = new Map("assets/test.bmp");

    player.addComponent<TransformComponent>(0.0f, 0.0f, 64.0f, 64.0f, 5.0);
    player.addComponent<SpriteComponent>("assets/green_circle.png");
    player.addComponent<KeyboardController>();
    player.addComponent<Collider>("player", COLLIDER_CIRCLE);
    player.addComponent<Wireframe>();
    player.addComponent<TextComponent>("", 160.0f, 16.0f);
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
    // circle_wall.addComponent<TextComponent>("circle_wall");
    circle_wall.addGroup(groupStationaries);


    mountain_tile.addComponent<TransformComponent>(150.0f, 150.0f, 100.0f, 100.0f, 1.0);
    mountain_tile.addComponent<SpriteComponent>("assets/tiles/mountain.png");
    mountain_tile.addComponent<Collider>("mountain_tile", COLLIDER_RECTANGLE);
    mountain_tile.addComponent<Wireframe>();
    mountain_tile.addGroup(groupStationaries);

    float side = 50.0f;
    center_tile.addComponent<TransformComponent>(
        static_cast<float>(width-side)/2, static_cast<float>(height-side)/2,
        side, side, 1.0
    );
    center_tile.addComponent<SpriteComponent>("assets/tiles/water.png");
    center_tile.addGroup(groupInerts);

    corner_tile.addComponent<TransformComponent>(
        static_cast<float>(width)-(side/2), static_cast<float>(height)-(side/2),
        side, side, 1.0
    );
    corner_tile.addComponent<SpriteComponent>("assets/tiles/water.png");
    corner_tile.addGroup(groupInerts);
}

auto& stationaries(manager->getGroup(groupStationaries));
auto& movables(manager->getGroup(groupMovables));
auto& inerts(manager->getGroup(groupInerts));

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
        TransformComponent *camera_transform = &Game::camera.getComponent<TransformComponent>();
        Vector2D *camera_v = &camera_transform->velocity;
        float *zoom = &camera_transform->scale;

        if(keystates[SDL_SCANCODE_UP   ]) { camera_v->y =  -2.0f / *zoom; }
        if(keystates[SDL_SCANCODE_DOWN ]) { camera_v->y =   2.0f / *zoom; }
        if(keystates[SDL_SCANCODE_LEFT ]) { camera_v->x =  -2.0f / *zoom; }
        if(keystates[SDL_SCANCODE_RIGHT]) { camera_v->x =   2.0f / *zoom; }

        if(!keystates[SDL_SCANCODE_UP  ] && !keystates[SDL_SCANCODE_DOWN ]) { camera_v->y = 0.0f; }
        if(!keystates[SDL_SCANCODE_LEFT] && !keystates[SDL_SCANCODE_RIGHT]) { camera_v->x = 0.0f; }

        if(keystates[SDL_SCANCODE_KP_PLUS]) { *zoom = std::min(*zoom + 0.02f, 10.0f); }
        if(keystates[SDL_SCANCODE_KP_MINUS]) { *zoom = std::max(*zoom - 0.02f, 0.01f); }
        
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

    Vector2D camera_pos = Game::camera.getComponent<TransformComponent>().position;
    Game::camera.getComponent<TextComponent>().setText(("Camera: "+camera_pos.FormatDecimal(4,0)).c_str());

    Vector2D player_pos = player.getComponent<TransformComponent>().position;
    player.getComponent<TextComponent>().setText(player_pos.FormatDecimal(4,0).c_str());

    manager->update();
}



void Game::render() {
    SDL_RenderClear(this->renderer);
    for(auto& s : stationaries) { s->draw(); }
    for(auto& m : movables) { m->draw(); }
    for(auto& i : inerts) { i->draw(); }
    Game::camera.draw();
    SDL_RenderPresent(this->renderer);
}

void Game::clean() {
    delete manager;
    delete map;
    TTF_CloseFont(Game::default_font);
    Game::default_font = NULL;

    SDL_DestroyWindow(this->window);
    SDL_DestroyRenderer(this->renderer);
    this->window = NULL;
    this->renderer = NULL;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    std::cout << "Game cleaned\n";
}

void Game::AddTile(int id, float width, int map_x, int map_y) {
    auto& tile(manager->addEntity());
    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id);
    tile.addGroup(groupMap);
}