#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"

class SceneLobby {
private:
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
        const SDL_Color dp_border_color  = {  20, 150, 200, SDL_ALPHA_OPAQUE };

        createUIDropdown("dropdown", {"base", "option1", "option22"}, 50, 50, Game::default_text_color, background_color, dp_border_color);

        createUIButton("button_back", "Back", 50,  -50, Game::default_text_color, background_color, border_color);
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
                                if(dropdown_id == "dropdown") {
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
                    for(auto& ui : this->ui_elements) {
                        if(ui->hasComponent<TextBoxComponent>()) {
                            TextBoxComponent& text_box = ui->getComponent<TextBoxComponent>();
                            if(Collision::pointInRect(pos.x, pos.y, text_box.x, text_box.y, text_box.w, text_box.h)) { 
                                if(ui->getComponent<TextBoxComponent>().mouse_down) {
                                    std::string button_id = ui->getIdentifier();
                                    if(button_id == "button_back") {
                                        Mix_PlayChannel(-1, this->sound_button, 0);
                                        clean();
                                        this->change_to_scene = SceneType::MAIN_MENU;
                                        break;
                                    }
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