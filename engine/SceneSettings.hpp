#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"

class SceneSettings {
    private:
        std::vector<Entity*>&    ui_elements = Game::manager->getGroup(groupUI);
        std::vector<Entity*>& bg_ui_elements = Game::manager->getGroup(groupBackgroundUI);

    public:
        SDL_Event* event;
        Mix_Chunk* sound_button = NULL;
        TextComponent* fps_text;
        SceneType change_to_scene = SceneType::NONE;

        SceneSettings(SDL_Event* e) { this->event = e; }
        ~SceneSettings() {}

        void setScene(Mix_Chunk*& sound_b, TextComponent* fps) {
            this->sound_button = sound_b;
            this->fps_text = fps;

            const SDL_Color background_color = {  20,  20, 100, SDL_ALPHA_OPAQUE };
            const SDL_Color border_color     = { 230, 210, 190, SDL_ALPHA_OPAQUE };
            
            // TODO: store these resolutions in some kind of table or hashmap
            createUIButton("button_res_SVGA",   " 800 x  600",  50,  50, Game::default_text_color, background_color, border_color);
            createUIButton("button_res_WXGA",   "1280 x  720",  50, 114, Game::default_text_color, background_color, border_color);
            createUIButton("button_res_1.56M3", "1440 x 1080",  50, 178, Game::default_text_color, background_color, border_color);
            createUIButton("button_res_FHD",    "1920 x 1080",  50, 242, Game::default_text_color, background_color, border_color);

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
                                    } else if(button_id == "button_res_SVGA") {
                                        changeScreenResolution(800, 600);
                                        clean();
                                        this->change_to_scene = SceneType::SETTINGS;
                                        break;
                                    } else if(button_id == "button_res_WXGA") {
                                        changeScreenResolution(1280, 720);
                                        clean();
                                        this->change_to_scene = SceneType::SETTINGS;
                                        break;
                                    } else if(button_id == "button_res_1.56M3") {
                                        changeScreenResolution(1440, 1080);
                                        clean();
                                        this->change_to_scene = SceneType::SETTINGS;
                                        break;
                                    } else if(button_id == "button_res_FHD") {
                                        changeScreenResolution(1920, 1080);
                                        clean();
                                        this->change_to_scene = SceneType::SETTINGS;
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
        }
        void clean() {
            Game::manager->clearEntities();
        }
};