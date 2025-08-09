#pragma once

#include <vector>
#include "Game.hpp"
#include "Vector2D.hpp"
#include "utils.hpp"
#include "Map.hpp"
#include "TextureManager.hpp"
#include "AudioManager.hpp"

#include "ECS/ECS.hpp"
#include "Colors.hpp"
#include "ECS/Components.hpp"
#include "ECS/Colliders/Collider.hpp"
#include "ECS/Colliders/Collision.hpp"

#include "path_finding.hpp"
#include "HexagonGrid.hpp"
#include "Camera.hpp"
#include "GroupLabels.hpp"
#include "SceneTypes.hpp"

#include "networking/MessageTypes.h"
#include "networking/Client.hpp"
#include "networking/Server.hpp"

#include "Scene_utils.hpp"
#include "SceneLobby.hpp"
#include "SceneMainMenu.hpp"
#include "SceneSettings.hpp"
#include "SceneMatchGame.hpp"

// Scene is a helper class to initialize Managers and globals
class Scene {
    private:
        SceneType st;
        SDL_Event event;
        bool load_textures = true;

        // ------------------ TEXTURES ------------------
        SDL_Texture* plain_terrain_texture;
        SDL_Texture* rough_terrain_texture;
        SDL_Texture* mountain_texture;
        SDL_Texture* water_bg_texture;
        SDL_Texture* water_fg_texture;

        // ------------------ AUDIO ------------------
        Mix_Music* music_main_menu = NULL;
        Mix_Chunk* sound_button = NULL;

        // ------------------ SCENES ------------------
        SceneLobby* S_Lobby;
        SceneMainMenu* S_MainMenu;
        SceneSettings* S_Settings;
        SceneMatchGame* S_MatchGame;

    public:
        // frame counter
        TextComponent* fps_text;

        Scene() {
            this->S_Lobby = new SceneLobby(&this->event);
            this->S_MainMenu = new SceneMainMenu(&this->event);
            this->S_Settings = new SceneSettings(&this->event);
            this->S_MatchGame = new SceneMatchGame(&this->event);
        }
        ~Scene() {
            Mix_HaltMusic();
            Mix_FreeMusic(this->music_main_menu);
            Mix_FreeChunk(this->sound_button);

            SDL_DestroyTexture(this->plain_terrain_texture);
            SDL_DestroyTexture(this->plain_terrain_texture);
            SDL_DestroyTexture(this->mountain_texture);
            SDL_DestroyTexture(this->water_bg_texture);
            SDL_DestroyTexture(this->water_fg_texture);
        }

        void loadTextures() {
            // white helps with color modulation
            Game::unit_tex     = TextureManager::LoadTexture("assets/white_circle.png");
            Game::building_tex = TextureManager::LoadTexture("assets/green_hexagon.png");

            this->plain_terrain_texture = TextureManager::LoadTexture("assets/tiles/plain.png");
            this->rough_terrain_texture = TextureManager::LoadTexture("assets/tiles/rough.png");
            this->mountain_texture      = TextureManager::LoadTexture("assets/tiles/mountain.png");
            this->water_bg_texture      = TextureManager::LoadTexture("assets/tiles/water_background.png");
            this->water_fg_texture      = TextureManager::LoadTexture("assets/tiles/water_foreground.png");

            this->load_textures = false;
        }

        void setScene(SceneType t) {
            // if this texture is null, all others are also null
            if(Game::unit_tex == nullptr || this->load_textures) { loadTextures(); }
            this->st = t;

            Entity& fps_ui = createUISimpleText("FPS_COUNTER", Game::SCREEN_WIDTH - 163, 3, "FPS:000.00");
            this->fps_text = &fps_ui.getComponent<TextComponent>();

            switch(t) {
                case SceneType::MAIN_MENU: { 
                    this->S_MainMenu->setScene(
                        this->plain_terrain_texture, 
                        this->music_main_menu,
                        this->sound_button,
                        this->fps_text
                    ); 
                } break;

                case SceneType::LOBBY: { 
                    this->S_Lobby->setScene(
                        this->sound_button, 
                        this->fps_text
                    ); 
                } break;

                case SceneType::SETTINGS: { 
                    this->S_Settings->setScene(
                        this->sound_button, 
                        this->fps_text
                    ); 
                } break;

                case SceneType::MATCH_GAME: {
                    this->S_MatchGame->setScene(
                        this->plain_terrain_texture, 
                        this->rough_terrain_texture,
                        this->mountain_texture,
                        this->water_bg_texture,
                        this->water_fg_texture,
                        this->fps_text
                    );
                } break;
            }
        }




        void handleEventsPrePoll() {
            switch(this->st) {
                case SceneType::MAIN_MENU: { this->S_MainMenu->handleEventsPrePoll(); } break;
                case SceneType::LOBBY: { this->S_Lobby->handleEventsPrePoll(); } break;
                case SceneType::SETTINGS: { this->S_Settings->handleEventsPrePoll(); } break;
                case SceneType::MATCH_GAME: { this->S_MatchGame->handleEventsPrePoll(); } break;
            }
        }

        void handleEventsPollEvent() {
            switch(this->st) {
                case SceneType::MAIN_MENU: {
                    this->S_MainMenu->handleEventsPollEvent();
                    if(this->S_MainMenu->change_to_scene != SceneType::NONE) {                        
                        setScene(this->S_MainMenu->change_to_scene);
                        this->S_MainMenu->change_to_scene = SceneType::NONE;
                    }
                } break;

                case SceneType::LOBBY: {
                    this->S_Lobby->handleEventsPollEvent();
                    if(this->S_Lobby->change_to_scene != SceneType::NONE) {                        
                        setScene(this->S_Lobby->change_to_scene);
                        this->S_Lobby->change_to_scene = SceneType::NONE;
                    }
                } break;

                case SceneType::SETTINGS: {
                    this->S_Settings->handleEventsPollEvent();
                    if(this->S_Settings->change_to_scene != SceneType::NONE) {                        
                        this->load_textures = true;
                        setScene(this->S_Settings->change_to_scene);
                        this->S_Settings->change_to_scene = SceneType::NONE;
                    }
                } break;

                case SceneType::MATCH_GAME: {
                    this->S_MatchGame->handleEventsPollEvent();
                } break;
            }
        }

        void handleEventsPostPoll() {
            const uint8_t *keystates = SDL_GetKeyboardState(NULL);
            switch(this->st) {
                case SceneType::MAIN_MENU: { this->S_MainMenu->handleEventsPostPoll(keystates); } break;
                case SceneType::LOBBY: { this->S_Lobby->handleEventsPostPoll(); } break;
                case SceneType::SETTINGS: { this->S_Settings->handleEventsPostPoll(); } break;
                case SceneType::MATCH_GAME: { this->S_MatchGame->handleEventsPostPoll(keystates); } break;
            }
        }

        void update() {
            switch(this->st) {
                case SceneType::MAIN_MENU: { this->S_MainMenu->update(); } break;
                case SceneType::LOBBY: { this->S_Lobby->update(); } break;
                case SceneType::SETTINGS: { this->S_Settings->update(); } break;
                case SceneType::MATCH_GAME: { this->S_MatchGame->update(); } break;
            }
        }

        void render() {
            this->fps_text->setText("FPS:" + format_decimal(Game::AVERAGE_FPS, 3, 2, false));
            switch(this->st) {
                case SceneType::MAIN_MENU: { this->S_MainMenu->render(); } break;
                case SceneType::LOBBY: { this->S_Lobby->render(); } break;
                case SceneType::SETTINGS: { this->S_Settings->render(); } break;
                case SceneType::MATCH_GAME: { this->S_MatchGame->render(); } break;
            }
        }

        void clean() {
            switch(this->st) {
                case SceneType::MAIN_MENU: { this->S_MainMenu->clean(); } break;
                case SceneType::LOBBY: { this->S_Lobby->clean(); } break;
                case SceneType::SETTINGS: { this->S_Settings->clean(); } break;
                case SceneType::MATCH_GAME: { this->S_MatchGame->clean(); } break;
            }
        }
};