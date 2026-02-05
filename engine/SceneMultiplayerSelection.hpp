#pragma once

#include "json.hpp"
#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"
#include "utils.hpp"

class SceneMultiplayerSelection {
private:
SceneType parent_scene;

std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
SDL_Event* event;
Entity* table_users_column = nullptr;
Entity* table_ips_column = nullptr;

std::ifstream config_file;
nlohmann::json config_json;

void goBack() {
    Mix_PlayChannel(-1, this->sound_button, 0);
    this->change_to_scene = this->parent_scene;
}

void setUsersIpTable() {
    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };

    std::vector<std::string> users = {};
    std::vector<std::string> ips = {};
    int max_size_user = 0;
    for(auto const& [user, ip] : Game::USERS_IP) {
        users.push_back(user);
        ips.push_back(ip);
        if(user.size() > max_size_user) { max_size_user = user.size(); }
    }

    for(std::string& user : users) {
        int to_pad = max_size_user - user.size();
        std::string padding = "";
        for(int i=0; i<to_pad; ++i) { padding += ' '; }
        user = padding + user;
        std::cout << "user:"<<user << '\n';
    }
    for(std::string& ip : ips) {
        std::cout << "ip:" << ip << '\n';
    }

    if(this->table_users_column != nullptr) {
        this->table_users_column->destroy();
        this->table_ips_column->destroy();
        this->table_users_column = nullptr;
        this->table_ips_column = nullptr;
    }

    this->table_users_column = &createUIMultilineText(
        "users_column", users,
        50, 50,
        Game::default_text_color, background_color, border_color
    );
    TextBoxComponent& left_column = this->table_users_column->getComponent<TextBoxComponent>();
    this->table_ips_column = &createUIMultilineText(
        "ips_column", ips,
        left_column.x + left_column.w - left_column.border_thickness, left_column.y,
        Game::default_text_color, background_color, border_color
    );
}

public:
Mix_Chunk* sound_button = NULL;
TextComponent* fps_text;
SceneType change_to_scene = SceneType::NONE;

SceneMultiplayerSelection(SDL_Event* e) { this->event = e; }
~SceneMultiplayerSelection() {}

void setScene(Mix_Chunk*& sound_b, TextComponent* fps, SceneType parent) {
    this->sound_button = sound_b;
    this->fps_text = fps;
    this->parent_scene = parent;
    
    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };

    setUsersIpTable();
    
    createUIButton(
        "button_back", 
        "Back", 
        50, -50, 
        Game::default_text_color, background_color, border_color,
        [this](TextBoxComponent& self) {
            this->goBack();
        }
    );
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

void handleMouseRelease(SDL_MouseButtonEvent& b) {
    Vector2D pos = Vector2D(b.x, b.y);
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            if(!clickedButton(pos)) {

            }
        } break;
    }
}

void handleMouseWheel(SDL_MouseWheelEvent& e) {
    int wheel_y = e.y;
    int scroll_offset = 0;
    if(wheel_y > 0) { // scroll up
        scroll_offset = 64;              
    } else if(wheel_y < 0) { // scroll down
        scroll_offset = -64;
    }
    if(scroll_offset != 0) {
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
            case SDL_MOUSEWHEEL: {
                handleMouseWheel(this->event->wheel);
            } break;
            case SDL_KEYUP: {
                if(this->event->key.keysym.sym == SDLK_ESCAPE) {
                    this->goBack();
                }
            } break;
        }                
    }
}
void handleEventsPostPoll(const uint8_t *keystates) {}
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