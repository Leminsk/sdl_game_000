#include <vector>
#include "../networking/MessageTypes.h"
#include "../networking/Client.hpp"
#include "../networking/Server.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "utils.hpp"
#include "Map.hpp"
#include "TextureManager.hpp"
#include "ECS/ECS.hpp"
#include "ECS/MainColors.hpp"
#include "ECS/Components.hpp"
#include "ECS/Colliders/Collider.hpp"
#include "ECS/Colliders/Collision.hpp"
#include "path_finding.hpp"
#include "Camera.hpp"

Map* map;
Manager* manager = new Manager();
const int Game::UNIT_SIZE = 32;
int Game::SCREEN_HEIGHT;
int Game::SCREEN_WIDTH;
bool Game::isRunning = false;
uint64_t Game::FRAME_COUNT;
float Game::AVERAGE_FPS;
float Game::FRAME_DELTA = 0.0f;
int Game::UNIT_COUNTER = 0;

SDL_Color Game::bg_color{ 220, 220, 220, SDL_ALPHA_OPAQUE };
TTF_Font *Game::default_font;
SDL_Color Game::default_text_color{ 0, 0, 0, SDL_ALPHA_OPAQUE };

SDL_Renderer *Game::renderer = nullptr;
SDL_Event Game::event;
Entity& Game::camera(manager->addEntity("CAMERA"));

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

bool Game::client_up = false;
bool Game::server_up = false;
Client* Game::client;
Server* Game::server;

std::unordered_map<int, Vector2D> previous_drones_positions;

enum groupLabels : size_t {
    groupTiles,
    groupDrones,
    groupBuildings,
    groupUI
};

auto& buildings(manager->getGroup(groupBuildings));
auto& drones(manager->getGroup(groupDrones));
auto& tiles(manager->getGroup(groupTiles));
auto& ui_elements(manager->getGroup(groupUI));

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

    // white helps with color modulation
    Game::unit_tex     = TextureManager::LoadTexture("assets/white_circle.png");
    Game::building_tex = TextureManager::LoadTexture("assets/green_hexagon.png");

    map = new Map("assets/test3.bmp", Game::UNIT_SIZE<<1);
    map->LoadMapRender();
    if(map->loaded) {
        std::cout << "Map loaded.\n";
    }

    createDrone(  0,   0, WHITE);
    // drone1.getComponent<TransformComponent>().velocity = { 0.5f, 0.5f };
    createDrone(100,   0, BLACK);
    createDrone(  0, 100, RED);
    createDrone(200, 200, GREEN);

    map->generateCollisionMesh( 1, Game::collision_mesh_1,  Game::collision_mesh_1_width,  Game::collision_mesh_1_height);
    map->generateCollisionMesh( 4, Game::collision_mesh_4,  Game::collision_mesh_4_width,  Game::collision_mesh_4_height);
    map->generateCollisionMesh(16, Game::collision_mesh_16, Game::collision_mesh_16_width, Game::collision_mesh_16_height);
    map->generateCollisionMesh(64, Game::collision_mesh_64, Game::collision_mesh_64_width, Game::collision_mesh_64_height);

    createSimpleUIText("FPS_COUNTER", Game::SCREEN_WIDTH - (Game::SCREEN_WIDTH/3), 0, Game::SCREEN_WIDTH/3, Game::SCREEN_HEIGHT/16);
}

void handleMouse(SDL_MouseButtonEvent& b) {
    Vector2D world_pos = convertScreenToWorld(Vector2D(b.x, b.y));
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            std::cout << "MOUSE BUTTON LEFT: " << world_pos << '\n'; 
            for(auto& dr : drones) {
                float r = dr->getComponent<CircleCollider>().radius;
                if(Distance(dr->getComponent<DroneComponent>().getPosition(), world_pos) <= r*r) {
                    std::cout << dr->getIdentifier() << " selected\n";
                    dr->getComponent<DroneComponent>().selected = true;
                } else {
                    dr->getComponent<DroneComponent>().selected = false;
                }
            }            
            break;
        }
            
        case SDL_BUTTON_MIDDLE: std::cout << "MOUSE BUTTON MIDDLE\n"; break;

        case SDL_BUTTON_RIGHT: {
            std::cout << "MOUSE BUTTON RIGHT: " << world_pos << '\n';
            for(auto& dr : drones) {
                if(dr->getComponent<DroneComponent>().selected) {
                    dr->getComponent<DroneComponent>().moveToPoint(world_pos);
                    path = dr->getComponent<DroneComponent>().path;
                }
            }
            break;
        }
    }
}

void Game::handleOnline() {
    // is client is server frame count
    std::cout << "Average FPS: " << Game::AVERAGE_FPS << '\n';
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
        if(keystates[SDL_SCANCODE_C] && !Game::client_up) {
            if(Game::server_up) {
                printf("Stopping server\n");
                Game::server->~Server();
                Game::server = NULL;
                Game::server_up = false;
            }
            Game::client = new Client();
            printf("Client UP\n");
            Game::client_up = true;
        }
        if(keystates[SDL_SCANCODE_S] && !Game::server_up) {
            if(Game::client_up) {
                printf("Disconnecting client\n");
                Game::client->~Client();
                Game::client = NULL;
                Game::client_up = false;
            }
            Game::server = new Server();
            printf("Server UP\n");
            Game::server_up = true;
        }



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
    for(int i=0; i<drones.size(); ++i) {
        previous_drones_positions[i] = drones[i]->getComponent<TransformComponent>().position;
    }

    manager->refresh();
    manager->preUpdate();
    manager->update();

    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleStaticCollisions(previous_drones_positions[i], tiles, buildings); }
    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleDynamicCollisions(drones); }
    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleCollisionTranslations(); }
    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleOutOfBounds(map->world_layout_width, map->world_layout_height); }

    Game::camera.getComponent<TextComponent>().setText(
        "Camera center: " + Game::camera.getComponent<TransformComponent>().getCenter().FormatDecimal(4,0)
    );

    manager->getEntity("FPS_COUNTER")->getComponent<TextComponent>().setText("FPS:" + format_decimal(Game::AVERAGE_FPS, 3, 2, false));

}



void Game::render() {
    SDL_RenderClear(Game::renderer);
    for(auto& t : tiles) { t->draw(); }
    for(auto& b : buildings) { b->draw(); }
    for(auto& dr : drones) { dr->draw(); }
    for(auto& ui : ui_elements) { ui->draw(); }
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
    manager = NULL;
    map = NULL;
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
    auto& tile(manager->addEntity("tile-"+std::to_string(map_x)+','+std::to_string(map_y)));
    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, t);

    if(id == 4) {
        auto& building(manager->addEntity("base"));
        building.addComponent<TransformComponent>(map_x*width, map_y*width, width, width, 1.0);
        building.addComponent<SpriteComponent>(Game::building_tex);
        building.addComponent<Collider>(COLLIDER_HEXAGON);
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

Entity& Game::createDrone(float pos_x, float pos_y, main_color c) {
    auto& new_drone(manager->addEntity("Drone_"+std::to_string(Game::UNIT_COUNTER)));
    new_drone.addComponent<DroneComponent>(Vector2D(pos_x, pos_y), Game::UNIT_SIZE, Game::unit_tex, c);
    new_drone.addComponent<Wireframe>();
    new_drone.addComponent<TextComponent>("", 0, 0, 160.0f, 16.0f);
    new_drone.addGroup(groupDrones);
    return new_drone;
}

Entity& Game::createSimpleUIText(std::string id, int pos_x, int pos_y, int width, int height, std::string text) {
    auto& new_ui_text(manager->addEntity(id));
    new_ui_text.addComponent<TextComponent>(
        text, 
        static_cast<float>(pos_x), static_cast<float>(pos_y),
        static_cast<float>(width), static_cast<float>(height),
        Game::default_text_color, true
    );
    new_ui_text.addGroup(groupUI);
    return new_ui_text;
}