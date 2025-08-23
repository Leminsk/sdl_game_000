#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"
#include "utils.hpp"

class SceneMatchSettings {
private:
int players_amount = 0;

Entity* title = nullptr;
Entity* selected_map = nullptr;
Entity* button_go = nullptr;
SceneType parent_scene;
std::vector<Entity*> spawn_info_entities = {};
std::vector<std::pair<int,int>> spawn_positions = {};

std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
SDL_Event* event;

public:
std::string map_name;
Mix_Chunk* sound_button = NULL;
TextComponent* fps_text = nullptr;
SceneType change_to_scene = SceneType::NONE;

SceneMatchSettings(SDL_Event* e) { this->event = e; }
~SceneMatchSettings() {}

void setScene(Mix_Chunk*& sound_b, TextComponent* fps, SceneType parent, const std::string& map_name) {
    this->sound_button = sound_b;
    this->fps_text = fps;
    this->parent_scene = parent;

    // reset pointers for when scene gets reset
    this->button_go = nullptr;
    this->selected_map = nullptr;
    this->spawn_info_entities.clear();
    this->spawn_info_entities.shrink_to_fit();

    this->map_name = map_name;
    this->selected_map = &createUIMapThumbnail(
        "map_preview_" + this->map_name, "assets/maps/", this->map_name, 
        20, 20, 
        1.0f, true, 
        500.0f, 500.0f
    );

    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };

    createUIButton("button_back", "Back", 50,  -50, Game::default_text_color, background_color, border_color);
    
    MapThumbnailComponent& thumbnail = this->selected_map->getComponent<MapThumbnailComponent>();
    std::vector<std::vector<SDL_Color>>& map_pixels = thumbnail.map_pixels;
    uint32_t map_width = thumbnail.map_width;
    uint32_t map_height = thumbnail.map_height;
    int width_max_digits = std::to_string(map_width).size();
    int heigth_max_digits = std::to_string(map_height).size();

    const int text_height = 32;
    const int v_spacing = 32;
    const int spawn_text_offset  = text_height + v_spacing;
    const int spawn_text_length_with_offset = ((width_max_digits + heigth_max_digits + 3) * 16) + 5;
    const int base_x = thumbnail.origin_x + thumbnail.border_rect.w + 64;
    int spawn_count = 0;
    int y, x;
    for(y=0; y<map_height; ++y) {
        for(x=0; x<map_width; ++x) {
            SDL_Color pixel = map_pixels[y][x];
            if(isSameColor(pixel, COLORS_SPAWN)) {
                int base_y = thumbnail.border_rect.y + (spawn_text_offset * spawn_count);
                spawn_positions.push_back({y,x});
                createUISimpleText(
                    "spawn_text_"+std::to_string(spawn_count), base_x, base_y,
                    '(' + left_pad_int(x+1, width_max_digits, ' ') + "," + left_pad_int(y+1, heigth_max_digits, ' ') + ')', 
                    COLORS_SPAWN
                );
                spawn_info_entities.push_back( 
                    &createUIDropdownColors(
                        "spawn_dropdown_"+std::to_string(spawn_count), 
                        base_x + spawn_text_length_with_offset, base_y,
                        COLORS_SPAWN, background_color
                    )
                );
                ++spawn_count;
            }
        }
    }

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

void handleMouseRelease(SDL_MouseButtonEvent& b) {
    Vector2D pos = Vector2D(b.x, b.y);
    switch(b.button) {
        case SDL_BUTTON_LEFT: {
            std::cout << "RELEASE LEFT: " << pos << '\n';
            for(auto& pr_ui : this->pr_ui_elements) {
                if(pr_ui->hasComponent<TextDropdownComponent>()) {
                    TextDropdownComponent& dropdown = pr_ui->getComponent<TextDropdownComponent>();
                    if(Collision::pointInRect(pos.x, pos.y, dropdown.x, dropdown.y, dropdown.w, dropdown.h)) {
                        std::string dropdown_id = pr_ui->getIdentifier();
                        if(dropdown_id.substr(0,14) == "spawn_dropdown") {
                            Mix_PlayChannel(-1, this->sound_button, 0);
                            dropdown.display_dropdown = !(dropdown.display_dropdown);
                            return;
                        }
                    }
                    if(dropdown.display_dropdown) {
                        for(int i=0; i<dropdown.options.size(); ++i) {
                            TextBoxComponent* option = dropdown.options[i];
                            if(Collision::pointInRect(pos.x, pos.y, option->x, option->y, option->w, option->h)) {
                                Mix_PlayChannel(-1, this->sound_button, 0);
                                dropdown.setSelectedOption(i);
                                for(int j=0; j<this->spawn_info_entities.size(); ++j) {
                                    if(this->spawn_info_entities[j]->getIdentifier() == pr_ui->getIdentifier()) {
                                        int y = this->spawn_positions[j].first;
                                        int x = this->spawn_positions[j].second;
                                        this->selected_map->getComponent<MapThumbnailComponent>().map_pixels[y][x] = dropdown.options_colors[i];
                                        break;
                                    }
                                }
                                return;
                            }
                        }
                        Mix_PlayChannel(-1, this->sound_button, 0);
                        dropdown.display_dropdown = false;
                    }   
                }
            }
            for(auto& ui : this->ui_elements) {
                if(ui->hasComponent<TextBoxComponent>()) {
                    TextBoxComponent& text_box = ui->getComponent<TextBoxComponent>();
                    if(
                        Collision::pointInRect(pos.x, pos.y, text_box.x, text_box.y, text_box.w, text_box.h) &&
                        ui->getComponent<TextBoxComponent>().mouse_down
                    ) { 
                        std::string button_id = ui->getIdentifier();
                        if(button_id == "button_back") {
                            Mix_PlayChannel(-1, this->sound_button, 0);
                            clean();                     
                            this->change_to_scene = this->parent_scene;
                            return;
                        }
                    }
                    ui->getComponent<TextBoxComponent>().mouse_down = false;
                }
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