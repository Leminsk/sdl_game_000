#pragma once

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
#include "SceneTypes.hpp"

#include "networking/MessageTypes.h"
#include "networking/Client.hpp"
#include "networking/Server.hpp"

// Scene is a helper class to initialize Managers and globals
// I thought about making a separate class for each different Scene, but I think a fat class for all of them might save me from linking errors down the road
class Scene {
    private:
        SceneType st;
        SDL_Event event;

        // --------------------------------  TEXTURES  --------------------------------
        SDL_Texture* plain_terrain_texture = TextureManager::LoadTexture("assets/tiles/plain.png");
        SDL_Texture* rough_terrain_texture = TextureManager::LoadTexture("assets/tiles/rough.png");
        SDL_Texture* mountain_texture      = TextureManager::LoadTexture("assets/tiles/mountain.png");
        SDL_Texture* water_bg_texture      = TextureManager::LoadTexture("assets/tiles/water_background.png");
        SDL_Texture* water_fg_texture      = TextureManager::LoadTexture("assets/tiles/water_foreground.png");
        // -------------------------------- ---------- --------------------------------

        // -------------------------------- MATCH_GAME --------------------------------
        Map* map;
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

        std::vector<Entity*>&      buildings = Game::manager->getGroup(groupBuildings);
        std::vector<Entity*>&         drones = Game::manager->getGroup(groupDrones);
        std::vector<Entity*>&          tiles = Game::manager->getGroup(groupTiles);
        std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
        std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);

        // -------------------------------- ---------- --------------------------------

    public:
        // frame counter
        TextComponent* fps_text;

        Scene() {}
        ~Scene() {}


        void setScene(SceneType t) {
            this->st = t;

            Entity& fps_ui = createUISimpleText("FPS_COUNTER", Game::SCREEN_WIDTH - 270, 0, Game::SCREEN_WIDTH/3, Game::SCREEN_HEIGHT/16);
            this->fps_text = &fps_ui.getComponent<TextComponent>();

            switch(t) {
                case SceneType::MAIN_MENU: {
                    Game::default_bg_color = { 123, 82, 35, SDL_ALPHA_OPAQUE };

                    const int w = 64;
                    const int h = 64;
                    for(int i=0; i<Game::SCREEN_HEIGHT; i+=h) {
                        for(int j=0; j<Game::SCREEN_WIDTH; j+=w) {
                            createUIImage("menu_background-" + std::to_string(j) + ',' + std::to_string(i), this->plain_terrain_texture, j, i, w, h);
                        }
                    }                    
                    
                    SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
                    SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };
                    createUIButton("button_single_player",  50,   50, "Single-Player", Game::default_text_color, background_color, border_color);
                    createUIButton(  "button_multiplayer",  50,  200,   "Multiplayer", Game::default_text_color, background_color, border_color);
                    createUIButton(     "button_settings",  50, -200,      "Settings", Game::default_text_color, background_color, border_color);
                    createUIButton(         "button_quit",  50,  -50,          "Quit", Game::default_text_color, background_color, border_color);
                } break;

                case SceneType::LOBBY: {} break;
                case SceneType::SETTINGS: {} break;

                case SceneType::MATCH_GAME: {
                    Game::default_bg_color = { 50, 5, 10, SDL_ALPHA_OPAQUE };

                    this->map = new Map(
                        "assets/test3.bmp", 
                        this->plain_terrain_texture,
                        this->rough_terrain_texture,
                        this->mountain_texture,
                        this->water_bg_texture,
                        this->water_fg_texture,
                        Game::UNIT_SIZE<<1
                    );

                    if(this->map->loaded) {
                        LoadMapRender();
                        Game::world_map_layout_width = this->map->world_layout_width;
                        Game::world_map_layout_height = this->map->world_layout_height;

                        this->map->generateCollisionMesh( 1, Game::collision_mesh_1,  Game::collision_mesh_1_width,  Game::collision_mesh_1_height);
                        this->map->generateCollisionMesh( 4, Game::collision_mesh_4,  Game::collision_mesh_4_width,  Game::collision_mesh_4_height);
                        this->map->generateCollisionMesh(16, Game::collision_mesh_16, Game::collision_mesh_16_width, Game::collision_mesh_16_height);
                        this->map->generateCollisionMesh(64, Game::collision_mesh_64, Game::collision_mesh_64_width, Game::collision_mesh_64_height);

                        createDrone(  0,   0, MainColors::WHITE);
                        createDrone(100,   0, MainColors::BLACK);
                        createDrone(  0, 100, MainColors::RED);
                        createDrone(200, 200, MainColors::GREEN);

                        createUISimpleText("crosshair", 0, 0, Game::SCREEN_WIDTH/3, Game::SCREEN_HEIGHT/16);
                        createUISimpleText("camera_zoom", 0, 30, Game::SCREEN_WIDTH/5, Game::SCREEN_HEIGHT/16);

                    } else {
                        printf("Map failed to load.\n");
                    }
                    
                } break;
            }
        }





        Entity& createDrone(float pos_x, float pos_y, MainColors c) {
            auto& new_drone(Game::manager->addEntity("DRO" + left_pad_int(Game::UNIT_COUNTER, 5)));
            new_drone.addComponent<DroneComponent>(Vector2D(pos_x, pos_y), Game::UNIT_SIZE, Game::unit_tex, c);
            new_drone.addComponent<Wireframe>();
            new_drone.addComponent<TextComponent>("", 0, 0, 160.0f, 16.0f);
            new_drone.addGroup(groupDrones);
            return new_drone;
        }
        Entity& createUIImage(
            const std::string& id, SDL_Texture* image_texture,
            int pos_x=0, int pos_y=0, int width=32, int height=32
        ) {
            auto& new_ui_image(Game::manager->addEntity(id));
            int x = pos_x < 0 ? ((Game::SCREEN_WIDTH - width) + pos_x) : pos_x; 
            int y = pos_y < 0 ? ((Game::SCREEN_HEIGHT - height) + pos_y) : pos_y;
            new_ui_image.addComponent<TransformComponent>(pos_x, pos_y, width, height, 1);
            new_ui_image.addComponent<SpriteComponent>(image_texture);
            new_ui_image.addGroup(groupBackgroundUI);
            return new_ui_image;
        }
        Entity& createUISimpleText(
            const std::string& id, 
            int pos_x=0, int pos_y=0, int width=100, int height=10, 
            const std::string& text="SIMPLE_TEXT",
            const SDL_Color& text_color=Game::default_text_color            
        ) {
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
        /* 
            if pos_x < 0 -> offset from the right (analogous with pos_y from the bottom)
            1 char -> width 26
            1 line -> height 42 // these values look okay
            ideally height should have a value such that height - (2*border_thickness + 4) is a multiple of 16 (e.g. 42 - (2*3 + 4) == 2*16), 
            but this is not enforced in TextBoxComponent
            similarly for width the value should be such that width - (2*border_thickness + 4) is a multiple of 8 (e.g. 26 - (2*3 + 4) == 2*8) 
            also not enforced
        */ 
        Entity& createUIButton(
            const std::string& id, 
            int pos_x=0, int pos_y=0,
            const std::string& text="TEXT_BOX",
            const SDL_Color& text_color=Game::default_text_color, 
            const SDL_Color& bg_color=Game::default_bg_color, 
            const SDL_Color& border_color=Game::default_text_color
        ) {
            auto& new_text_box(Game::manager->addEntity(id));
            const int w = 26*text.size(); const int h = 42;
            int x = pos_x < 0 ? ((Game::SCREEN_WIDTH - w) + pos_x) : pos_x; 
            int y = pos_y < 0 ? ((Game::SCREEN_HEIGHT - h) + pos_y) : pos_y;
            new_text_box.addComponent<TextBoxComponent>(
                text, 
                x, y, w, h,
                text_color, bg_color, border_color,
                3 // fixed border thickness for now
            );
            new_text_box.addGroup(groupUI);
            return new_text_box;
        }
        Entity& createBaseBuilding(std::string id, float world_pos_x, float world_pos_y, float width) {
            auto& building(Game::manager->addEntity(id));
            building.addComponent<TransformComponent>(world_pos_x, world_pos_y, width, width, 1.0);
            building.addComponent<SpriteComponent>(Game::building_tex);
            building.addComponent<Collider>(ColliderType::HEXAGON);
            building.addComponent<Wireframe>();
            building.addGroup(groupBuildings);
            return building;
        }
        void SetSolidTileNeighbors(uint8_t* neighbors, int map_x, int map_y, const std::vector<std::vector<int>>& layout) {
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
        Entity& AddTileOnMap(int id, float width, int map_x, int map_y, const std::vector<std::vector<int>>& layout) {
            auto& tile(Game::manager->addEntity("tile-"+std::to_string(map_x)+','+std::to_string(map_y)));
            
            switch(id) {
                case TILE_ROUGH:  {
                    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, this->rough_terrain_texture);
                } break;
                case TILE_IMPASSABLE: { 
                    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, this->mountain_texture);
                    uint8_t* neighbors = &tile.getComponent<RectangleCollider>().adjacent_rectangles;
                    SetSolidTileNeighbors(neighbors, map_x, map_y, layout);
                } break;
                case TILE_NAVIGABLE: { 
                    // need to be in this order to render the foreground "above" the background
                    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, this->water_bg_texture);
                    tile.addComponent<TileFGComponent>(map_x*width, map_y*width, width, width, id, this->water_fg_texture);
                }
                break;
                case TILE_BASE_SPAWN: {
                    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, this->plain_terrain_texture);
                    createBaseBuilding("base", map_x*width, map_y*width, width);
                } break;
                default: 
                    tile.addComponent<TileComponent>(map_x*width, map_y*width, width, width, id, this->plain_terrain_texture);
            }
            
            tile.addGroup(groupTiles);
            return tile;
        }
        void LoadMapRender(float tile_scale=1.0f) {
            uint64_t row, column;
            SDL_Texture** tex = nullptr;
            for(row = 0; row < this->map->layout_height; ++row) {
                for(column = 0; column < this->map->layout_width; ++column) {
                    AddTileOnMap(
                        this->map->layout[row][column], 
                        this->map->tile_width * tile_scale,
                        column, row,
                        this->map->layout
                    );
                }
            }
        }
        




        void handleMouseMainMenu(SDL_MouseButtonEvent& b) {
            Vector2D pos = Vector2D(b.x, b.y);
            switch(b.button) {
                case SDL_BUTTON_LEFT: {
                    std::cout << "MOUSE BUTTON LEFT: " << pos << '\n';
                    for(auto& ui : this->ui_elements) {
                        if(ui->hasComponent<TextBoxComponent>()) {
                            TextBoxComponent& text_box = ui->getComponent<TextBoxComponent>();
                            if(Collision::pointInRect(pos.x, pos.y, text_box.x, text_box.y, text_box.w, text_box.h)) {
                                ui->getComponent<TextBoxComponent>().mouse_down = true;
                            }
                        }
                    }
                } break;
                    
                case SDL_BUTTON_MIDDLE: std::cout << "MOUSE BUTTON MIDDLE\n"; break;

                case SDL_BUTTON_RIGHT: {
                    std::cout << "MOUSE BUTTON RIGHT: " << pos << '\n';
                } break;
            }
        }
        void handleMouseMainMenuRelease(SDL_MouseButtonEvent& b) {
            Vector2D pos = Vector2D(b.x, b.y);
            switch(b.button) {
                case SDL_BUTTON_LEFT: {
                    std::cout << "RELEASE LEFT: " << pos << '\n';
                    for(auto& ui : this->ui_elements) {
                        if(ui->hasComponent<TextBoxComponent>()) {
                            TextBoxComponent& text_box = ui->getComponent<TextBoxComponent>();
                            if(Collision::pointInRect(pos.x, pos.y, text_box.x, text_box.y, text_box.w, text_box.h)) { 
                                if(ui->getComponent<TextBoxComponent>().mouse_down) {
                                    std::string button_id = ui->getIdentifier();
                                    if(button_id == "button_single_player") {
                                        printf("single-player\n");
                                    } else if(button_id == "button_multiplayer") {
                                        clean();
                                        setScene(SceneType::MATCH_GAME);
                                    } else if(button_id == "button_settings") {
                                        printf("settings\n");
                                    } else if(button_id == "button_quit") {
                                        Game::isRunning = false;
                                    }
                                }                               
                            }
                            ui->getComponent<TextBoxComponent>().mouse_down = false;
                        }
                    }
                } break;
                    
                case SDL_BUTTON_MIDDLE: std::cout << "RELEASE MIDDLE\n"; break;

                case SDL_BUTTON_RIGHT: {
                    std::cout << "RELEASE RIGHT: " << pos << '\n';
                } break;
            }
        }
        void handleMouseMatchGame(SDL_MouseButtonEvent& b) {
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
                    
                case SDL_BUTTON_MIDDLE: std::cout << "MOUSE BUTTON MIDDLE\n"; break;

                case SDL_BUTTON_RIGHT: {
                    std::cout << "MOUSE BUTTON RIGHT: " << world_pos << '\n';
                    DroneComponent* drone;
                    for(auto& dr : this->drones) {
                        drone = &dr->getComponent<DroneComponent>();
                        if(drone->selected) {
                            if(this->is_client) {
                                // store path to send to server, but do not move drone
                                drone->path = find_path(drone->getPosition(), world_pos);
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
            this->server->~Server();
            this->server = nullptr;
        }
        void destroyClient() {
            printf("Disconnecting client\n");
            this->is_client = false;
            this->PLAYER_CLIENT_ID = 0;
            this->PLAYER_COLOR = MainColors::NONE;
            this->client->WarnDisconnect();
            this->client->Disconnect();
            this->client->~Client();
            this->client = nullptr;
        }




        void handleEventsPrePoll() {
            switch(this->st) {
                case SceneType::MAIN_MENU: {} break;


                case SceneType::LOBBY: {} break;
                case SceneType::SETTINGS: {} break;


                case SceneType::MATCH_GAME: {
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
                } break;
            }
        }

        void handleEventsPollEvent() {
            switch(this->st) {
                case SceneType::MAIN_MENU: {
                    while( SDL_PollEvent(&this->event) ) {
                        switch(this->event.type) {
                            case SDL_QUIT: {
                                Game::isRunning = false;
                                return;
                            } break;
                            case SDL_MOUSEBUTTONDOWN: {
                                handleMouseMainMenu(this->event.button);
                            } break;
                            case SDL_MOUSEBUTTONUP: {
                                handleMouseMainMenuRelease(this->event.button);
                            } break;
                            case SDL_WINDOWEVENT: {
                                switch(this->event.window.event) {
                                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                                        std::cout << "Window Size Change\n";
                                        Game::SCREEN_WIDTH = this->event.window.data1;
                                        Game::SCREEN_HEIGHT = this->event.window.data2;
                                        Game::camera_focus.x = Game::SCREEN_WIDTH>>1;
                                        Game::camera_focus.y = Game::SCREEN_HEIGHT>>1;
                                        this->fps_text->setRenderPos(Game::SCREEN_WIDTH - 270, 0, this->fps_text->w, this->fps_text->h);
                                    } break;
                                    case SDL_WINDOWEVENT_ENTER: std::cout << "Mouse IN\n"; break;
                                    case SDL_WINDOWEVENT_LEAVE: std::cout << "Mouse OUT\n"; break;
                                    case SDL_WINDOWEVENT_FOCUS_GAINED: std::cout << "Keyboard IN\n"; break;
                                    case SDL_WINDOWEVENT_FOCUS_LOST: std::cout << "Keyboard OUT\n"; break;
                                }
                            } break;
                        }                
                    }
                } break;


                case SceneType::LOBBY: {} break;
                case SceneType::SETTINGS: {} break;

                case SceneType::MATCH_GAME: {
                    while( SDL_PollEvent(&this->event) ) {
                        if(this->event.type == SDL_QUIT) {
                            Game::isRunning = false;
                            return;
                        }

                        if(this->event.type == SDL_MOUSEBUTTONDOWN) {
                            handleMouseMatchGame(this->event.button);
                        }

                        if(this->event.type == SDL_MOUSEWHEEL) {
                            int wheel_y = this->event.wheel.y;
                            if(wheel_y > 0) { // zoom in
                                Game::camera_zoom = std::min(Game::camera_zoom + 0.5f, 5.0f);
                            } else if(wheel_y < 0) { // zoom out
                                Game::camera_zoom = std::max(Game::camera_zoom - 0.5f, 0.5f);
                            }
                        }

                        if(this->event.type == SDL_WINDOWEVENT) {
                            switch(this->event.window.event) {
                                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                                    std::cout << "Window Size Change\n";
                                    Game::SCREEN_WIDTH = this->event.window.data1;
                                    Game::SCREEN_HEIGHT = this->event.window.data2;
                                    Game::camera_focus.x = Game::SCREEN_WIDTH>>1;
                                    Game::camera_focus.y = Game::SCREEN_HEIGHT>>1;
                                    this->fps_text->setRenderPos(Game::SCREEN_WIDTH - 270, 0, this->fps_text->w, this->fps_text->h);
                                } break;
                                case SDL_WINDOWEVENT_ENTER: std::cout << "Mouse IN\n"; break;
                                case SDL_WINDOWEVENT_LEAVE: std::cout << "Mouse OUT\n"; break;
                                case SDL_WINDOWEVENT_FOCUS_GAINED: std::cout << "Keyboard IN\n"; break;
                                case SDL_WINDOWEVENT_FOCUS_LOST: std::cout << "Keyboard OUT\n"; break;
                            }
                        }
                        
                    }
                } break;
            }
        }

        void handleEventsPostPoll() {
            const uint8_t *keystates = SDL_GetKeyboardState(NULL);

            switch(this->st) {
                case SceneType::MAIN_MENU: {
                    if(keystates[SDL_SCANCODE_ESCAPE]) { 
                            Game::isRunning = false; 
                    } 
                } break;


                case SceneType::LOBBY: {} break;
                case SceneType::SETTINGS: {} break;

                case SceneType::MATCH_GAME: {
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

                } break;
            }
        }

        void update() {
            switch(this->st) {
                case SceneType::MAIN_MENU: {
                    Game::manager->refresh();
                    Game::manager->preUpdate();
                    Game::manager->update();
                } break;

                case SceneType::LOBBY: {} break;
                case SceneType::SETTINGS: {} break;

                case SceneType::MATCH_GAME: {
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
                } break;
            }
        }

        void render() {
            this->fps_text->setText("FPS:" + format_decimal(Game::AVERAGE_FPS, 3, 2, false));
            switch(this->st) {
                case SceneType::MAIN_MENU: {
                    for(auto& bg_ui : this->bg_ui_elements) { bg_ui->draw(); }
                    for(auto& ui : this->ui_elements) { ui->draw(); }
                } break;

                case SceneType::LOBBY: {} break;
                case SceneType::SETTINGS: {} break;

                case SceneType::MATCH_GAME: {
                    for(auto& t : this->tiles) { t->draw(); }
                    for(auto& b : this->buildings) { b->draw(); }
                    for(auto& dr : this->drones) { dr->draw(); }
                    for(auto& ui : this->ui_elements) { ui->draw(); }

                    // draw the path trajectory for debugging
                    if(this->path_to_draw.size() > 0) {
                        int limit = this->path_to_draw.size()-1;
                        for(int i=0; i<limit; ++i) {
                            TextureManager::DrawLine(
                                convertWorldToScreen(this->path_to_draw[i]), 
                                convertWorldToScreen(this->path_to_draw[i+1]), 
                                {0x00, 0x00, 0xFF, SDL_ALPHA_OPAQUE}
                            );
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
                } break;
            }
        }

        void clean() {
            switch(this->st) {
                case SceneType::MAIN_MENU: {} break;
                
                case SceneType::LOBBY: {} break;
                case SceneType::SETTINGS: {} break;

                case SceneType::MATCH_GAME: {
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
                } break;
            }
            Game::manager->clearEntities();
        }
};