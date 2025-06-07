#include <vector>
#include "Game.hpp"
#include "Vector2D.hpp"
#include "utils.hpp"
#include "Map.hpp"
#include "TextureManager.hpp"
#include "ECS/ECS.hpp"
#include "ECS/Components.hpp"
#include "ECS/Colliders/Collider.hpp"
#include "ECS/Colliders/Collision.hpp"
#include "theta_star.hpp"
#include "Camera.hpp"

Map* map;
Manager* manager = new Manager();
const int Game::UNIT_SIZE = 32;
int Game::SCREEN_HEIGHT;
int Game::SCREEN_WIDTH;
bool Game::isRunning = false;
float Game::frame_delta = 0.0f;
int Game::UNIT_COUNTER = 0;

SDL_Color Game::bg_color{ 220, 220, 220, SDL_ALPHA_OPAQUE };
TTF_Font *Game::default_font;
SDL_Color Game::default_text_color{ 0, 0, 0, SDL_ALPHA_OPAQUE };

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
Entity& Game::camera(manager->addEntity());

SDL_Texture *Game::unit_tex, *Game::building_tex;

int Game::collision_mesh_1_height;
int Game::collision_mesh_1_width;        
int Game::collision_mesh_4_height;
int Game::collision_mesh_4_width;        
int Game::collision_mesh_16_height;
int Game::collision_mesh_16_width;
int Game::collision_mesh_64_height;
int Game::collision_mesh_64_width;
std::vector<std::vector<bool>> Game::collision_mesh_64;
std::vector<std::vector<bool>> Game::collision_mesh_16;
std::vector<std::vector<bool>> Game::collision_mesh_4;
std::vector<std::vector<bool>> Game::collision_mesh_1;

std::unordered_map<int, Vector2D> previous_movables_positions;

enum groupLabels : size_t {
    groupTiles,
    groupMovables,
    groupInerts,
    groupStationaries,
    groupBuildings
};

auto& stationaries(manager->getGroup(groupStationaries));
auto& buildings(manager->getGroup(groupBuildings));
auto& movables(manager->getGroup(groupMovables));
auto& inerts(manager->getGroup(groupInerts));
auto& tiles(manager->getGroup(groupTiles));

std::vector<Vector2D> path;

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

    Game::unit_tex     = TextureManager::LoadTexture("assets/green_circle.png");
    Game::building_tex = TextureManager::LoadTexture("assets/green_hexagon.png");

    map = new Map("assets/test3.bmp", Game::UNIT_SIZE<<1);
    map->LoadMapRender();
    if(map->loaded) {
        std::cout << "Map loaded.\n";
    }

    createDrone(  0,   0);
    createDrone(100,   0);
    createDrone(  0, 100);
    createDrone(100, 100);

    map->generateCollisionMesh( 1, Game::collision_mesh_1,  Game::collision_mesh_1_width,  Game::collision_mesh_1_height);
    map->generateCollisionMesh( 4, Game::collision_mesh_4,  Game::collision_mesh_4_width,  Game::collision_mesh_4_height);
    map->generateCollisionMesh(16, Game::collision_mesh_16, Game::collision_mesh_16_width, Game::collision_mesh_16_height);
    map->generateCollisionMesh(64, Game::collision_mesh_64, Game::collision_mesh_64_width, Game::collision_mesh_64_height);
}

void handleMouse(SDL_MouseButtonEvent& b) {
    Vector2D world_pos = convertScreenToWorld(Vector2D(b.x, b.y));
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            std::cout << "MOUSE BUTTON LEFT: " << world_pos << '\n'; 
            for(auto& m : movables) {
                float r = m->getComponent<CircleCollider>().radius;
                if(Distance(m->getComponent<DroneComponent>().getPosition(), world_pos) <= r*r) {
                    std::cout << m->getComponent<Collider>().identifier << " selected\n";
                    m->getComponent<DroneComponent>().selected = true;
                } else {
                    m->getComponent<DroneComponent>().selected = false;
                }
            }            
            break;
        }
            
        case SDL_BUTTON_MIDDLE: std::cout << "MOUSE BUTTON MIDDLE\n"; break;

        case SDL_BUTTON_RIGHT: {
            std::cout << "MOUSE BUTTON RIGHT: " << world_pos << '\n';
            for(auto& m : movables) {
                if(m->getComponent<DroneComponent>().selected) {
                    m->getComponent<DroneComponent>().moveToPoint(world_pos);
                    path = m->getComponent<DroneComponent>().path;
                }
            }
            break;
        }
    }
}

void Game::handleEvents() {
    
    while( SDL_PollEvent(&Game::event) ) {
        if(Game::event.type == SDL_QUIT) {
            Game::isRunning = false;
            return;
        }

        if(Game::event.type == SDL_MOUSEBUTTONDOWN) {
            handleMouse(Game::event.button);
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
    for(int i=0; i<movables.size(); ++i) {
        previous_movables_positions[i] = movables[i]->getComponent<TransformComponent>().position;
    }

    manager->refresh();
    manager->preUpdate();
    manager->update();

    for(int i=0; i<movables.size(); ++i) {
        movables[i]->getComponent<DroneComponent>().handleCollisions(previous_movables_positions[i], tiles, buildings);
    }    

    Game::camera.getComponent<TextComponent>().setText((
        "Camera center: " + Game::camera.getComponent<TransformComponent>().getCenter().FormatDecimal(4,0)
    ).c_str());
}



void Game::render() {
    SDL_RenderClear(Game::renderer);
    for(auto& t : tiles) { t->draw(); }
    for(auto& s : stationaries) { s->draw(); }
    for(auto& b : buildings) { b->draw(); }
    for(auto& m : movables) { m->draw(); }
    for(auto& i : inerts) { i->draw(); }
    Game::camera.draw();    

    // draw the path trajectory for debugging
    if(path.size() > 0) {
        int limit = path.size()-1;
        for(int i=0; i<limit; ++i) {
            TextureManager::DrawLine(
                convertWorldToScreen(path[i]), 
                convertWorldToScreen(path[i+1]), 
                {0x00, (uint8_t)i, 0xFF, SDL_ALPHA_OPAQUE}
            );
        }
    }

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

void Game::AddTile(SDL_Texture* t, int id, float width, int map_x, int map_y, const std::vector<std::vector<int>>& layout) {
    auto& tile(manager->addEntity());
    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, t);

    if(id == 4) {
        auto& building(manager->addEntity());
        building.addComponent<TransformComponent>(map_x*width, map_y*width, width, width, 1.0);
        building.addComponent<SpriteComponent>(Game::building_tex);
        building.addComponent<Collider>("base", COLLIDER_HEXAGON);
        building.addComponent<Wireframe>();
        building.addGroup(groupBuildings);
    }

    if(id == 2) {
        uint8_t* neighbors = &tile.getComponent<RectangleCollider>().adjacent_rectangles;
        int dec_map_x = map_x-1;
        int inc_map_x = map_x+1;
        int dec_map_y = map_y-1;
        int inc_map_y = map_y+1;

        bool top_left  = false;
        bool top_mid   = false;
        bool top_right = false;
        bool left      = false;
        bool right     = false;
        bool bot_left  = false;
        bool bot_mid   = false;
        bool bot_right = false;

        int layout_width = layout[0].size();
        int layout_height = layout.size();

        if(map_x == 0) {
            if(map_y == 0) { // top left corner
                right     = layout[    map_y][inc_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            } else if(map_y == layout_height-1) { // bottom left corner
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
            } else { // left column
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            }
        } else if(map_x == layout_width-1) {
            if(map_y == 0) { // top right corner
                left      = layout[    map_y][dec_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
            } else if(map_y == layout_height-1) { // bottom right corner
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
            } else { // right column
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
            }
        } else {
            if(map_y == 0) { // top row
                left      = layout[    map_y][dec_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            } else if(map_y == layout_height-1) { // bottom row
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
            } else { // middle of the layout (most cases)
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            }
        }

        setBit(neighbors, 0, top_left);
        setBit(neighbors, 1, top_mid);
        setBit(neighbors, 2, top_right);
        setBit(neighbors, 3, left);
        setBit(neighbors, 4, right);
        setBit(neighbors, 5, bot_left);
        setBit(neighbors, 6, bot_mid);
        setBit(neighbors, 7, bot_right);
    }
    
    tile.addGroup(groupTiles);
}

void Game::createDrone(float pos_x, float pos_y) {
    auto& new_drone(manager->addEntity());
    new_drone.addComponent<DroneComponent>(Vector2D(pos_x, pos_y), Game::UNIT_SIZE, Game::unit_tex);
    new_drone.addComponent<Wireframe>();
    new_drone.addComponent<TextComponent>("", 160.0f, 16.0f);
    new_drone.addGroup(groupMovables);
}