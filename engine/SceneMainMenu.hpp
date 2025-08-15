#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"

class SceneMainMenu {
private:
std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
SDL_Event* event = nullptr;

public:
SDL_Texture* plain_terrain_texture = nullptr;
Mix_Music* music_main_menu = nullptr;
Mix_Chunk* sound_button = nullptr;
TextComponent* fps_text;

SceneType change_to_scene = SceneType::NONE;

SceneMainMenu(SDL_Event* e) { this->event = e; }
~SceneMainMenu() {}

void setScene(SDL_Texture* plain_terrain, Mix_Music*& music, Mix_Chunk*& sound, TextComponent* fps) {
    this->plain_terrain_texture = plain_terrain;
    this->music_main_menu = music;
    this->sound_button = sound;
    this->fps_text = fps;

    if(music_main_menu == nullptr) {
        Mix_HaltMusic();
        music = AudioManager::LoadMusic("assets/audio/music/f-zero-ending_theme_dsp_1.wav");
        sound = AudioManager::LoadSound("assets/audio/sfx/mario64-bowser_road_channel_9-noise.wav");
        this->music_main_menu = music;
        this->sound_button = sound;
        Mix_PlayMusic(this->music_main_menu, -1);
    }

    Game::default_bg_color = { 123, 82, 35, SDL_ALPHA_OPAQUE };

    const int w = 64;
    const int h = 64;
    for(int i=0; i<Game::SCREEN_HEIGHT; i+=h) {
        for(int j=0; j<Game::SCREEN_WIDTH; j+=w) {
            createUIImage("menu_background-" + std::to_string(j) + ',' + std::to_string(i), this->plain_terrain_texture, j, i, w, h);
        }
    }
    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };
    createUIButton("button_single_player", "Single-Player", 50,   50, Game::default_text_color, background_color, border_color);
    createUIButton(  "button_multiplayer",   "Multiplayer", 50,  114, Game::default_text_color, background_color, border_color);
    createUIButton(     "button_settings",      "Settings", 50,  178, Game::default_text_color, background_color, border_color);

    createUIButton(         "button_quit",          "Quit", 50,  -50, Game::default_text_color, background_color, border_color);
    createUIMultilineText("multiline", { "first line", "second line", " ", "fourth line" }, -10, 50, Game::default_text_color, background_color, border_color);
}

void handleMouse(SDL_MouseButtonEvent& b) {
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
        case SDL_BUTTON_RIGHT: std::cout << "MOUSE BUTTON RIGHT: " << pos << '\n'; break;
    }
}
void handleMouseRelease(SDL_MouseButtonEvent& b) {
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
                                Mix_PlayChannel(-1, this->sound_button, 0);
                                clean();
                                this->change_to_scene = SceneType::LOBBY;
                                break;
                            } else if(button_id == "button_multiplayer") {
                                Mix_PlayChannel(-1, this->sound_button, 0);
                                clean();
                                this->change_to_scene = SceneType::MATCH_GAME;
                                break;
                            } else if(button_id == "button_settings") {
                                Mix_PlayChannel(-1, this->sound_button, 0);
                                clean();
                                this->change_to_scene = SceneType::SETTINGS;
                                break;
                            } else if(button_id == "button_quit") {
                                Mix_PlayChannel(-1, this->sound_button, 0);
                                Game::isRunning = false;
                                break;
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




void handleEventsPrePoll() {}
void handleEventsPollEvent() {
    while( SDL_PollEvent(this->event) ) {
        switch(this->event->type) {
            case SDL_QUIT: {
                Game::isRunning = false;
                return;
            } break;
            case SDL_MOUSEBUTTONDOWN: {
                handleMouse(this->event->button);
            } break;
            case SDL_MOUSEBUTTONUP: {
                handleMouseRelease(this->event->button);
            } break;
            case SDL_WINDOWEVENT: {
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
            } break;
        }                
    }
}
void handleEventsPostPoll(const uint8_t* keystates) {
    if(keystates[SDL_SCANCODE_ESCAPE]) { 
        Game::isRunning = false; 
    } 
}

void update() {
    Game::manager->refresh();
    Game::manager->preUpdate();
    Game::manager->update();
}
void render() {
    for(auto& bg_ui : this->bg_ui_elements) { bg_ui->draw(); }
    for(auto& ui : this->ui_elements) { ui->draw(); }
    for(auto& pr_ui : this->pr_ui_elements) { pr_ui->draw(); }
}
void clean() {
    Game::manager->clearEntities();
}
};