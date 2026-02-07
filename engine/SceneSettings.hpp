#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"
#include "json.hpp"
#include "TextFieldEditStyle.hpp"

class SceneSettings {
private:
SceneType parent_scene;
std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
SDL_Event* event;
Entity* fps_dropdown = nullptr;
std::vector<unsigned int> fps_values = { 
      1,  10,  20,  30,  40,  50,  60,  70,  80,  90, 
    100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 
    200, 210, 220, 230, 240, 250, 260, 270, 280, 290
};
std::vector<std::string> fps_labels = { 
    "  1 Hz", " 10 Hz", " 20 Hz", " 30 Hz", " 40 Hz", " 50 Hz", " 60 Hz", " 70 Hz", " 80 Hz", " 90 Hz", 
    "100 Hz", "110 Hz", "120 Hz", "130 Hz", "140 Hz", "150 Hz", "160 Hz", "170 Hz", "180 Hz", "190 Hz", 
    "200 Hz", "210 Hz", "220 Hz", "230 Hz", "240 Hz", "250 Hz", "260 Hz", "270 Hz", "280 Hz", "290 Hz"
};
Entity* resolution_dropdown = nullptr;
std::vector<std::pair<unsigned int, unsigned int>> resolution_values = {
    {  800,  600 },
    { 1280,  720 },
    { 1366,  768 },
    { 1536,  864 },
    { 1440,  900 },
    { 1920, 1080 },
    { 2560, 1440 },
    { 3840, 2160 }
};
std::vector<std::string> resolution_labels = {
    " 800 x  600",
    "1280 x  720",
    "1366 x  768",
    "1536 x  864",
    "1440 x  900",
    "1920 x 1080",
    "2560 x 1440",
    "3840 x 2160"
};


std::ifstream config_file;
nlohmann::json config_json;

public:
Mix_Chunk* sound_button = NULL;
TextComponent* fps_text;
SceneType change_to_scene = SceneType::NONE;

SceneSettings(SDL_Event* e) { this->event = e; }
~SceneSettings() {}

void setScene(Mix_Chunk*& sound_b, TextComponent* fps, SceneType parent) {
    this->sound_button = sound_b;
    this->fps_text = fps;
    this->parent_scene = parent;

    if(this->config_file.is_open()) { this->config_file.close(); }
    this->config_file.open("config.json");
    if(!this->config_file.is_open()) { std::cerr << "Error: could not open config.json\n"; }
    this->config_json = nlohmann::json::parse(config_file);

    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };

    this->resolution_dropdown = &createUIDropdown(
        "resolution_dropdown", resolution_labels, this->sound_button,
        50, 50, 
        Game::default_text_color, background_color, border_color
    );
    TextDropdownComponent& res_dropdown = this->resolution_dropdown->getComponent<TextDropdownComponent>();
    createUIButton(
        "button_apply_resolution",
        "Apply Resolution",
        res_dropdown.borderRect.x + res_dropdown.borderRect.w + 30, res_dropdown.y,
        Game::default_text_color, background_color, border_color,
        [this](TextBoxComponent& self) {
            std::string resolution_option = this->resolution_dropdown->getComponent<TextDropdownComponent>().selected_option_label;
            for(int i=0; i<this->resolution_labels.size(); ++i) {
                if(this->resolution_labels[i] == resolution_option) {
                    this->changeScreenResolution(this->resolution_values[i].first, this->resolution_values[i].second);
                    this->change_to_scene = SceneType::SETTINGS;
                    break;
                }
            }
        }
    );

    this->fps_dropdown = &createUIDropdown(
        "fps_dropdown", fps_labels, this->sound_button,
        res_dropdown.borderRect.x, res_dropdown.borderRect.y + res_dropdown.borderRect.h + 20, 
        Game::default_text_color, background_color, border_color
    );
    TextDropdownComponent& fps_dropdown_comp = this->fps_dropdown->getComponent<TextDropdownComponent>();
    createUIButton(
        "button_apply_fps",
        "Apply FPS",
        fps_dropdown_comp.borderRect.x + fps_dropdown_comp.borderRect.w + 30, fps_dropdown_comp.y,
        Game::default_text_color, background_color, border_color,
        [this](TextBoxComponent& self) {
            std::string fps_option = this->fps_dropdown->getComponent<TextDropdownComponent>().selected_option_label;
            for(int i=0; i<this->fps_labels.size(); ++i) {
                if(this->fps_labels[i] == fps_option) {
                    changeFPS(this->fps_values[i]);
                    this->config_json["FRAME_RATE"] = this->fps_values[i];
                    std::ofstream o("config.json");
                    o << this->config_json.dump(4);
                    o.close();
                    break;
                }
            }
        }
    );

    createUITextField(
        "button_keyboard",
        "testing_keyboard",
        fps_dropdown_comp.borderRect.x, fps_dropdown_comp.borderRect.y + fps_dropdown_comp.borderRect.h + 20,
        TextFieldEditStyle::IP,
        16,
        Game::default_text_color, Game::default_bg_color, border_color,
        [this](TextBoxComponent& self) {
            Mix_PlayChannel(-1, this->sound_button, 0);
            self.editing = true;
            self.cursor_pos = self.text_content[0].size();
        }
    );

    createUIButton(
        "button_back", 
        "Back", 
        50,  -50, 
        Game::default_text_color, background_color, border_color,
        [this](TextBoxComponent& self) {
            Mix_PlayChannel(-1, this->sound_button, 0);
            this->change_to_scene = this->parent_scene;
        }
    );

    // set labels according to current loaded values
    for(int i=0; i<resolution_values.size(); ++i) {
        if(
            resolution_values[i].first == Game::SCREEN_WIDTH &&
            resolution_values[i].second == Game::SCREEN_HEIGHT
        ) {
            res_dropdown.setSelectedOption(i);
            break;
        }
    }
    for(int i=0; i<fps_values.size(); ++i) {
        if(fps_values[i] == Game::MAX_FPS) {
            fps_dropdown_comp.setSelectedOption(i);
            break;
        }
    }
}

void changeScreenResolution(unsigned int width, unsigned int height) {
    this->config_json["SCREEN_WIDTH"] = width;
    this->config_json["SCREEN_HEIGHT"] = height;
    std::ofstream o("config.json");
    o << this->config_json.dump(4);
    o.close();
    Mix_PlayChannel(-1, this->sound_button, 0);
    // soft destroy
    SDL_DestroyTexture(Game::unit_tex);
    SDL_DestroyTexture(Game::building_tex);
    Game::unit_tex = nullptr;
    Game::building_tex = nullptr;

    SDL_DestroyWindow(Game::window);
    SDL_DestroyRenderer(Game::renderer);

    const uint32_t flags = SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_GRABBED;
    Game::window = SDL_CreateWindow("Bétula Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);
    Game::renderer = SDL_CreateRenderer(Game::window, -1, SDL_RENDERER_ACCELERATED);
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
            for(auto& ui : this->ui_elements) {
                if(ui->hasComponent<TextBoxComponent>()) {
                    if(ui->getComponent<TextBoxComponent>().onMousePress(pos)) {
                        break;
                    }
                }
            }
        } break;
    }
}

bool clickedButton(Vector2D& pos) {
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<TextBoxComponent>()) {
            if(ui->getComponent<TextBoxComponent>().onMouseRelease(pos)) {
                return true;
            }
        }
    }
    return false;
}
void handleKeyDown(SDL_Keycode key) {
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<TextBoxComponent>()) {
            if(ui->getComponent<TextBoxComponent>().editing) {
                ui->getComponent<TextBoxComponent>().handleKeyDown(key);
                return;
            }
        }
    }
}
void handleKeyUp() {
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<TextBoxComponent>()) {
            if(ui->getComponent<TextBoxComponent>().editing) {
                ui->getComponent<TextBoxComponent>().handleKeyUp();
                return;
            }
        }
    }
}

bool clickedDropdown(Vector2D& pos) {
    for(auto& pr_ui : this->pr_ui_elements) {
        if(pr_ui->hasComponent<TextDropdownComponent>()) {
            if(pr_ui->getComponent<TextDropdownComponent>().onMouseRelease(pos)) {
                return true;
            }
        }
    }
    return false;
}

void handleMouseRelease(SDL_MouseButtonEvent& b) {
    Vector2D pos = Vector2D(b.x, b.y);
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            if(!clickedDropdown(pos)) {
                clickedButton(pos);
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
            case SDL_KEYUP: {
                handleKeyUp();
            } break;
            case SDL_KEYDOWN: {
                handleKeyDown(this->event->key.keysym.sym);
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
                    case SDL_WINDOWEVENT_ENTER: break;
                    case SDL_WINDOWEVENT_LEAVE: break;
                    case SDL_WINDOWEVENT_FOCUS_GAINED: break;
                    case SDL_WINDOWEVENT_FOCUS_LOST: break;
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
    
    std::vector<Entity*> opened_dropdowns = {};
    for(auto& pr_ui : this->pr_ui_elements) { 
        if(pr_ui->hasComponent<TextDropdownComponent>() && pr_ui->getComponent<TextDropdownComponent>().display_dropdown) {
            opened_dropdowns.push_back(pr_ui);
        } else {
            pr_ui->draw();
        }
    }
    for(Entity*& drop_down : opened_dropdowns) { drop_down->draw(); }
}
void clean() {
    Game::manager->clearEntities();
}
};