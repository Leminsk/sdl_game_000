#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"
#include "json.hpp"

class SceneSettings {
private:
std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
SDL_Event* event;
Entity* fps_dropdown = nullptr;
std::vector<unsigned int> fps_values = { 30, 40, 50, 60, 70, 80, 90, 100, 110, 120 };
std::vector<std::string> frame_rates = { " 30 Hz", " 40 Hz", " 50 Hz", " 60 Hz", " 70 Hz", " 80 Hz", " 90 Hz", "100 Hz", "110 Hz", "120 Hz" };
std::ifstream config_file;
nlohmann::json config_json;

public:
Mix_Chunk* sound_button = NULL;
TextComponent* fps_text;
SceneType change_to_scene = SceneType::NONE;

SceneSettings(SDL_Event* e) { this->event = e; }
~SceneSettings() {}

void setScene(Mix_Chunk*& sound_b, TextComponent* fps) {
    this->sound_button = sound_b;
    this->fps_text = fps;

    if(this->config_file.is_open()) { this->config_file.close(); }
    this->config_file.open("config.json");
    if(!this->config_file.is_open()) { std::cerr << "Error: could not open config.json\n"; }
    this->config_json = nlohmann::json::parse(config_file);

    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };
    
    // TODO: store these resolutions in some kind of table or hashmap
    createUIButton("button_res_SVGA",   " 800 x  600",  50,  50, Game::default_text_color, background_color, border_color);
    createUIButton("button_res_WXGA",   "1280 x  720",  50, 114, Game::default_text_color, background_color, border_color);
    createUIButton("button_res_1.56M3", "1440 x 1080",  50, 178, Game::default_text_color, background_color, border_color);
    createUIButton("button_res_FHD",    "1920 x 1080",  50, 242, Game::default_text_color, background_color, border_color);

    fps_dropdown = &createUIDropdown("fps_dropdown", frame_rates, 300, 50, Game::default_text_color, background_color, border_color);

    createUIButton("button_apply_fps", "Apply FPS", 450, 50, Game::default_text_color, background_color, border_color);

    createUIButton("button_back", "Back", 50,  -50, Game::default_text_color, background_color, border_color);
}

void changeScreenResolution(unsigned int width, unsigned int height) {
    Mix_PlayChannel(-1, this->sound_button, 0);
    SDL_DestroyWindow(Game::window);
    SDL_DestroyRenderer(Game::renderer);
    const uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_GRABBED;
    Game::window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    Game::renderer = SDL_CreateRenderer(Game::window, -1, 0);
    Game::SCREEN_WIDTH = width;
    Game::SCREEN_HEIGHT = height;
    Game::camera_focus = Vector2D(Game::SCREEN_WIDTH>>1, Game::SCREEN_HEIGHT>>1);
}

void changeFPS(unsigned int new_fps) {
    Mix_PlayChannel(-1, this->sound_button, 0);
    int previous_broadcast_rate = (1/static_cast<float>(Game::SERVER_STATE_SHARE_RATE)) * Game::MAX_FPS;
    Game::MAX_FPS = new_fps;
    Game::MAX_FRAME_DELAY = 1000.0f / new_fps;
    Game::SERVER_STATE_SHARE_RATE = new_fps / previous_broadcast_rate;
    Game::CLIENT_PING_RATE = new_fps * 3;
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
    }
}

bool clickedButton(Vector2D& pos) {
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<TextBoxComponent>()) {
            TextBoxComponent& text_box = ui->getComponent<TextBoxComponent>();
            if(Collision::pointInRect(pos.x, pos.y, text_box.x, text_box.y, text_box.w, text_box.h)) { 
                if(ui->getComponent<TextBoxComponent>().mouse_down) {
                    std::string button_id = ui->getIdentifier();
                    if(button_id == "button_apply_fps") {
                        std::string fps_option = this->fps_dropdown->getComponent<TextDropdownComponent>().selected_option_label;
                        for(int i=0; i<this->frame_rates.size(); ++i) {
                            if(this->frame_rates[i] == fps_option) {
                                changeFPS(this->fps_values[i]);
                                this->config_json["FRAME_RATE"] = this->fps_values[i];
                                std::ofstream o("config.json");
                                o << this->config_json.dump(4);
                                o.close();
                                break;
                            }
                        }

                    } else {
                        if(button_id == "button_back") {
                            Mix_PlayChannel(-1, this->sound_button, 0);
                            this->change_to_scene = SceneType::MAIN_MENU;
                        } else {
                            int new_width = -1;
                            int new_height = -1;
                            if(button_id == "button_res_SVGA") {        new_width =  800; new_height =  600; } 
                            else if(button_id == "button_res_WXGA") {   new_width = 1280; new_height =  720; } 
                            else if(button_id == "button_res_1.56M3") { new_width = 1440; new_height = 1080; } 
                            else if(button_id == "button_res_FHD") {    new_width = 1920; new_height = 1080; }

                            if(new_width > 0 && new_height > 0) {
                                this->config_json["SCREEN_WIDTH"] = new_width;
                                this->config_json["SCREEN_HEIGHT"] = new_height;
                                std::ofstream o("config.json");
                                o << this->config_json.dump(4);
                                o.close();
                                changeScreenResolution(new_width, new_height);
                                this->change_to_scene = SceneType::SETTINGS;
                            }                            
                        }
                        clean();
                    }
                    
                    return true;
                }                               
            }
            ui->getComponent<TextBoxComponent>().mouse_down = false;
        }
    }
    return false;
}

bool clickedDropdown(Vector2D& pos) {
    for(auto& pr_ui : this->pr_ui_elements) {
        if(pr_ui->hasComponent<TextDropdownComponent>()) {
            std::string dropdown_id = pr_ui->getIdentifier();
            TextDropdownComponent& dropdown = pr_ui->getComponent<TextDropdownComponent>();

            if(Collision::pointInRect(pos.x, pos.y, dropdown.x, dropdown.y, dropdown.w, dropdown.h)) {
                if(dropdown_id == "fps_dropdown") {
                    Mix_PlayChannel(-1, this->sound_button, 0);
                    dropdown.display_dropdown = !(dropdown.display_dropdown);
                    return true;
                }
            }

            if(dropdown.display_dropdown) {
                if(dropdown_id == "fps_dropdown") {
                    for(int i=0; i<dropdown.options.size(); ++i) {
                        TextBoxComponent* option = dropdown.options[i];
                        if(Collision::pointInRect(pos.x, pos.y, option->x, option->y, option->w, option->h)) {
                            Mix_PlayChannel(-1, this->sound_button, 0);
                            dropdown.setSelectedOption(i);
                            return true;
                        }
                    }
                }
                Mix_PlayChannel(-1, this->sound_button, 0);
                dropdown.display_dropdown = false;
            }   
        }
    }
    return false;
}

void handleMouseRelease(SDL_MouseButtonEvent& b) {
    Vector2D pos = Vector2D(b.x, b.y);
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            std::cout << "RELEASE LEFT: " << pos << '\n';
            if(!clickedButton(pos)) {
                clickedDropdown(pos);
            }
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
void handleEventsPostPoll() {}

void update() {
    Game::manager->refresh();
    Game::manager->preUpdate();
    Game::manager->update();
}
void render() {
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };
    const SDL_Color background_color = { 123,  82,  35, SDL_ALPHA_OPAQUE };
    const SDL_FRect borderRect     = { 0.0f, 0.0f, static_cast<float>(Game::SCREEN_WIDTH),   static_cast<float>(Game::SCREEN_HEIGHT)   };
    const SDL_FRect backgroundRect = { 3.0f, 3.0f, static_cast<float>(Game::SCREEN_WIDTH-6), static_cast<float>(Game::SCREEN_HEIGHT-6) };
    
    TextureManager::DrawRect(&borderRect, border_color);
    TextureManager::DrawRect(&backgroundRect, background_color);

    for(auto& bg_ui : this->bg_ui_elements) { bg_ui->draw(); }
    for(auto& ui : this->ui_elements) { ui->draw(); }
    for(auto& pr_ui : this->pr_ui_elements) { pr_ui->draw(); }
}
void clean() {
    Game::manager->clearEntities();
}
};