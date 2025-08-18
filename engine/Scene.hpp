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
#include "SceneMapSelection.hpp"
#include "SceneMainMenu.hpp"
#include "SceneSettings.hpp"
#include "SceneMatchGame.hpp"
#include "SceneMatchSettings.hpp"

// Scene is a helper class to initialize Managers and globals
class Scene {
private:
SceneType st = SceneType::MAIN_MENU;
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
SceneMapSelection  *S_MapSelection;
SceneMainMenu      *S_MainMenu;
SceneSettings      *S_Settings;
SceneMatchGame     *S_MatchGame;
SceneMatchSettings *S_MatchSettings;

public:
// frame counter
TextComponent* fps_text;

Scene() {
    this->S_MapSelection  = new SceneMapSelection(&this->event);
    this->S_MainMenu      = new SceneMainMenu(&this->event);
    this->S_Settings      = new SceneSettings(&this->event);
    this->S_MatchGame     = new SceneMatchGame(&this->event);
    this->S_MatchSettings = new SceneMatchSettings(&this->event);
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

    delete  this->S_MapSelection;  this->S_MapSelection = nullptr;
    delete      this->S_MainMenu;      this->S_MainMenu = nullptr;
    delete      this->S_Settings;      this->S_Settings = nullptr;
    delete     this->S_MatchGame;     this->S_MatchGame = nullptr;
    delete this->S_MatchSettings; this->S_MatchSettings = nullptr;
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

    Entity& fps_ui = createUISimpleText("FPS_COUNTER", Game::SCREEN_WIDTH - 163, 3, "FPS:000.00", Game::default_text_color, groupPriorityUI);
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

        case SceneType::MAP_SELECTION: { 
            this->S_MapSelection->setScene(
                this->sound_button, 
                this->fps_text,
                SceneType::MAIN_MENU
            ); 
        } break;

        case SceneType::MATCH_SETTINGS: {
            this->S_MatchSettings->setScene(
                this->sound_button,
                this->fps_text,
                SceneType::MAP_SELECTION,
                this->S_MapSelection->selected_map_name
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
        case SceneType::MAP_SELECTION: { this->S_MapSelection->handleEventsPrePoll(); } break;
        case SceneType::SETTINGS: { this->S_Settings->handleEventsPrePoll(); } break;
        case SceneType::MATCH_GAME: { this->S_MatchGame->handleEventsPrePoll(); } break;
        case SceneType::MATCH_SETTINGS: { this->S_MatchSettings->handleEventsPrePoll(); } break;
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

        case SceneType::MAP_SELECTION: {
            this->S_MapSelection->handleEventsPollEvent();
            if(this->S_MapSelection->change_to_scene != SceneType::NONE) {                        
                setScene(this->S_MapSelection->change_to_scene);
                this->S_MapSelection->change_to_scene = SceneType::NONE;
            }
        } break;

        case SceneType::MATCH_SETTINGS: {
            this->S_MatchSettings->handleEventsPollEvent();
            if(this->S_MatchSettings->change_to_scene != SceneType::NONE) {                        
                setScene(this->S_MatchSettings->change_to_scene);
                this->S_MatchSettings->change_to_scene = SceneType::NONE;
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
        case SceneType::MAP_SELECTION: { this->S_MapSelection->handleEventsPostPoll(); } break;
        case SceneType::SETTINGS: { this->S_Settings->handleEventsPostPoll(); } break;
        case SceneType::MATCH_GAME: { this->S_MatchGame->handleEventsPostPoll(keystates); } break;
        case SceneType::MATCH_SETTINGS: { this->S_MatchSettings->handleEventsPostPoll(); } break;
    }
}

void update() {
    switch(this->st) {
        case SceneType::MAIN_MENU: { this->S_MainMenu->update(); } break;
        case SceneType::MAP_SELECTION: { this->S_MapSelection->update(); } break;
        case SceneType::SETTINGS: { this->S_Settings->update(); } break;
        case SceneType::MATCH_GAME: { this->S_MatchGame->update(); } break;
        case SceneType::MATCH_SETTINGS: { this->S_MatchSettings->update(); } break;
    }
}

void render() {
    this->fps_text->setText("FPS:" + format_decimal(Game::AVERAGE_FPS, 3, 2, false));
    switch(this->st) {
        case SceneType::MAIN_MENU: { this->S_MainMenu->render(); } break;
        case SceneType::MAP_SELECTION: { this->S_MapSelection->render(); } break;
        case SceneType::SETTINGS: { this->S_Settings->render(); } break;
        case SceneType::MATCH_GAME: { this->S_MatchGame->render(); } break;
        case SceneType::MATCH_SETTINGS: { this->S_MatchSettings->render(); } break;
    }
}

void clean() {
    switch(this->st) {
        case SceneType::MAIN_MENU: { this->S_MainMenu->clean(); } break;
        case SceneType::MAP_SELECTION: { this->S_MapSelection->clean(); } break;
        case SceneType::SETTINGS: { this->S_Settings->clean(); } break;
        case SceneType::MATCH_GAME: { this->S_MatchGame->clean(); } break;
        case SceneType::MATCH_SETTINGS: { this->S_MatchSettings->clean(); } break;
    }
}
};