#pragma once

#include "HexagonGrid.hpp"
#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "TextureManager.hpp"
#include "AudioManager.hpp"
#include "Map.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"
#include "networking/MessageTypes.h"
#include "networking/Client.hpp"
#include "networking/Server.hpp"

class SceneMatchGame {
private:
SDL_Event* event;
// --------------------------------  TEXTURES  --------------------------------
SDL_Texture* plain_terrain_texture;
SDL_Texture* rough_terrain_texture;
SDL_Texture* mountain_texture;
SDL_Texture* water_bg_texture;
SDL_Texture* water_fg_texture;
// -------------------------------- ---------- --------------------------------

// --------------------------------  AUDIO  --------------------------------
Mix_Music* music_main_menu = NULL;
Mix_Chunk* sound_button = NULL;
// --------------------------------  -----  --------------------------------



bool draw_grids = false;
bool pressed_toggle_collision_mesh_crosshair = false;
int toggle_collision_mesh_crosshair = -1;
std::vector<Vector2D> hex_tile = { {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0} };
Vector2D clicked_point;
Vector2D converted_point;

Map* map = nullptr;


// --------------------------- NETWORKING ------------------------
Client* client;
Server* server;
bool is_server = false;
bool is_client = false;
std::unordered_map<uint32_t, int> clients_ping = {};
std::unordered_map<uint32_t, MainColors> clients_color = {};
MainColors PLAYER_COLOR = MainColors::NONE;
bool update_server = false;
const int PACKET_SIZE = 1300; // the ideal max size in bytes. 10 bytes or so over it is fine
int64_t PING_MS; // this client's ping on the server
uint32_t PLAYER_CLIENT_ID; // this client's ID on the server
// drones which have been selected and have had their moveToPoint invoked on this Client. Their paths should then be sent to the server
std::vector<Entity*> moved_drones = {};

std::unordered_map<int, Vector2D> previous_drones_positions = {};
std::vector<Vector2D> path_to_draw = {};

// --------------------------- ---------- ------------------------



std::vector<Entity*>&      buildings = Game::manager->getGroup(groupBuildings);
std::vector<Entity*>&         drones = Game::manager->getGroup(groupDrones);
std::vector<Entity*>&          tiles = Game::manager->getGroup(groupTiles);

std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);

public:


TextComponent* fps_text;

SceneMatchGame(SDL_Event* e) { this->event = e; }
~SceneMatchGame() {
    if(this->map) {
        delete this->map;
        this->map = nullptr;
    }    
}


Entity& AddTileOnMap(int id, float width, int map_x, int map_y, std::vector<std::vector<int>>& layout, const std::vector<std::vector<SDL_Color>>& map_pixels = {}) {
    auto& tile(Game::manager->addEntity("tile-"+std::to_string(map_x)+','+std::to_string(map_y)));
    tile.reserveComponents(3);
    const float world_x = map_x * width;
    const float world_y = map_y * width;
    
    switch(id) {
        case TILE_ROUGH:  {
            tile.addComponent<TileComponent>(world_x, world_y, width, width, id, this->rough_terrain_texture);
        } break;
        case TILE_IMPASSABLE: { 
            tile.addComponent<TileComponent>(world_x, world_y, width, width, id, this->mountain_texture);
            uint8_t* neighbors = &tile.getComponent<RectangleCollider>().adjacent_rectangles;
            SetSolidTileNeighbors(neighbors, map_x, map_y, layout);
        } break;
        case TILE_NAVIGABLE: { 
            // need to be in this order to render the foreground "above" the background
            tile.addComponent<TileComponent>(world_x, world_y, width, width, id, this->water_bg_texture);
            tile.addComponent<TileFGComponent>(world_x, world_y, width, width, id, this->water_fg_texture);
        }
        break;
        case TILE_BASE_SPAWN: {
            tile.addComponent<TileComponent>(world_x, world_y, width, width, id, this->plain_terrain_texture);
            /*
            TODO: 
            - assign random color here or get it from somewhere
            - deal with spawn collision like in the TILE_PLAYER case
            */
            createBaseBuilding(
                "base_"+std::to_string((int)convertSDLColorToMainColor(map_pixels[map_y][map_x])), 
                world_x, world_y, width, map_pixels[map_y][map_x]
            );
        } break;
        case TILE_PLAYER: {
            // whether it created a building or not, set as TILE_PLAIN regardless
            layout[map_y][map_x] = tile_type::TILE_PLAIN;
            tile.addComponent<TileComponent>(world_x, world_y, width, width, id, this->plain_terrain_texture);

            Vector2D tile_xy = Vector2D(map_x, map_y);
            Vector2D tile_center = Vector2D(world_x + width/2, world_y + width/2);
            HexPos hex_tile = convertWorldToHex(tile_center);
            std::cout << "TILE_PLAYER placed on hex tile {" << hex_tile.q << " , " << hex_tile.r << " }\n";
            std::vector<Vector2D> hex_hull = getPointsFromHexPos(hex_tile);

            Entity* created_building = nullptr;

            if(this->map->hexFreeInMap(hex_hull, tile_xy)) {
                Vector2D hex_center = convertHexToWorld(hex_tile);
                std::cout << "success on first pass: " << hex_center << '\n';
                created_building = &createBaseBuilding(
                    "base_"+std::to_string((int)convertSDLColorToMainColor(map_pixels[map_y][map_x])), 
                    hex_center.x - HEX_SIDE_LENGTH, hex_center.y - HEX_SIDE_LENGTH, width, map_pixels[map_y][map_x]
                );

            } else {
                std::cout << "else\n";
                Vector2D hex_tile_pos = convertHexToWorld(hex_tile);
                Vector2D neighbor_pos;
                std::vector<HexPos> hex_neighbors = hexNeighbors(hex_tile, this->map->hex_grid_rect);
                bool valid_spawn = false;
                float free_pos_x, free_pos_y;
                // first pass to prioritize spawn in same tile
                for(HexPos& n : hex_neighbors) {
                    neighbor_pos = convertHexToWorld(n);
                    if(this->map->getTileCoordFromWorldPos(hex_tile_pos) == this->map->getTileCoordFromWorldPos(neighbor_pos)) {
                        hex_hull = getPointsFromHexPos(n);
                        if(this->map->hexFreeInMap(hex_hull, tile_xy)) {
                            valid_spawn = true;
                            free_pos_x = neighbor_pos.x - HEX_SIDE_LENGTH;
                            free_pos_y = neighbor_pos.y - HEX_SIDE_LENGTH;
                            break;
                        }                        
                    }
                }
                // second pass to attempt any other free adjacent tile 
                if(!valid_spawn) {
                    for(HexPos& n : hex_neighbors) {
                        neighbor_pos = convertHexToWorld(n);
                        hex_hull = getPointsFromHexPos(n);
                        if(this->map->hexFreeInMap(hex_hull, tile_xy)) {
                            valid_spawn = true;
                            free_pos_x = neighbor_pos.x - HEX_SIDE_LENGTH;
                            free_pos_y = neighbor_pos.y - HEX_SIDE_LENGTH;
                            break;
                        }
                    }
                }
                if(valid_spawn) {
                    std::cout << "success on second pass: " << free_pos_x << ", " << free_pos_y << '\n';
                    created_building = &createBaseBuilding(
                        "base_"+std::to_string((int)convertSDLColorToMainColor(map_pixels[map_y][map_x])), 
                        free_pos_x, free_pos_y, width, map_pixels[map_y][map_x]
                    );
                } else {
                    std::cout << "WARNING: can't create hex building from tile: " << tile_xy << '\n';
                }
            }
        } break;
        default: 
            tile.addComponent<TileComponent>(world_x, world_y, width, width, id, this->plain_terrain_texture); 
    }
    
    tile.addGroup(groupTiles);
    return tile;
}
void LoadMapRender(float tile_scale=1.0f) {
    uint64_t row, column;
    SDL_Texture** tex = nullptr;
    const float scaled_width = this->map->tile_width * tile_scale;
    for(row = 0; row < this->map->layout_height; ++row) {
        for(column = 0; column < this->map->layout_width; ++column) {
            AddTileOnMap(
                this->map->layout[row][column], 
                scaled_width,
                column, row,
                this->map->layout,
                this->map->map_pixels
            );
        }
    }
}

void setScene(
    const std::vector<std::vector<SDL_Color>>& map_pixels, const SDL_Color& player_color, const std::pair<int,int>& player_spawn,
    const std::vector<std::pair<int,int>>& spawn_positions,
    SDL_Texture* plain, SDL_Texture* rough, SDL_Texture* mountain, SDL_Texture* water_bg, SDL_Texture* water_fg, 
    TextComponent* fps
) {
    Mix_HaltMusic();
    this->PLAYER_COLOR = convertSDLColorToMainColor(player_color);
    Game::default_bg_color = COLORS_ROUGH;

    this->plain_terrain_texture = plain;
    this->rough_terrain_texture = rough;
    this->mountain_texture = mountain;
    this->water_bg_texture = water_bg;
    this->water_fg_texture = water_fg;
    this->fps_text = fps;

    this->map = new Map(
        map_pixels, 
        this->plain_terrain_texture,
        this->rough_terrain_texture,
        this->mountain_texture,
        this->water_bg_texture,
        this->water_fg_texture,
        Game::DOUBLE_UNIT_SIZE
    );

    if(this->map->loaded) {
        printf("Map  x: %d  by  y: %d\n", this->map->layout_width, this->map->layout_height);
        const int tiles_amount = this->map->layout_width * this->map->layout_height;
        Game::manager->reserveEntities(tiles_amount);
        this->tiles.reserve(tiles_amount);
        LoadMapRender();
        Game::world_map_layout_width = this->map->world_layout_width;
        Game::world_map_layout_height = this->map->world_layout_height;
        Game::camera_position = this->map->getWorldPosFromTileCoord(player_spawn.second, player_spawn.first) - Vector2D(Game::SCREEN_WIDTH>>1, Game::SCREEN_HEIGHT>>1);

        this->map->generateCollisionMesh( 1, Game::collision_mesh_1,  Game::collision_mesh_1_width,  Game::collision_mesh_1_height,  this->buildings);
        this->map->generateCollisionMesh( 4, Game::collision_mesh_4,  Game::collision_mesh_4_width,  Game::collision_mesh_4_height,  this->buildings);
        this->map->generateCollisionMesh(16, Game::collision_mesh_16, Game::collision_mesh_16_width, Game::collision_mesh_16_height, this->buildings);
        this->map->generateCollisionMesh(64, Game::collision_mesh_64, Game::collision_mesh_64_width, Game::collision_mesh_64_height, this->buildings);
        this->map->generateCollisionMacroMesh( 4, Game::collision_mesh_macro_4,  Game::collision_mesh_macro_4_width,  Game::collision_mesh_macro_4_height);

        for(const std::pair<int,int>& pos : spawn_positions) {
            MainColors c = convertSDLColorToMainColor(map_pixels[pos.first][pos.second]);
            Vector2D world_pos = this->map->getWorldPosFromTileCoord(pos.second, pos.first-1);
            createDrone(world_pos.x, world_pos.y, c);
        }

        createUISimpleText("crosshair", 0, 0, "Crosshair: (-0000,-0000)");
        createUISimpleText("camera_zoom", 0, 30, "Camera zoom: 0.0");

    } else {
        printf("Map failed to load.\n");
    }
}



// --------------------------------- MOUSE ----------------------------------------
void handleMouse(SDL_MouseButtonEvent& b) {
    Vector2D world_pos = convertScreenToWorld(Vector2D(b.x, b.y));
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            std::cout << "MOUSE BUTTON LEFT: " << world_pos << '\n'; 
            DroneComponent* drone;
            for(auto& dr : this->drones) {
                float r = dr->getComponent<CircleCollider>().radius;
                drone = &dr->getComponent<DroneComponent>();
                if(Distance(drone->getPosition(), world_pos) <= r*r && drone->color_type == this->PLAYER_COLOR) {
                    std::cout << dr->getIdentifier() << " selected\n";
                    drone->selected = true;
                } else {
                    drone->selected = false;
                }
            }
        } break;
            
        case SDL_BUTTON_MIDDLE: {
            std::cout << "MOUSE BUTTON MIDDLE: " << world_pos << '\n';
            HexPos h = convertWorldToHex(world_pos);
            Vector2D tile_xy = this->map->getTileCoordFromWorldPos(world_pos);
            std::cout << "getTileCoordFromWorldPos: " << tile_xy << '\n';
            std::cout << "convertWorldToHex: " << h.q << ',' << h.r << '\n';
            std::cout << "convertHexToWorld: " << convertHexToWorld(h) << '\n';
            std::vector<Vector2D> hex_points = getPointsFromHexPos(h);
            std::cout << "hexOverlapsInMap: " << (this->map->hexFreeInMap(hex_points, tile_xy) ? "Clear" : "BLOCKED") << '\n';

        } break;

        case SDL_BUTTON_RIGHT: {
            std::cout << "MOUSE BUTTON RIGHT: " << world_pos << '\n';
            DroneComponent* drone;
            for(auto& dr : this->drones) {
                drone = &dr->getComponent<DroneComponent>();
                if(drone->selected) {
                    if(this->is_client) {
                        // store path to send to server, but do not move drone
                        drone->path = find_path(drone->getPosition(), world_pos, drone->offcourse_limit);
                        drone->destination_position = world_pos;
                        this->path_to_draw = drone->path;
                        if(!this->path_to_draw.empty()) {
                            this->moved_drones.push_back(dr);
                            this->update_server = true;
                        }                 
                    } else {
                        drone->moveToPoint(world_pos);
                        this->path_to_draw = drone->path;
                    }
                }
            }  
        } break;
    }
}
void handleMouseRelease(SDL_MouseButtonEvent& b) {
    if(b.button == SDL_BUTTON_MIDDLE) {
        this->draw_grids = !this->draw_grids;
    }
}
// --------------------------------- ===== ----------------------------------------




void sendStateToServer() {
    if(this->moved_drones.size() > 0) {
        DroneComponent* drone;
        olc::net::message<MessageTypes> msg;
        msg.header.id = MessageTypes::ClientState_Drones;
        int drone_counter = 0;
        int drone_path_size;
        for(auto& dr : this->moved_drones) {
            drone = &dr->getComponent<DroneComponent>();
            drone_path_size = static_cast<int>(drone->path.size());
            // send packet if the next drone would surpass 1300 B
            if((static_cast<int>(msg.size()) + 14 + (8 * drone_path_size)) >= this->PACKET_SIZE) {
                msg << drone_counter; // 4 B
                this->client->Send(msg);
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
            msg << drone->offcourse_limit; // 4 B
            msg <= dr->getIdentifier(); // 8 B
            ++drone_counter;
        }
        if(drone_counter > 0) { // for loop leftovers
            msg << drone_counter;
            this->client->Send(msg);
        }
        this->moved_drones = {};
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
    for(int i=0; i<this->drones.size(); ++i) {
        if(static_cast<int>(msg.size()) + 26 >= this->PACKET_SIZE) {
            msg << drones_to_send; // 4 B -> at most msg should have TOTAL: 1304 B
            this->server->MessageAllClients(msg);
            msg.body = {};
            msg.header.size = msg.size();
            drones_to_send = 0;
        }
        dr = this->drones[i];

        // TODO: filter and only send data of "active" drones
        drone_transform = &dr->getComponent<TransformComponent>();
        msg << drone_transform->position.y;    // 4 B
        msg << drone_transform->position.x;    // 4 B
        msg << drone_transform->velocity.y;    // 4 B
        msg << drone_transform->velocity.x;    // 4 B
        msg <= dr->getIdentifier();            // 8 B
                                        // TOTAL: 24 B
        ++drones_to_send;
    }
    if(drones_to_send > 0) { // for loop leftovers
        msg << drones_to_send;
        this->server->MessageAllClients(msg);
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
    this->is_server = false;
    delete this->server;
    this->server = nullptr;
}
void destroyClient() {
    printf("Disconnecting client\n");
    this->is_client = false;
    this->PLAYER_CLIENT_ID = 0;
    this->PLAYER_COLOR = MainColors::NONE;
    this->client->WarnDisconnect();
    this->client->Disconnect();
    delete this->client;
    this->client = nullptr;
}









void handleEventsPrePoll() {
    // for multiplayer
    this->moved_drones = {};
    if(this->is_server) {

        this->server->Update(-1);
        
    } else if(this->is_client && this->client->IsConnected()) {

        while(!this->client->Incoming().empty()) {
            auto msg = this->client->Incoming().pop_front().msg;
            switch (msg.header.id) {
                case MessageTypes::ServerAccept: {
                    std::string server_name;
                    msg >= server_name;
                    msg >> this->PLAYER_CLIENT_ID;
                    std::cout << "Connected to: " << server_name << " as user id [" << this->PLAYER_CLIENT_ID << "]\n";
                } break;
                case MessageTypes::ServerState_Colors: {
                    int clients_amount;
                    msg >> clients_amount;
                    int id; MainColors c;
                    std::cout << "   id | main_color\n";
                    for(int i=0; i<clients_amount; ++i) {
                        msg >> id;
                        msg >> c;
                        this->clients_color[id] = c;
                        if(id == this->PLAYER_CLIENT_ID) {
                            this->PLAYER_COLOR = c;
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
                    this->client->Send(msg);
                } break;
                case MessageTypes::UsersStatus: {
                    int clients_amount;
                    msg >> clients_amount;
                    int id, ping;
                    std::cout << "   id | ping (ms)\n";
                    for(int i=0; i<clients_amount; ++i) {
                        msg >> id;
                        msg >> ping;
                        this->clients_ping[id] = ping;
                        if(id == this->PLAYER_CLIENT_ID) {
                            this->PING_MS = ping;
                            std::cout << id << " | " << ping << " (me)\n";
                        } else {
                            std::cout << id << " | " << ping << '\n';
                        }
                    }
                } break;
            }
        }

    }
}
void handleEventsPollEvent() {
    while( SDL_PollEvent(this->event) ) {
        if(this->event->type == SDL_QUIT) {
            Game::isRunning = false;
            return;
        }

        if(this->event->type == SDL_KEYUP) {
            if(this->event->key.keysym.scancode == SDL_SCANCODE_F) {
                this->pressed_toggle_collision_mesh_crosshair = true;
            }
        }

        if(this->event->type == SDL_MOUSEBUTTONDOWN) {
            handleMouse(this->event->button);
        }

        if(this->event->type == SDL_MOUSEBUTTONUP) {
            handleMouseRelease(this->event->button);
        }

        if(this->event->type == SDL_MOUSEWHEEL) {
            int wheel_y = this->event->wheel.y;
            if(wheel_y > 0) { // zoom in
                Game::camera_zoom = std::min(Game::camera_zoom + 0.25f, 4.0f);
            } else if(wheel_y < 0) { // zoom out
                Game::camera_zoom = std::max(Game::camera_zoom - 0.25f, 0.5f);
            }
        }

        if(this->event->type == SDL_WINDOWEVENT) {
            switch(this->event->window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    std::cout << "Window Size Change\n";
                    Game::SCREEN_WIDTH = this->event->window.data1;
                    Game::SCREEN_HEIGHT = this->event->window.data2;
                    Game::camera_focus.x = Game::SCREEN_WIDTH>>1;
                    Game::camera_focus.y = Game::SCREEN_HEIGHT>>1;
                    this->fps_text->setRenderPos(Game::SCREEN_WIDTH - (this->fps_text->w+3), 3, this->fps_text->w, this->fps_text->h);
                } break;
                case SDL_WINDOWEVENT_ENTER: std::cout << "Mouse IN\n"; break;
                case SDL_WINDOWEVENT_LEAVE: std::cout << "Mouse OUT\n"; break;
                case SDL_WINDOWEVENT_FOCUS_GAINED: std::cout << "Keyboard IN\n"; break;
                case SDL_WINDOWEVENT_FOCUS_LOST: std::cout << "Keyboard OUT\n"; break;
            }
        }
        
    }
}
void handleEventsPostPoll(const uint8_t *keystates) {
    if(keystates[SDL_SCANCODE_ESCAPE]) { 
            Game::isRunning = false; 
    } else {
        if(keystates[SDL_SCANCODE_O] && !this->is_client) {
            if(this->is_server) { destroyServer(); }
            this->is_client = true;
            this->client = new Client();
            this->client->Connect("[IPv4|IPv6]", 50000);
            printf("Client UP\n");            
        }
        if(keystates[SDL_SCANCODE_P] && !this->is_server) {
            if(this->is_client) { destroyClient(); }
            this->is_server = true;
            this->server = new Server();
            this->server->Start();
            this->PLAYER_COLOR = MainColors::WHITE;
            printf("Server UP\n");
        }
        if(keystates[SDL_SCANCODE_DELETE]) {
            if(this->is_server) { destroyServer(); }
            if(this->is_client) { destroyClient(); }
        }


        if(this->pressed_toggle_collision_mesh_crosshair && keystates[SDL_SCANCODE_F]) { 
            ++this->toggle_collision_mesh_crosshair;
            if(this->toggle_collision_mesh_crosshair >= 5) {
                this->toggle_collision_mesh_crosshair = -1;
            }
            this->pressed_toggle_collision_mesh_crosshair = false;
        }


        if(keystates[SDL_SCANCODE_W]) { Game::camera_velocity.y =  -2.0f / Game::camera_zoom; }
        if(keystates[SDL_SCANCODE_S]) { Game::camera_velocity.y =   2.0f / Game::camera_zoom; }
        if(keystates[SDL_SCANCODE_A]) { Game::camera_velocity.x =  -2.0f / Game::camera_zoom; }
        if(keystates[SDL_SCANCODE_D]) { Game::camera_velocity.x =   2.0f / Game::camera_zoom; }

        if(!keystates[SDL_SCANCODE_W] && !keystates[SDL_SCANCODE_S]) { Game::camera_velocity.y = 0.0f; }
        if(!keystates[SDL_SCANCODE_A] && !keystates[SDL_SCANCODE_D]) { Game::camera_velocity.x = 0.0f; }
        
        if(keystates[SDL_SCANCODE_SPACE]) {
            std::cout << "map{x , y}: " << this->map->layout.size() << ',' << this->map->layout[0].size() << "tile_width: " << this->map->tile_width << '\n';
        }
    }

    if(this->is_client) {
        if(this->update_server) {
            sendStateToServer();
            this->update_server = false;
        }

    } else if(this->is_server) {
        if(Game::FRAME_COUNT % Game::CLIENT_PING_RATE == 0) { // once every 3 s
            this->server->PingAllClients();
        }

        if(Game::FRAME_COUNT % Game::SERVER_STATE_SHARE_RATE == 0) { // once every 3 frames
            sendStateToClients();
        }     
    }
}




void update() {
    for(int i=0; i<this->drones.size(); ++i) {
        this->previous_drones_positions[i] = this->drones[i]->getComponent<TransformComponent>().position;
    }

    Game::manager->refresh();
    Game::manager->preUpdate();
    Game::manager->update();

    // special case for camera
    Game::camera_position = Game::camera_position + (Game::camera_velocity * Game::DEFAULT_SPEED * Game::FRAME_DELTA);

    for(int i=0; i<this->drones.size(); ++i) { this->drones[i]->getComponent<DroneComponent>().handleStaticCollisions(this->previous_drones_positions[i], this->tiles, this->buildings); }
    for(int i=0; i<this->drones.size(); ++i) { this->drones[i]->getComponent<DroneComponent>().handleDynamicCollisions(this->drones); }
    for(int i=0; i<this->drones.size(); ++i) { this->drones[i]->getComponent<DroneComponent>().handleCollisionTranslations(); }
    for(int i=0; i<this->drones.size(); ++i) { this->drones[i]->getComponent<DroneComponent>().handleOutOfBounds(Game::world_map_layout_width, Game::world_map_layout_height); }

    Game::manager->getEntityFromGroup("crosshair", groupUI)->getComponent<TextComponent>().setText(
        "Crosshair: " + convertScreenToWorld(Vector2D(Game::SCREEN_WIDTH>>1, Game::SCREEN_HEIGHT>>1)).FormatDecimal(4,0)
    );
    Game::manager->getEntityFromGroup("camera_zoom", groupUI)->getComponent<TextComponent>().setText(
        "Camera zoom: " + format_decimal(Game::camera_zoom, 1, 1, false)
    );
}
void render() {
    for(auto& t : this->tiles) { t->draw(); }
    for(auto& b : this->buildings) { b->draw(); }
    for(auto& dr : this->drones) { dr->draw(); }
    for(auto& bg_ui : this->bg_ui_elements) { bg_ui->draw(); }
    for(auto& ui : this->ui_elements) { ui->draw(); }
    for(auto& pr_ui : this->pr_ui_elements) { pr_ui->draw(); }

    // draw the path trajectory for debugging
    if(this->path_to_draw.size() > 0) {
        int limit = this->path_to_draw.size()-1;
        for(int i=0; i<limit; ++i) {
            TextureManager::DrawLine(
                convertWorldToScreen(this->path_to_draw[i]), 
                convertWorldToScreen(this->path_to_draw[i+1]), 
                COLORS_RED
            );
            Vector2D p_pos = convertWorldToScreen(this->path_to_draw[i]);
            SDL_FRect path_point = { p_pos.x - 2.0f, p_pos.y - 2.0f, 4.0f, 4.0f };
            TextureManager::DrawRect(&path_point, COLORS_CYAN);
        }
    }


    // debugging hex grid
    if(this->draw_grids) {
        int max_q_axis = (this->map->layout_width<<1) / sqrt_3;
        int max_r_axis = (this->map->layout_height<<1) / 1.5f;
        int min_q_axis = -(max_q_axis>>1);

        HexPos current_hex = { 0, 0 };
        Vector2D hex_pos, hex_world_pos;
        float hex_pos_w = 4.0f;
        SDL_Color hex_border_color = { 0x00, 0xF0, 0x20, SDL_ALPHA_OPAQUE };
        SDL_FRect hex_center = { hex_pos.x, hex_pos.y, hex_pos_w, hex_pos_w };
        for(int q=min_q_axis; q<=max_q_axis; ++q) {
            current_hex.q = q;
            for(int r=0; r<=max_r_axis; ++r) {
                current_hex.r = r;
                hex_world_pos = convertHexToWorld(current_hex);

                if(Collision::pointInRect(
                    hex_world_pos.x, hex_world_pos.y, 
                    this->map->hex_grid_rect.x, this->map->hex_grid_rect.y, 
                    this->map->hex_grid_rect.w, this->map->hex_grid_rect.h
                )) { 
                    float lesser_height = HEX_SIDE_LENGTH>>1;
                    float cos_30_radius = HEX_SIDE_LENGTH * 0.8660254f;
                    float x_gap = HEX_SIDE_LENGTH - cos_30_radius;
                    float right_x = HEX_RECT_TILE_WIDTH - x_gap;
                    float greater_height = HEX_RECT_TILE_WIDTH - lesser_height;

                    float offset_x = hex_world_pos.x - HEX_SIDE_LENGTH;
                    float offset_y = hex_world_pos.y - HEX_SIDE_LENGTH;

                    std::vector<Vector2D> hex_hull = { 
                        {         right_x + offset_x,      greater_height + offset_y },
                        { HEX_SIDE_LENGTH + offset_x, HEX_RECT_TILE_WIDTH + offset_y },
                        {           x_gap + offset_x,      greater_height + offset_y },
                        {           x_gap + offset_x,       lesser_height + offset_y },
                        { HEX_SIDE_LENGTH + offset_x,                0.0f + offset_y },
                        {         right_x + offset_x,       lesser_height + offset_y }
                    };

                    SDL_FPoint draw_points[7];
                    SDL_FPoint p;
                    for(int i=0; i<6; ++i) {
                        Vector2D s_p = convertWorldToScreen(hex_hull[i]);
                        p.x = s_p.x; p.y = s_p.y;
                        draw_points[i] = p;
                    }
                    draw_points[6] = draw_points[0];                                

                    hex_pos = convertWorldToScreen(hex_world_pos);
                    hex_center.x = hex_pos.x - 2.0f;
                    hex_center.y = hex_pos.y - 2.0f;

                    bool can_draw = true;

                    for(auto& d : draw_points) {
                        if(!Collision::pointInRect(d.x, d.y, -100.0f, -100.0f, Game::SCREEN_WIDTH+200, Game::SCREEN_HEIGHT+200)) {
                            can_draw = false;
                        }
                    }
                    if(can_draw) {
                        TextureManager::DrawWireframe(draw_points, 7, hex_border_color);
                        TextureManager::DrawRect(&hex_center, COLORS_MAGENTA);
                    }                    
                }                            
            }
        }

        // map grid
        Vector2D pivot;
        SDL_FRect grid_line;
        pivot.y = 0.0f;
        grid_line.w = 1.0f;
        grid_line.h = this->map->world_layout_height * Game::camera_zoom;
        for(int i=0; i<=this->map->layout_width; ++i) {
            pivot.x = i*(this->map->tile_width);
            Vector2D pos = convertWorldToScreen(Vector2D(pivot.x, pivot.y));
            grid_line.x = pos.x;
            grid_line.y = pos.y;
            if(
                Game::SCREEN_WIDTH >= grid_line.x &&
                grid_line.x + grid_line.w >= 0.0f &&
                Game::SCREEN_HEIGHT >= grid_line.y &&
                grid_line.y + grid_line.h >= 0.0f
            ) {
                TextureManager::DrawRect(&grid_line, COLORS_CYAN);
            }
        }
        pivot.x = 0.0f;
        grid_line.w = this->map->world_layout_width * Game::camera_zoom;
        grid_line.h = 1.0f;
        for(int i=0; i<=this->map->layout_height; ++i) {
            pivot.y = i*(this->map->tile_width);
            Vector2D pos = convertWorldToScreen(Vector2D(pivot.x, pivot.y));
            grid_line.x = pos.x;
            grid_line.y = pos.y;
            if(
                Game::SCREEN_WIDTH >= grid_line.x &&
                grid_line.x + grid_line.w >= 0.0f &&
                Game::SCREEN_HEIGHT >= grid_line.y &&
                grid_line.y + grid_line.h >= 0.0f
            ) {
                TextureManager::DrawRect(&grid_line, COLORS_CYAN);
            }
        }
    }                  

    // debugging collision meshes
    int debug_mesh_height = -1;
    int debug_mesh_width = -1;
    std::vector<std::vector<uint8_t>>* debug_collision_mesh = nullptr;
    int mesh_density;
    switch(this->toggle_collision_mesh_crosshair) {
        case 0: 
            mesh_density = 1;
            debug_collision_mesh = &Game::collision_mesh_1;
            debug_mesh_height = Game::collision_mesh_1_height;
            debug_mesh_width  = Game::collision_mesh_1_width;
            break;
        case 1:
            mesh_density = 4;
            debug_collision_mesh = &Game::collision_mesh_4;
            debug_mesh_height = Game::collision_mesh_4_height;
            debug_mesh_width  = Game::collision_mesh_4_width;
            break;
        case 2:
            mesh_density = 16;
            debug_collision_mesh = &Game::collision_mesh_16;
            debug_mesh_height = Game::collision_mesh_16_height;
            debug_mesh_width  = Game::collision_mesh_16_width;
            break;
        case 3:
            mesh_density = 64;
            debug_collision_mesh = &Game::collision_mesh_64;
            debug_mesh_height = Game::collision_mesh_64_height;
            debug_mesh_width  = Game::collision_mesh_64_width;
            break;
        case 4:
            mesh_density = 4;
            debug_collision_mesh = &Game::collision_mesh_macro_4;
            debug_mesh_height = Game::collision_mesh_macro_4_height;
            debug_mesh_width  = Game::collision_mesh_macro_4_width;
            break;
    }
    if(debug_collision_mesh != nullptr) {
        for(int y=0; y<debug_mesh_height; ++y) {
            for(int x=0; x<debug_mesh_width; ++x) {
                Vector2D p_center;
                switch(this->toggle_collision_mesh_crosshair) {
                    case 0: 
                    case 1:
                    case 2:
                    case 3:
                        p_center = convertWorldToScreen( convertMeshNodeToVector2D({x, y}, mesh_density) ); break;
                    case 4:
                        p_center = convertWorldToScreen( convertMacroMeshNodeToVector2D({x, y}, mesh_density) ); break;
                }
                if(!Collision::pointInRect(p_center.x, p_center.y, 0.0f, 0.0f, Game::SCREEN_WIDTH, Game::SCREEN_HEIGHT)) {
                    continue;
                }
                Vector2D line_horizontal[2] = { Vector2D(p_center.x-1, p_center.y  ), Vector2D(p_center.x+1, p_center.y  ) };
                Vector2D line_vertical[2]   = { Vector2D(p_center.x,   p_center.y-1), Vector2D(p_center.x,   p_center.y+1) };
                if(
                    (*debug_collision_mesh)[y][x] != TILE_IMPASSABLE && 
                    (*debug_collision_mesh)[y][x] != TILE_NAVIGABLE && 
                    (*debug_collision_mesh)[y][x] != TILE_BASE_SPAWN &&
                    (*debug_collision_mesh)[y][x] != TILE_PLAYER
                ) {
                    TextureManager::DrawLine(line_horizontal[0], line_horizontal[1], COLORS_GREEN);
                    TextureManager::DrawLine(  line_vertical[0],   line_vertical[1], COLORS_GREEN);
                } else {
                    TextureManager::DrawLine(line_horizontal[0], line_horizontal[1], COLORS_MAGENTA);
                    TextureManager::DrawLine(  line_vertical[0],   line_vertical[1], COLORS_MAGENTA);
                }         
            }
        }
    }

    // crosshair
    float line_length = 10;
    Vector2D screen_center = Vector2D(Game::SCREEN_WIDTH>>1, Game::SCREEN_HEIGHT>>1);
    Vector2D line_left[2]   = { Vector2D(screen_center.x-1, screen_center.y), Vector2D(screen_center.x-1-line_length, screen_center.y) };
    Vector2D line_right[2]  = { Vector2D(screen_center.x+1, screen_center.y), Vector2D(screen_center.x+1+line_length, screen_center.y) };
    Vector2D line_top[2]    = { Vector2D(screen_center.x, screen_center.y-1), Vector2D(screen_center.x, screen_center.y-1-line_length) };
    Vector2D line_bottom[2] = { Vector2D(screen_center.x, screen_center.y+1), Vector2D(screen_center.x, screen_center.y+1+line_length) };
    TextureManager::DrawLine(  line_left[0],   line_left[1], Game::default_text_color);
    TextureManager::DrawLine( line_right[0],  line_right[1], Game::default_text_color);
    TextureManager::DrawLine(   line_top[0],    line_top[1], Game::default_text_color);
    TextureManager::DrawLine(line_bottom[0], line_bottom[1], Game::default_text_color);
}
void clean() {
    if(this->is_server) { destroyServer(); }
    if(this->is_client) { destroyClient(); }
    this->map = nullptr;
    this->PLAYER_COLOR = MainColors::NONE;
    this->clients_ping = {};
    this->clients_color = {};
    this->moved_drones = {};
    this->previous_drones_positions = {};
    this->PING_MS = 0;
    this->PLAYER_CLIENT_ID = -1;
    this->update_server = false;
    Game::manager->clearEntities();
}
};