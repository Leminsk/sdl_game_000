#pragma once

#include "json.hpp"
#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"
#include "utils.hpp"
#include "ModalContentType.hpp"
#include "GroupLabels.hpp"
#include "TextFieldEditStyle.hpp"

class SceneMultiplayerSelection {
private:
SceneType parent_scene;

std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
std::vector<Entity*>& modal_bg_ui_elements = Game::manager->getGroup(groupModalBackground);
std::vector<Entity*>& modal_fg_ui_elements = Game::manager->getGroup(groupModalForeground);
SDL_Event* event;
Entity* table_header_users = nullptr;
Entity* table_header_ips = nullptr;
Entity* table_column_users = nullptr;
Entity* table_column_ips = nullptr;

std::vector<Entity*> modal_entities = {};

std::ifstream config_file;
nlohmann::json config_json;

void goBack() {
    Mix_PlayChannel(-1, this->sound_button, 0);
    this->change_to_scene = this->parent_scene;
}

void destroyAddUserModal() {
    for(Entity*& e : this->modal_entities) {
        if(e != nullptr) { e->destroy(); e = nullptr; }
    }
    this->modal_entities.clear();
    this->modal_entities.shrink_to_fit();
}

void createAddUserModal() {
    Entity* modal_content = &createUITextField(
        "modal_content_textfield", " ", 0, 0,
        TextFieldEditStyle::IP, // TODO: change this to GENERAL later when I implement it
        Game::default_text_color, COLORS_BLACK, Game::default_text_color,
        [this](TextBoxComponent& self) {
            Mix_PlayChannel(-1, this->sound_button, 0);
            self.editing = true;
            self.cursor_pos = self.text_content[0].size();
        },
        groupModalForeground
    );
    this->modal_entities = createUIModal(
        "modal_add_user", ModalContentType::MODAL_TEXTFIELD, modal_content,
        Game::SCREEN_WIDTH>>2, Game::SCREEN_HEIGHT>>2, Game::SCREEN_WIDTH>>2, Game::SCREEN_HEIGHT>>2, 
        [this](TextBoxComponent& self) {
            Mix_PlayChannel(-1, this->sound_button, 0);
            this->destroyAddUserModal();
        },
        [this, modal_content](TextBoxComponent& self) {
            Mix_PlayChannel(-1, this->sound_button, 0);
            std::cout << modal_content->getComponent<TextBoxComponent>().text_content[0] << '\n';
        },
        COLORS_NAVIGABLE, COLORS_IMPASSABLE
    );
}

void setUsersIpTable() {
    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };
    std::string header_label = "Username";

    std::vector<std::string> users = {};
    std::vector<std::string> ips = {};
    int max_size_user = 0;
    for(auto const& [user, ip] : Game::USERS_IP) {
        users.push_back(user);
        ips.push_back(ip);
        if(user.size() > max_size_user) { max_size_user = user.size(); }
    }
    if(header_label.size() > max_size_user) { max_size_user = header_label.size(); }

    for(std::string& user : users) {
        int to_pad = max_size_user - user.size();
        std::string padding = "";
        for(int i=0; i<to_pad; ++i) { padding += ' '; }
        user = padding + user;
    }
    int to_pad = max_size_user - header_label.size();
    std::string padding = "";
    for(int i=0; i<to_pad; ++i) { padding += ' '; }
    header_label = padding + header_label;

    if(this->table_header_users != nullptr) {
        this->table_header_users->destroy();
        this->table_header_ips->destroy();
        this->table_column_users->destroy();
        this->table_column_ips->destroy();
        this->table_column_users = nullptr;
        this->table_column_ips = nullptr;
        this->table_header_users = nullptr;
        this->table_header_ips = nullptr;
    }

    this->table_header_users = &createUIMultilineText(
        "table_headers_users", { header_label },
        50, 50,
        Game::default_text_color, background_color, border_color
    );
    TextBoxComponent& users_header = this->table_header_users->getComponent<TextBoxComponent>();
    this->table_header_ips = &createUIMultilineText(
        "table_headers_users", { "IP" },
        users_header.x + users_header.w - users_header.border_thickness, users_header.y,
        Game::default_text_color, background_color, border_color
    );

    this->table_column_users = &createUIMultilineText(
        "users_column", users,
        users_header.x, users_header.y + users_header.h,
        Game::default_text_color, background_color, border_color
    );
    this->table_column_ips = &createUIMultilineText(
        "ips_column", ips,
        this->table_header_ips->getComponent<TextBoxComponent>().x, users_header.y + users_header.h,
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

    TextBoxComponent& t_header_ips = this->table_header_ips->getComponent<TextBoxComponent>();
    Entity* add_button = &createUIButton(
        "button_add_user", 
        " + ",
        t_header_ips.x + t_header_ips.w + 4, t_header_ips.y,
        COLORS_GREEN, COLORS_BLACK, border_color,
        [this](TextBoxComponent& self) {
            this->createAddUserModal();
        }
    );
    TextBoxComponent& t_add_button = add_button->getComponent<TextBoxComponent>();
    createUIButton(
        "button_delete_user", 
        " - ",
        t_add_button.x + t_add_button.w + 4, t_add_button.y,
        COLORS_RED, COLORS_BLACK, border_color,
        [this](TextBoxComponent& self) {
            std::cout << "Pressed -\n";
        }
    );
    
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
            for(auto& modal_fg_ui : this->modal_fg_ui_elements) {
                if(modal_fg_ui->hasComponent<TextBoxComponent>()) {
                    if(modal_fg_ui->getComponent<TextBoxComponent>().onMousePress(pos)) {
                        return;
                    }
                }
            }
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
    for(auto& modal_fg_ui : this->modal_fg_ui_elements) {
        if(modal_fg_ui->hasComponent<TextBoxComponent>()) {
            if(modal_fg_ui->getComponent<TextBoxComponent>().onMouseRelease(pos)) {
                return true;
            }
        }
    }
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
            clickedButton(pos);
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

void handleKeyDown(SDL_Keycode key) {
    for(auto& modal_fg_ui : this->modal_fg_ui_elements) {
        if(modal_fg_ui->hasComponent<TextBoxComponent>()) {
            if(modal_fg_ui->getComponent<TextBoxComponent>().editing) {
                modal_fg_ui->getComponent<TextBoxComponent>().handleKeyDown(key);
                return;
            }
        }
    }
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<TextBoxComponent>()) {
            if(ui->getComponent<TextBoxComponent>().editing) {
                ui->getComponent<TextBoxComponent>().handleKeyDown(key);
                return;
            }
        }
    }
}
void handleKeyUp(SDL_Keycode key) {
    if(this->event->key.keysym.sym == SDLK_ESCAPE) {
        if(this->modal_entities.size() == 0 || this->modal_entities[0] == nullptr) {
            this->goBack();
            return;
        }
    }
    for(auto& modal_fg_ui : this->modal_fg_ui_elements) {
        if(modal_fg_ui->hasComponent<TextBoxComponent>()) {
            if(modal_fg_ui->getComponent<TextBoxComponent>().editing) {
                modal_fg_ui->getComponent<TextBoxComponent>().handleKeyUp();
                return;
            }
        }
    }
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<TextBoxComponent>()) {
            if(ui->getComponent<TextBoxComponent>().editing) {
                ui->getComponent<TextBoxComponent>().handleKeyUp();
                return;
            }
        }
    }
    if(this->event->key.keysym.sym == SDLK_ESCAPE) {
        if(this->modal_entities.size() > 0 && this->modal_entities[0] != nullptr) {
            this->goBack();
            return;
        }
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
                handleKeyUp(this->event->key.keysym.sym);
            } break;
            case SDL_KEYDOWN: {
                handleKeyDown(this->event->key.keysym.sym);
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
    for(auto& modal_bg_ui : this->modal_bg_ui_elements) { modal_bg_ui->draw(); }
    for(auto& modal_fg_ui : this->modal_fg_ui_elements) { modal_fg_ui->draw(); }
}
void clean() {
    Game::manager->clearEntities();
    this->destroyAddUserModal();
    this->table_header_users = nullptr;
    this->table_header_ips = nullptr;
    this->table_column_users = nullptr;
    this->table_column_ips = nullptr;
    this->modal_entities = {};
}
};