#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"
#include "utils.hpp"

class SceneLobby {
private:
int thumbnail_width;
int thumbnail_height;
const int max_y_title = 20;
int min_y_title = this->max_y_title;
Entity* title = nullptr;
Entity* button_go = nullptr;
Entity* selected_map = nullptr;
std::vector<Entity*> maps_thumbnails;
int maps_amount;


std::vector<Entity*>& pr_ui_elements = Game::manager->getGroup(groupPriorityUI);
std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);
SDL_Event* event;

public:
Mix_Chunk* sound_button = NULL;
TextComponent* fps_text;
SceneType change_to_scene = SceneType::NONE;

SceneLobby(SDL_Event* e) { this->event = e; }
~SceneLobby() {}

void setScene(Mix_Chunk*& sound_b, TextComponent* fps) {
    this->sound_button = sound_b;
    this->fps_text = fps;

    const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
    const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };
    const SDL_Color dp_border_color  = {  40, 220, 200, SDL_ALPHA_OPAQUE };

    // createUIDropdown("dropdown", {"base", "option1", "option 2"}, 50, 50, Game::default_text_color, background_color, dp_border_color);
    // createUIDropdownColors("dropdown_colors", -50, 50, Game::default_text_color, background_color, dp_border_color);

    const int base_x = 70;
    const int base_y = 50;

    this->title = &createUISimpleText("select_map_title", base_x, max_y_title, "-- Map Selection --");
    
    const std::string maps_dir = "assets/maps/";
    std::vector<std::string> map_names = getFileNamesInDirectory(maps_dir, "BMP");
    this->maps_amount = map_names.size();
    this->maps_thumbnails.resize(this->maps_amount, nullptr);


    this->maps_thumbnails[0] = &createUIMapThumbnail("thumbnail_"+map_names[0], maps_dir, map_names[0], base_x, base_y);
    MapThumbnailComponent& base_thumbnail = this->maps_thumbnails[0]->getComponent<MapThumbnailComponent>();
    const int margin = 20;
    this->thumbnail_width  = base_thumbnail.border_rect.w + margin;
    this->thumbnail_height = base_thumbnail.border_rect.h + (base_thumbnail.text_height<<1) + (base_thumbnail.thumbnail_gap<<1) + margin;

    int width_overlap = (base_x + (this->maps_amount * this->thumbnail_width)) - Game::SCREEN_WIDTH;
    int row_max = this->maps_amount;
    if(width_overlap > 0) {
        row_max = this->maps_amount - std::ceil(static_cast<float>(width_overlap) / static_cast<float>(this->thumbnail_width));
    }

    int row_count = 1;
    int y_offset = 0;
    for(int i=1; i<this->maps_amount; ++i) {
        if(row_count == row_max) {
            ++y_offset;
            row_count = 0;
        }
        this->maps_thumbnails[i] = &createUIMapThumbnail(
            "thumbnail_" + std::to_string(i) + "_" + map_names[i], maps_dir, map_names[i],
            base_thumbnail.origin_x + (this->thumbnail_width * row_count),
            base_thumbnail.origin_y + (this->thumbnail_height * y_offset)
        );
        ++row_count;
    }

    const int back_button_y = 50;
    Entity& back_button = createUIButton("button_back", "Back", 50, -back_button_y, Game::default_text_color, background_color, border_color);

    int ui_elements_stacked_height = this->max_y_title + this->title->getComponent<TextComponent>().h + ((y_offset+1) * this->thumbnail_height);
    int height_overlap = ui_elements_stacked_height - Game::SCREEN_HEIGHT;
    if(height_overlap > 0) {
        this->min_y_title = this->max_y_title - (height_overlap + back_button_y + back_button.getComponent<TextBoxComponent>().h);
    } else {
        this->min_y_title = this->max_y_title;
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

bool clickedThumbnail(Vector2D& pos) {
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<MapThumbnailComponent>()) {
            MapThumbnailComponent& thumbnail = ui->getComponent<MapThumbnailComponent>();
            if(Collision::pointInRect(
                pos.x, pos.y, 
                thumbnail.map_title->x, thumbnail.map_title->y, 
                this->thumbnail_width, this->thumbnail_height
            )) {
                for(int i=0; i<this->maps_amount; ++i) {
                    MapThumbnailComponent& t = this->maps_thumbnails[i]->getComponent<MapThumbnailComponent>();
                    if(t.map_title->x == thumbnail.map_title->x && t.map_title->y == thumbnail.map_title->y) {
                        t.selected = true;
                        this->selected_map = this->maps_thumbnails[i];
                    } else {
                        t.selected = false;
                    }                                
                }
                return true;
            }
        }
    }
    return false;
}
bool clickedButton(Vector2D& pos) {
    for(auto& ui : this->ui_elements) {
        if(ui->hasComponent<TextBoxComponent>()) {
            TextBoxComponent& text_box = ui->getComponent<TextBoxComponent>();
            if(
                Collision::pointInRect(pos.x, pos.y, text_box.x, text_box.y, text_box.w, text_box.h) &&
                ui->getComponent<TextBoxComponent>().mouse_down
            ) {
                Mix_PlayChannel(-1, this->sound_button, 0);
                std::string button_id = ui->getIdentifier();
                if(button_id == "button_back") {
                    clean();
                    this->change_to_scene = SceneType::MAIN_MENU;
                } else if(button_id == "button_go") {
                    // go to other scene to set spawns on selected map: this->change_to_scene = SceneType:???
                }
                return true;
            }
            ui->getComponent<TextBoxComponent>().mouse_down = false;
        }
    }
    return false;
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
                        if(dropdown_id == "dropdown" || dropdown_id == "dropdown_colors") {
                            Mix_PlayChannel(-1, this->sound_button, 0);
                            pr_ui->getComponent<TextDropdownComponent>().display_dropdown = !(pr_ui->getComponent<TextDropdownComponent>().display_dropdown);
                            return;
                        }
                    }

                    if(pr_ui->getComponent<TextDropdownComponent>().display_dropdown) {
                        for(int i=0; i<dropdown.options.size(); ++i) {
                            TextBoxComponent* option = dropdown.options[i];
                            if(Collision::pointInRect(pos.x, pos.y, option->x, option->y, option->w, option->h)) {
                                Mix_PlayChannel(-1, this->sound_button, 0);
                                pr_ui->getComponent<TextDropdownComponent>().setSelectedOption(i);
                                return;
                            }
                        }
                    }
                }
            }

            if(!clickedButton(pos)) {
                if(clickedThumbnail(pos)) {
                    if(this->button_go == nullptr) {
                        this->button_go = &createUIButton("button_go", " Go ", -50, -50, Game::default_text_color, { 20, 20, 100, SDL_ALPHA_OPAQUE }, { 230, 210, 190, SDL_ALPHA_OPAQUE });
                    }
                } else {
                    this->selected_map = nullptr;
                    for(int i=0; i<this->maps_amount; ++i) {
                        this->maps_thumbnails[i]->getComponent<MapThumbnailComponent>().selected = false;                        
                    }
                    if(this->button_go != nullptr) {
                        this->button_go->destroy();
                        this->button_go = nullptr;
                    }
                }
            }
        } break;
        case SDL_BUTTON_RIGHT: {
            std::cout << "RELEASE RIGHT: " << pos << '\n';
            if(this->selected_map != nullptr) {
                std::cout << "selected_map: " << this->selected_map->getComponent<MapThumbnailComponent>().map_name << '\n';
            } else {
                printf("no selected_map\n");
            }
            

            // was just debugging if it would really delete the entity from the screen (it did)
            // int destroyed_index = -1;
            // for(int i=0; i<this->maps_amount; ++i) {
            //     if(this->maps_thumbnails[i] != nullptr && this->maps_thumbnails[i]->getComponent<MapThumbnailComponent>().selected) {
            //         this->maps_thumbnails[i]->destroy();
            //         this->maps_thumbnails[i] = nullptr;
            //         destroyed_index = i;
            //         break;
            //     }
            // }
            // if(destroyed_index >= 0) {
            //     const int last_index = this->maps_amount-1; 
            //     if(destroyed_index == last_index) {
            //         this->maps_thumbnails.resize(destroyed_index);
            //     } else {
            //         for(int i=destroyed_index; i<last_index; ++i) {
            //             this->maps_thumbnails[i] = this->maps_thumbnails[i+1];
            //         }
            //         this->maps_thumbnails.resize(last_index);
            //     }
            //     this->maps_thumbnails.shrink_to_fit();
            // }
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
        TextComponent& title_text = this->title->getComponent<TextComponent>();
        int potential_new_pos = title_text.y + scroll_offset;
        if(potential_new_pos <= this->max_y_title && potential_new_pos >= this->min_y_title) {
            title_text.setRenderPos(title_text.x, potential_new_pos, title_text.w, title_text.h);
            for(auto& ui : this->ui_elements) {
                if(ui->getIdentifier().substr(0, 9) == "thumbnail") {
                    MapThumbnailComponent& thumbnail = ui->getComponent<MapThumbnailComponent>();
                    thumbnail.map_title->setRenderPos(
                        thumbnail.map_title->x, 
                        thumbnail.map_title->y + scroll_offset, 
                        thumbnail.map_title->w, 
                        thumbnail.map_title->h
                    );
                    thumbnail.border_rect.y += scroll_offset;
                    thumbnail.map_rect.y += scroll_offset;
                    thumbnail.map_dimensions_subtitle->setRenderPos(
                        thumbnail.map_dimensions_subtitle->x,
                        thumbnail.map_dimensions_subtitle->y + scroll_offset,
                        thumbnail.map_dimensions_subtitle->w,
                        thumbnail.map_dimensions_subtitle->h
                    );
                }
            }
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