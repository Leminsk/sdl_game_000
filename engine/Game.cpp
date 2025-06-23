#include <vector>
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
#include "GroupLabels.hpp"
#include "networking/MessageTypes.h"
#include "networking/Client.hpp"
#include "networking/Server.hpp"

Map* map;
Client* client;
Server* server;

Manager* Game::manager = new Manager();
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
Entity& Game::camera(Game::manager->addEntity("CAMERA"));

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
std::vector<std::vector<bool>> Game::collision_mesh_64;
std::vector<std::vector<bool>> Game::collision_mesh_16;
std::vector<std::vector<bool>> Game::collision_mesh_4;
std::vector<std::vector<bool>> Game::collision_mesh_1;

MainColors Game::PLAYER_COLOR = MainColors::NONE;

bool Game::is_client = false;
bool Game::is_server = false;
int Game::SERVER_STATE_SHARE_RATE;
int Game::CLIENT_PING_RATE;
bool Game::update_server = false;
const int Game::PACKET_SIZE = 1300;
int64_t Game::PING_MS;
uint32_t Game::PLAYER_CLIENT_ID;
std::unordered_map<uint32_t, int> clients_ping = {};
std::unordered_map<uint32_t, MainColors> clients_color = {};



std::unordered_map<int, Vector2D> previous_drones_positions;

auto& buildings(Game::manager->getGroup(groupBuildings));
auto& drones(Game::manager->getGroup(groupDrones));
auto& tiles(Game::manager->getGroup(groupTiles));
auto& ui_elements(Game::manager->getGroup(groupUI));

std::vector<Vector2D> path;
std::vector<Entity*> Game::moved_drones;

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
        Game::world_map_layout_width = map->world_layout_width;
        Game::world_map_layout_height = map->world_layout_height;        
    }

    createDrone(  0,   0, MainColors::WHITE);
    // drone1.getComponent<TransformComponent>().velocity = { 0.5f, 0.5f };
    createDrone(100,   0, MainColors::BLACK);
    createDrone(  0, 100, MainColors::RED);
    createDrone(200, 200, MainColors::GREEN);

    map->generateCollisionMesh( 1, Game::collision_mesh_1,  Game::collision_mesh_1_width,  Game::collision_mesh_1_height);
    map->generateCollisionMesh( 4, Game::collision_mesh_4,  Game::collision_mesh_4_width,  Game::collision_mesh_4_height);
    map->generateCollisionMesh(16, Game::collision_mesh_16, Game::collision_mesh_16_width, Game::collision_mesh_16_height);
    map->generateCollisionMesh(64, Game::collision_mesh_64, Game::collision_mesh_64_width, Game::collision_mesh_64_height);

    createSimpleUIText("FPS_COUNTER", Game::SCREEN_WIDTH - 270, 0, Game::SCREEN_WIDTH/3, Game::SCREEN_HEIGHT/16);
}

void handleMouse(SDL_MouseButtonEvent& b) {
    Vector2D world_pos = convertScreenToWorld(Vector2D(b.x, b.y));
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            std::cout << "MOUSE BUTTON LEFT: " << world_pos << '\n'; 
            DroneComponent* drone;
            for(auto& dr : drones) {
                float r = dr->getComponent<CircleCollider>().radius;
                drone = &dr->getComponent<DroneComponent>();
                if(Distance(drone->getPosition(), world_pos) <= r*r && drone->color_type == Game::PLAYER_COLOR) {
                    std::cout << dr->getIdentifier() << " selected\n";
                    drone->selected = true;
                } else {
                    drone->selected = false;
                }
            }
        } break;
            
        case SDL_BUTTON_MIDDLE: std::cout << "MOUSE BUTTON MIDDLE\n"; break;

        case SDL_BUTTON_RIGHT: {
            std::cout << "MOUSE BUTTON RIGHT: " << world_pos << '\n';
            DroneComponent* drone;
            for(auto& dr : drones) {
                drone = &dr->getComponent<DroneComponent>();
                if(drone->selected) {
                    if(Game::is_client) {
                        // store path to send to server, but do not move drone
                        drone->path = find_path(drone->getPosition(), world_pos);
                        drone->destination_position = world_pos;
                        path = drone->path;
                        if(!path.empty()) {
                            Game::moved_drones.push_back(dr);
                            Game::update_server = true;
                        }                 
                    } else {
                        drone->moveToPoint(world_pos);
                        path = drone->path;
                    }
                }
            }  
        } break;
    }
}



void sendStateToServer() {
    if(Game::moved_drones.size() > 0) {
        DroneComponent* drone;
        olc::net::message<MessageTypes> msg;
        msg.header.id = MessageTypes::ClientState_Drones;
        int drone_counter = 0;
        int drone_path_size;
        for(auto& dr : Game::moved_drones) {
            drone = &dr->getComponent<DroneComponent>();
            drone_path_size = static_cast<int>(drone->path.size());
            // send packet if the next drone would surpass 1300 B
            if((static_cast<int>(msg.size()) + 14 + (8 * drone_path_size)) >= Game::PACKET_SIZE) {
                msg << drone_counter; // 4 B
                client->Send(msg);
                msg.body = {};
                msg.header.size = msg.size();
                drone_counter = 0;
            }
            // send in reverse to ease load on server
            for(int j=drone_path_size-1; j>=0; --j) {
                msg << drone->path[j].y; // 4 B
                msg << drone->path[j].x; // 4 B
            } // subtotal: 8 ~ 1024 B (Most paths should be short, but it's technically unbounded)
            msg << drone_path_size; // 4 B
            msg <= dr->getIdentifier(); // 8 B + 2 B (null-terminators)
            ++drone_counter;
        }
        if(drone_counter > 0) { // for loop leftovers
            msg << drone_counter;
            client->Send(msg);
        }
        Game::moved_drones = {};
    }
}

void sendStateToClients() {
    bool send_drones_state = true;
    int current_drone_index = 0;
    int drones_to_send = 0;
    Entity* dr;
    DroneComponent* drone_main;
    TransformComponent* drone_transform;
    // < 1 KB GOOD, 1200 B Okay, 1400 B Erm, 1500 B LIMIT
    olc::net::message<MessageTypes> msg;
    msg.header.id = MessageTypes::ServerState_Drones;
    for(int i=0; i<drones.size(); ++i) {
        if(static_cast<int>(msg.size()) + 26 >= Game::PACKET_SIZE) {
            msg << drones_to_send; // 4 B -> at most msg should have TOTAL: 1304 B
            server->MessageAllClients(msg);
            msg.body = {};
            msg.header.size = msg.size();
            drones_to_send = 0;
        }
        dr = drones[i];

        // TODO: filter and only send data of "active" drones
        drone_transform = &dr->getComponent<TransformComponent>();
        msg << drone_transform->position.y;    // 4 B
        msg << drone_transform->position.x;    // 4 B
        msg << drone_transform->velocity.y;    // 4 B
        msg << drone_transform->velocity.x;    // 4 B
        msg <= dr->getIdentifier();            // 8 B + 2 B (null-terminators)
                                        // TOTAL: 26 B
        ++drones_to_send;
    }
    if(drones_to_send > 0) { // for loop leftovers
        msg << drones_to_send;
        server->MessageAllClients(msg);
    }
}

void handleStateFromServer(olc::net::message<MessageTypes>& msg) {
    // get all new states and overwrite local state when needed
    int drones_to_update;
    std::string current_identifier;
    Entity* drone;
    TransformComponent* drone_transf;
    msg >> drones_to_update;
    for(int i=0; i<drones_to_update; ++i) {
        msg >= current_identifier;
        drone = Game::manager->getEntityFromGroup(current_identifier, groupDrones);
        drone_transf = &drone->getComponent<TransformComponent>();
        msg >> drone_transf->velocity.x;
        msg >> drone_transf->velocity.y;
        msg >> drone_transf->position.x;
        msg >> drone_transf->position.y;
    }
}

void destroyServer() {
    printf("Stopping server\n");
    Game::is_server = false;
    server->~Server();
    server = NULL;
    
}
void destroyClient() {
    printf("Disconnecting client\n");
    Game::is_client = false;
    Game::PLAYER_CLIENT_ID = 0;
    Game::PLAYER_COLOR = MainColors::NONE;
    client->WarnDisconnect();
    client->Disconnect();
    client->~Client();
    client = NULL;    
}

void Game::handleEvents() {
    Game::moved_drones = {};
    TextComponent& fps_text = Game::manager->getEntityFromGroup("FPS_COUNTER", groupUI)->getComponent<TextComponent>();

    if(Game::is_server) {
        server->Update(-1);

    } else if(Game::is_client && client->IsConnected()) {

        while(!client->Incoming().empty()) {
            auto msg = client->Incoming().pop_front().msg;
            switch (msg.header.id) {
                case MessageTypes::ServerAccept: {
                    std::string server_name;
                    msg >= server_name;
                    msg >> Game::PLAYER_CLIENT_ID;
                    std::cout << "Connected to: " << server_name << " as user id [" << Game::PLAYER_CLIENT_ID << "]\n";
                } break;
                case MessageTypes::ServerState_Colors: {
                    int clients_amount;
                    msg >> clients_amount;
                    int id; MainColors c;
                    std::cout << "   id | main_color\n";
                    for(int i=0; i<clients_amount; ++i) {
                        msg >> id;
                        msg >> c;
                        clients_color[id] = c;
                        if(id == Game::PLAYER_CLIENT_ID) {
                            Game::PLAYER_COLOR = c;
                            std::cout << id << " | " << static_cast<int>(c) << " (me)\n";
                        } else {
                            std::cout << id << " | " << static_cast<int>(c) << '\n';
                        }
                    }
                } break;
                case MessageTypes::ServerState_Drones: {
                    handleStateFromServer(msg);
                } break;
                case MessageTypes::ClientPing: {
                    // bounce back to calculate ping on server
                    client->Send(msg);
                } break;
                case MessageTypes::UsersStatus: {
                    int clients_amount;
                    msg >> clients_amount;
                    int id, ping;
                    std::cout << "   id | ping (ms)\n";
                    for(int i=0; i<clients_amount; ++i) {
                        msg >> id;
                        msg >> ping;
                        clients_ping[id] = ping;
                        if(id == Game::PLAYER_CLIENT_ID) {
                            Game::PING_MS = ping;
                            std::cout << id << " | " << ping << " (me)\n";
                        } else {
                            std::cout << id << " | " << ping << '\n';
                        }
                    }
                } break;
            }
        }
        
        
    }

    
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
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    std::cout << "Window Size Change\n";
                    Game::SCREEN_WIDTH = Game::event.window.data1;
                    Game::SCREEN_HEIGHT = Game::event.window.data2;
                    fps_text.setRenderPos(Game::SCREEN_WIDTH - 270, 0, fps_text.w, fps_text.h);
                } break;
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
        if(keystates[SDL_SCANCODE_C] && !Game::is_client) {
            if(Game::is_server) { destroyServer(); }
            Game::is_client = true;
            client = new Client();
            client->Connect("[IPv4|IPv6]_HERE", 50000);
            printf("Client UP\n");            
        }
        if(keystates[SDL_SCANCODE_S] && !Game::is_server) {
            if(Game::is_client) { destroyClient(); }
            Game::is_server = true;
            server = new Server();
            server->Start();
            Game::PLAYER_COLOR = MainColors::WHITE;
            printf("Server UP\n");
        }
        if(keystates[SDL_SCANCODE_DELETE]) {
            if(Game::is_server) { destroyServer(); }
            if(Game::is_client) { destroyClient(); }
        }




        // Camera controls
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

    if(Game::is_client) {
        if(Game::update_server) {
            sendStateToServer();
            Game::update_server = false;
        }

    } else if(Game::is_server) {
        if(Game::FRAME_COUNT % Game::CLIENT_PING_RATE == 0) { // once every 3 s
           server->PingAllClients();
        }

        if(Game::FRAME_COUNT % Game::SERVER_STATE_SHARE_RATE == 0) { // once every 3 frames
            sendStateToClients();
        }     
    }

    fps_text.setText("FPS:" + format_decimal(Game::AVERAGE_FPS, 3, 2, false));
}

void Game::update() {
    for(int i=0; i<drones.size(); ++i) {
        previous_drones_positions[i] = drones[i]->getComponent<TransformComponent>().position;
    }

    Game::manager->refresh();
    Game::manager->preUpdate();
    Game::manager->update();

    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleStaticCollisions(previous_drones_positions[i], tiles, buildings); }
    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleDynamicCollisions(drones); }
    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleCollisionTranslations(); }
    for(int i=0; i<drones.size(); ++i) { drones[i]->getComponent<DroneComponent>().handleOutOfBounds(Game::world_map_layout_width, Game::world_map_layout_height); }

    Game::camera.getComponent<TextComponent>().setText(
        "Camera center: " + Game::camera.getComponent<TransformComponent>().getCenter().FormatDecimal(4,0)
    );
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
    if(Game::is_server) { destroyServer(); }
    if(Game::is_client) { destroyClient(); }

    Game::manager = NULL;
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
    auto& tile(Game::manager->addEntity("tile-"+std::to_string(map_x)+','+std::to_string(map_y)));
    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, t);

    if(id == 4) {
        auto& building(Game::manager->addEntity("base"));
        building.addComponent<TransformComponent>(map_x*width, map_y*width, width, width, 1.0);
        building.addComponent<SpriteComponent>(Game::building_tex);
        building.addComponent<Collider>(ColliderType::HEXAGON);
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

Entity& Game::createDrone(float pos_x, float pos_y, MainColors c) {
    auto& new_drone(Game::manager->addEntity("DRO" + left_pad_int(Game::UNIT_COUNTER, 5)));
    new_drone.addComponent<DroneComponent>(Vector2D(pos_x, pos_y), Game::UNIT_SIZE, Game::unit_tex, c);
    new_drone.addComponent<Wireframe>();
    new_drone.addComponent<TextComponent>("", 0, 0, 160.0f, 16.0f);
    new_drone.addGroup(groupDrones);
    return new_drone;
}

Entity& Game::createSimpleUIText(std::string id, int pos_x, int pos_y, int width, int height, std::string text) {
    auto& new_ui_text(Game::manager->addEntity(id));
    new_ui_text.addComponent<TextComponent>(
        text, 
        static_cast<float>(pos_x), static_cast<float>(pos_y),
        static_cast<float>(width), static_cast<float>(height),
        Game::default_text_color, true
    );
    new_ui_text.addGroup(groupUI);
    return new_ui_text;
}