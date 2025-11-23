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
std::vector<Entity*> spawn_selections = {};
std::vector<Entity*> spawn_info_entities = {};

std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
SDL_Event* event;

public:
std::string map_name;
Mix_Chunk* sound_button = NULL;
TextComponent* fps_text = nullptr;
SceneType change_to_scene = SceneType::NONE;
std::vector<std::pair<int,int>> spawn_positions = {};
int player_spawn_index = -1;
SDL_Color player_sdl_color = COLORS_SPAWN;
std::vector<std::vector<SDL_Color>> map_pixels = {};

SceneMatchSettings(SDL_Event* e) { this->event = e; }
~SceneMatchSettings() {}

void setScene(Mix_Chunk*& sound_b, TextComponent* fps, SceneType parent, const std::string& map_name) {
    this->sound_button = sound_b;
    this->fps_text = fps;
    this->parent_scene = parent;
    this->map_name = map_name;

    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };

    // TODO: this will actually crash the game if clicked initially with "Random" colors. IMPLEMENT random color assignment on scene change.
    this->button_go = &createUIButton(
        "button_go", 
        "PLAY", 
        -50, -50, 
        Game::default_text_color, background_color, border_color,
        [this](TextBoxComponent& self) {
            Mix_PlayChannel(-1, this->sound_button, 0);
            this->map_pixels = this->selected_map->getComponent<MapThumbnailComponent>().map_pixels;
            this->player_sdl_color = this->map_pixels[
                this->spawn_positions[ this->player_spawn_index ].first
            ][
                this->spawn_positions[ this->player_spawn_index ].second
            ];
            this->change_to_scene = SceneType::MATCH_GAME;
        }
    );

    this->selected_map = &createUIMapThumbnail(
        "map_preview_" + this->map_name, "assets/maps/", this->map_name, 
        20, 20, 
        1.0f, true, 
        500.0f, 500.0f
    );
    
    // can't be reset on clean() it's shared with MatchGame
    this->spawn_positions = {};

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
    
    MapThumbnailComponent& thumbnail = this->selected_map->getComponent<MapThumbnailComponent>();
    std::vector<std::vector<SDL_Color>>& map_pixels = thumbnail.map_pixels;
    uint32_t map_width = thumbnail.map_width;
    uint32_t map_height = thumbnail.map_height;
    int width_max_digits = std::to_string(map_width).size();
    int height_max_digits = std::to_string(map_height).size();

    const int text_height = 32;
    const int v_spacing = 32;
    const int spawn_text_offset  = text_height + v_spacing;
    const int spawn_text_length_with_offset = ((width_max_digits + height_max_digits + 3) * 16) + 5;
    const int base_x = thumbnail.origin_x + thumbnail.border_rect.w + 64;
    int spawn_count = 0;
    int y, x;
    for(y=0; y<map_height; ++y) {
        for(x=0; x<map_width; ++x) {
            SDL_Color pixel = map_pixels[y][x];
            if(isSameColor(pixel, COLORS_SPAWN)) {
                int base_y = thumbnail.border_rect.y + (spawn_text_offset * spawn_count);
                this->spawn_positions.push_back({y,x});
                createUISimpleText(
                    "spawn_text_"+std::to_string(spawn_count), base_x, base_y,
                    '(' + left_pad_int(x+1, width_max_digits, ' ') + "," + left_pad_int(y+1, height_max_digits, ' ') + ')', 
                    COLORS_SPAWN
                );
                this->spawn_info_entities.push_back( 
                    &createUIDropdownColors(
                        "spawn_dropdown_"+std::to_string(spawn_count), 
                        base_x + spawn_text_length_with_offset, base_y,
                        COLORS_SPAWN, background_color
                    )
                );
                this->spawn_selections.push_back(
                    &createUIDropdown(
                        "spawn_selection_dropdown_"+std::to_string(spawn_count), { "CPU", "Player" }, 
                        base_x + spawn_text_length_with_offset + 10 + this->spawn_info_entities[spawn_count]->getComponent<TextDropdownComponent>().w, base_y
                    )
                );
                ++spawn_count;
            }
        }
    }
    this->players_amount = spawn_count;
}

bool isValidPlayerSpawn() {
    int player_count = 0;
    for(Entity*& e : this->spawn_selections) {
        if(e->getComponent<TextDropdownComponent>().selected_option_label == "Player") {
            ++player_count;
        }
    }
    return player_count <= 1;
}

bool isValidMapSpawns() {
    MapThumbnailComponent& map_thumbnail = this->selected_map->getComponent<MapThumbnailComponent>();
    std::vector<std::vector<SDL_Color>>& map_pixels = map_thumbnail.map_pixels;
    std::vector<SDL_Color> colors_found = {};
    
    for(uint32_t y=0; y<map_thumbnail.map_height; ++y) {
        for(uint32_t x=0; x<map_thumbnail.map_width; ++x) {
            SDL_Color& curr_pix = map_pixels[y][x];
            if(isSameColor(curr_pix, COLORS_SPAWN)) { continue; }
            if(
                !isSameColor(curr_pix, COLORS_IMPASSABLE) &&
                !isSameColor(curr_pix, COLORS_NAVIGABLE) &&
                !isSameColor(curr_pix, COLORS_PLAIN) &&
                !isSameColor(curr_pix, COLORS_ROUGH)
            ) {
                for(int i=0; i<colors_found.size(); ++i) {
                    if(isSameColor(colors_found[i], curr_pix)) { 
                        std::cout << "colors_found.size():" << colors_found.size() << " curr_pix: (" << (int)curr_pix.r << ' ' << (int)curr_pix.g << ' ' << (int)curr_pix.b << ")\n";
                        return false; 
                    }
                }
                colors_found.push_back(curr_pix);
            }
        }
    }
    return true;
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

bool clickedDropdown(Vector2D& pos) {
    for(auto& pr_ui : this->pr_ui_elements) {
        if(pr_ui->hasComponent<TextDropdownComponent>()) {
            std::string dropdown_id = pr_ui->getIdentifier();
            TextDropdownComponent& dropdown = pr_ui->getComponent<TextDropdownComponent>();

            if(Collision::pointInRect(pos.x, pos.y, dropdown.x, dropdown.y, dropdown.w, dropdown.h)) {
                if(dropdown_id.substr(0,5) == "spawn") {
                    Mix_PlayChannel(-1, this->sound_button, 0);
                    dropdown.display_dropdown = !(dropdown.display_dropdown);
                    return true;
                }
            }

            if(dropdown.display_dropdown) {
                if(dropdown_id.substr(0,14) == "spawn_dropdown") {
                    for(int i=0; i<dropdown.options.size(); ++i) {
                        TextBoxComponent* option = dropdown.options[i];
                        if(Collision::pointInRect(pos.x, pos.y, option->x, option->y, option->w, option->h)) {
                            Mix_PlayChannel(-1, this->sound_button, 0);
                            dropdown.setSelectedOption(i);
                            for(int j=0; j<this->spawn_info_entities.size(); ++j) {
                                if(this->spawn_info_entities[j]->getIdentifier() == dropdown_id) {
                                    int y = this->spawn_positions[j].first;
                                    int x = this->spawn_positions[j].second;
                                    this->selected_map->getComponent<MapThumbnailComponent>().map_pixels[y][x] = dropdown.options_colors[i];
                                    return true;
                                }
                            }
                        }
                    }
                }
                if(dropdown_id.substr(0,15) == "spawn_selection") {
                    for(int i=0; i<dropdown.options.size(); ++i) {
                        TextBoxComponent* option = dropdown.options[i];
                        if(Collision::pointInRect(pos.x, pos.y, option->x, option->y, option->w, option->h)) {
                            Mix_PlayChannel(-1, this->sound_button, 0);
                            dropdown.setSelectedOption(i);
                            if(i == 1) { // selecting "Player" means deselect for all others
                                for(int j=0; j<this->spawn_selections.size(); ++j) {
                                    if(this->spawn_selections[j]->getIdentifier() == dropdown_id) {
                                        this->player_spawn_index = j;
                                    } else {
                                        this->spawn_selections[j]->getComponent<TextDropdownComponent>().setSelectedOption(0);
                                    }                                    
                                }
                            }
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
                if(clickedDropdown(pos)) {
                    if(isValidMapSpawns() && isValidPlayerSpawn()) {
                        if(this->button_go == nullptr) {
                            this->button_go = &createUIButton(
                                "button_go", 
                                "PLAY", 
                                -50, -50, 
                                Game::default_text_color, { 20, 20, 100, SDL_ALPHA_OPAQUE }, { 230, 210, 190, SDL_ALPHA_OPAQUE },
                                [this](TextBoxComponent& self) {
                                    self.mouse_down = false;
                                    Mix_PlayChannel(-1, this->sound_button, 0);
                                    this->map_pixels = this->selected_map->getComponent<MapThumbnailComponent>().map_pixels;
                                    this->player_sdl_color = this->map_pixels[
                                        this->spawn_positions[ this->player_spawn_index ].first
                                    ][
                                        this->spawn_positions[ this->player_spawn_index ].second
                                    ];
                                    this->change_to_scene = SceneType::MATCH_GAME;
                                },
                                [this](TextBoxComponent& self) {
                                    self.mouse_down = true;
                                }
                            );
                        }
                    } else if(this->button_go != nullptr) {
                        this->button_go->destroy();
                        this->button_go = nullptr;
                    }                    
                }
            }
        } break;
        case SDL_BUTTON_RIGHT: break;
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
    this->button_go = nullptr;
    this->selected_map = nullptr;
    this->spawn_info_entities = {};        
    this->spawn_selections = {};
}
};