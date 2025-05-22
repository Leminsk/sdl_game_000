#include <vector>
#include "Game.hpp"
#include "Vector2D.hpp"
#include "Map.hpp"
#include "TextureManager.hpp"
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
    groupTiles,
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
    
    Game::camera.addComponent<TransformComponent>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 1.0f);
    Game::camera.addComponent<TextComponent>("", true);

    SDL_Texture* player_texture   = TextureManager::LoadTexture("assets/green_circle.png");
    SDL_Texture* hexagon_texture  = TextureManager::LoadTexture("assets/green_circle2.png");
    SDL_Texture* circle_texture   = TextureManager::LoadTexture("assets/magenta_circle.png");

    // tiles textures
    SDL_Texture* dirt_texture     = TextureManager::LoadTexture("assets/tiles/dirt.png");
    SDL_Texture* mountain_texture = TextureManager::LoadTexture("assets/tiles/mountain.png");
    SDL_Texture* water_texture    = TextureManager::LoadTexture("assets/tiles/water.png");
    SDL_Texture* grass_texture    = TextureManager::LoadTexture("assets/tiles/grass.png");

    map = new Map("assets/test.bmp", 64);
    map->LoadMapRender();

    player.addComponent<TransformComponent>(0.0f, 0.0f, 32.0f, 32.0f, 1.0);
    player.addComponent<SpriteComponent>(player_texture);
    player.addComponent<KeyboardController>();
    player.addComponent<Collider>("player", COLLIDER_CIRCLE);
    player.addComponent<Wireframe>();
    player.addComponent<TextComponent>("", 160.0f, 16.0f);
    player.addGroup(groupMovables);


    // hexagon_wall.addComponent<TransformComponent>(0.0f, 400.0f, 200.0f, 200.0f, 1.0);
    // hexagon_wall.addComponent<SpriteComponent>(hexagon_texture);
    // hexagon_wall.addComponent<Collider>("hexagon_wall", COLLIDER_HEXAGON);
    // hexagon_wall.addComponent<Wireframe>();
    // hexagon_wall.addGroup(groupStationaries);

    // circle_wall.addComponent<TransformComponent>(600.0f, 0.0f, 200.0f, 200.0f, 1.0);
    // circle_wall.addComponent<SpriteComponent>(circle_texture);
    // circle_wall.addComponent<Collider>("circle_wall", COLLIDER_CIRCLE);
    // circle_wall.addComponent<Wireframe>();
    // // circle_wall.addComponent<TextComponent>("circle_wall");
    // circle_wall.addGroup(groupStationaries);


    // mountain_tile.addComponent<TransformComponent>(150.0f, 150.0f, 100.0f, 100.0f, 1.0);
    // mountain_tile.addComponent<SpriteComponent>(mountain_texture);
    // mountain_tile.addComponent<Collider>("mountain_tile", COLLIDER_RECTANGLE);
    // mountain_tile.addComponent<Wireframe>();
    // mountain_tile.addGroup(groupStationaries);

    // float side = 50.0f;
    // center_tile.addComponent<TransformComponent>(
    //     static_cast<float>(width-side)/2, static_cast<float>(height-side)/2,
    //     side, side, 1.0
    // );
    // center_tile.addComponent<SpriteComponent>(water_texture);
    // center_tile.addGroup(groupInerts);

    // corner_tile.addComponent<TransformComponent>(
    //     static_cast<float>(width)-(side/2), static_cast<float>(height)-(side/2),
    //     side, side, 1.0
    // );
    // corner_tile.addComponent<SpriteComponent>(water_texture);
    // corner_tile.addGroup(groupInerts);
}

auto& stationaries(manager->getGroup(groupStationaries));
auto& movables(manager->getGroup(groupMovables));
auto& inerts(manager->getGroup(groupInerts));
auto& tiles(manager->getGroup(groupTiles));

void Game::handleEvents() {
    
    while( SDL_PollEvent(&Game::event) ) {
        if(Game::event.type == SDL_QUIT) {
            Game::isRunning = false;
            return;
        }

        if(Game::event.type == SDL_WINDOWEVENT) {
            switch(Game::event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    Game::SCREEN_WIDTH = Game::event.window.data1;
                    Game::SCREEN_HEIGHT = Game::event.window.data2;
                    break;
                case SDL_WINDOWEVENT_ENTER: std::cout << "Mouse IN\n"; break;
                case SDL_WINDOWEVENT_LEAVE: std::cout << "Mouse OUT\n"; break;
                case SDL_WINDOWEVENT_FOCUS_GAINED: std::cout << "Keyboard IN\n"; break;
                case SDL_WINDOWEVENT_FOCUS_LOST: std::cout << "Keyboard OUT\n"; break;
            }
        }
        
    }


    const uint8_t *keystates = SDL_GetKeyboardState(NULL);

    if(keystates[SDL_SCANCODE_ESCAPE]) { 
        Game::isRunning = false; 
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
        if(keystates[SDL_SCANCODE_KP_MINUS]) { *zoom = std::max(*zoom - 0.02f, 0.05f); }
        
        if(keystates[SDL_SCANCODE_SPACE]) {
            std::cout << "map{x , y}: " << map->layout.size() << ',' << map->layout[0].size() << "tile_width: " << map->tile_width << '\n';
        }
    }
}

void Game::update() {
    Collider *player_collider = &player.getComponent<Collider>();
    manager->refresh();
    manager->update();

    // TODO: iterate all stationaries here in a "smart" manner
    float distance_2;
    Collider *current_col;

    std::vector<std::pair<int, float>> entity_distances = {};
    Entity *t, *s;
    for(int i=0; i<tiles.size(); ++i) {
        t = tiles[i];
        if(t->hasComponent<Collider>()) {
            current_col = &t->getComponent<Collider>();
            distance_2 = Distance(player_collider->getCenter(), current_col->getCenter());
            if(distance_2 <= 100000.0f) {
                // player_collider->transform->position = Collision::Collide(*player_collider, *current_col);
                std::cout << "push_back broken\n";
                entity_distances.push_back({i, distance_2});
            }
        }        
    }
    std::sort(entity_distances.begin(), entity_distances.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
        return a.second < b.second;
    });
    for(auto& p : entity_distances) {
        player_collider->transform->position = Collision::Collide(*player_collider, tiles[p.first]->getComponent<Collider>());
    }

    // entity_distances = {};
    // for(int i=0; i<stationaries.size(); ++i) {
    //     current_col = &stationaries[i]->getComponent<Collider>();
    //     distance_2 = Distance(player_collider->getCenter(), current_col->getCenter());
    //     if(distance_2 <= 100000.0f) {
    //         // player_collider->transform->position = Collision::Collide(*player_collider, *current_col);
    //         entity_distances.push_back(std::make_pair(i, distance_2));
    //     }
    // }
    // std::sort(entity_distances.begin(), entity_distances.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
    //     return a.second < b.second;
    // });

    Game::camera.getComponent<TextComponent>().setText((
        "Camera center: " + Game::camera.getComponent<TransformComponent>().getCenter().FormatDecimal(4,0)
    ).c_str());

    player.getComponent<TextComponent>().setText(
        player.getComponent<TransformComponent>().position.FormatDecimal(4,0)
    .c_str());
}



void Game::render() {
    SDL_RenderClear(Game::renderer);
    for(auto& t : tiles) { t->draw(); }
    for(auto& s : stationaries) { s->draw(); }
    for(auto& m : movables) { m->draw(); }
    for(auto& i : inerts) { i->draw(); }
    Game::camera.draw();

    // cross-hair
    float line_length = 10;
    Vector2D screen_center = Vector2D(Game::SCREEN_WIDTH>>1, Game::SCREEN_HEIGHT>>1);
    Vector2D line_left[2]   = { Vector2D(screen_center.x-1, screen_center.y), Vector2D(screen_center.x-1-line_length, screen_center.y) };
    Vector2D line_right[2]  = { Vector2D(screen_center.x+1, screen_center.y), Vector2D(screen_center.x+1+line_length, screen_center.y) };
    Vector2D line_top[2]    = { Vector2D(screen_center.x, screen_center.y-1), Vector2D(screen_center.x, screen_center.y-1-line_length) };
    Vector2D line_bottom[2] = { Vector2D(screen_center.x, screen_center.y+1), Vector2D(screen_center.x, screen_center.y+1+line_length) };
    SDL_Color cross_hair_color = {0xBA, 0x00, 0x03, SDL_ALPHA_OPAQUE};
    TextureManager::DrawLine(  line_left[0],   line_left[1], cross_hair_color);
    TextureManager::DrawLine( line_right[0],  line_right[1], cross_hair_color);
    TextureManager::DrawLine(   line_top[0],    line_top[1], cross_hair_color);
    TextureManager::DrawLine(line_bottom[0], line_bottom[1], cross_hair_color);

    SDL_RenderPresent(Game::renderer);
}

void Game::clean() {
    delete manager;
    delete map;
    TTF_CloseFont(Game::default_font);
    Game::default_font = NULL;

    SDL_DestroyWindow(Game::window);
    SDL_DestroyRenderer(Game::renderer);
    Game::window = NULL;
    Game::renderer = NULL;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    std::cout << "Game cleaned\n";
}

void Game::AddTile(SDL_Texture* t, int id, float width, int map_x, int map_y) {
    auto& tile(manager->addEntity());
    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, t);
    tile.addGroup(groupTiles);
}