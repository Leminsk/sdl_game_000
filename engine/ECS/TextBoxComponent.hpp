#pragma once
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include "../utils.hpp"
#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "Colliders/Collision.hpp"
#include "../TextFieldEditStyle.hpp"

// Text with background surrounded by a border
class TextBoxComponent : public Component {
private:
SDL_Texture *texture = NULL;
std::vector<SDL_Texture*> textures = {};
std::vector<int> width_per_line = {};
SDL_Rect srcRect;
bool single_line = true;
int lines_amount = 1;
int line_thickness;

bool key_already_pressed = false;
uint64_t key_frame_ref = 0;

const int v_line_gap = 2;
const int h_line_gap = 2;
const int v_char_height = 32; // height in pixels of a single character in the text line
const int h_char_width = 16; // width in pixels of a single character in the text line

std::function<void(TextBoxComponent&)> onMouseUp = nullptr;
std::function<void(TextBoxComponent&)> onMouseDown = nullptr;

void setProportions(int pos_x, int pos_y, const std::vector<std::string>& text_lines, int border_thickness) {
    const int double_thickness = border_thickness<<1;
    int max_char_on_line = 0;
    for(std::string line : text_lines) {
        if(line.size() > max_char_on_line) { max_char_on_line = line.size(); }
    }
    const int total_width = (max_char_on_line * this->h_char_width) + (this->h_line_gap<<1) + double_thickness;
    const int total_height = (text_lines.size() * (this->v_char_height + this->v_line_gap)) + this->v_line_gap + double_thickness;

    this->border_thickness = border_thickness;

    this->x = pos_x < 0 ? ((Game::SCREEN_WIDTH - total_width) + pos_x) : pos_x; 
    this->y = pos_y < 0 ? ((Game::SCREEN_HEIGHT - total_height) + pos_y) : pos_y;
    this->w = total_width; 
    this->h = total_height;

    this->inner_x = this->x + border_thickness;
    this->inner_y = this->y + border_thickness;
    this->inner_w = total_width - double_thickness;
    this->inner_h = total_height - double_thickness;
}

void setColors(const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c) {
    this->text_color = t_c;
    this->bg_color = bg_c;
    this->border_color = b_c;
    this->mouse_down_text_color   = { static_cast<uint8_t>( t_c.r>>1), static_cast<uint8_t>( t_c.g>>1), static_cast<uint8_t>( t_c.b>>1), SDL_ALPHA_OPAQUE };
    this->mouse_down_bg_color     = { static_cast<uint8_t>(bg_c.r>>1), static_cast<uint8_t>(bg_c.g>>1), static_cast<uint8_t>(bg_c.b>>1), SDL_ALPHA_OPAQUE };
    this->mouse_down_border_color = { static_cast<uint8_t>( b_c.r>>1), static_cast<uint8_t>( b_c.g>>1), static_cast<uint8_t>( b_c.b>>1), SDL_ALPHA_OPAQUE };
}


public:
SDL_FRect destRect;
SDL_FRect bgRect;
SDL_FRect borderRect;
SDL_FRect cursorRect;
std::vector<std::string> text_content = {}; // each element holds the text of one line

double rotation = 0;
int rotation_tick = 1;
bool rotating = false;

int border_thickness;

const char* font_path;
SDL_Color text_color, bg_color, border_color;
SDL_Color mouse_down_text_color, mouse_down_bg_color, mouse_down_border_color;
bool mouse_down = false;
float w = 0.0f; float inner_w;
float h = 0.0f; float inner_h;
float x = 0.0f; float inner_x;
float y = 0.0f; float inner_y;


bool editing = false;
TextFieldEditStyle edit_style = TextFieldEditStyle::NONE;
int cursor_pos = 0;
int character_limit;

TextBoxComponent(
    const std::string& text, 
    float pos_x, float pos_y,
    const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c,
    int border_thickness=3,
    std::function<void(TextBoxComponent&)> onUp = nullptr,
    std::function<void(TextBoxComponent&)> onDown = nullptr,
    TextFieldEditStyle edit_style=TextFieldEditStyle::NONE,
    int character_limit=-1,
    const char* path=nullptr
) : onMouseUp(onUp), onMouseDown(onDown) {
    this->edit_style = edit_style;
    this->character_limit = character_limit;
    if(character_limit > 0) {
        std::string padding_string = "";
        for(int i=0; i<character_limit; ++i) { padding_string += ' '; }
        setProportions(pos_x, pos_y, { padding_string }, border_thickness);
    } else {
        setProportions(pos_x, pos_y, { text }, border_thickness);
    }    
    setColors(t_c, bg_c, b_c);
    this->font_path = path;
    setText(text, path);
}
// multiline does not set mouse callbacks by default
TextBoxComponent(
    const std::vector<std::string>& text_lines, 
    float pos_x, float pos_y,
    const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c,
    int border_thickness=3, const char* path=nullptr
) {
    setProportions(pos_x, pos_y, text_lines, border_thickness);
    setColors(t_c, bg_c, b_c);
    this->font_path = path;            
    this->single_line = false;
    this->lines_amount = text_lines.size();
    setMultiText(text_lines, path);
}
~TextBoxComponent() {
    this->text_content.clear();
    this->text_content.shrink_to_fit();
    SDL_DestroyTexture(this->texture);
    for(SDL_Texture* t : this->textures) {
        if(t != NULL) { SDL_DestroyTexture(t); }
    }
}

void setText(std::string text="", const char* path=nullptr) {
    if(this->texture != NULL) { SDL_DestroyTexture(this->texture); }
    
    int width, height;
    if(text == "") { 
        const char* placeholder = " ";
        this->texture = TextureManager::LoadTextTexture(placeholder, this->text_color, width, height, path);
    } else {
        this->texture = TextureManager::LoadTextTexture(text.c_str(), this->text_color, width, height, path);
    }    
    this->text_content = { text };
}
void setMultiText(const std::vector<std::string>& lines, const char* path=nullptr) {
    for(SDL_Texture* t : this->textures) {
        if(t != NULL) { SDL_DestroyTexture(t); }
    }
    this->width_per_line.clear();
    this->width_per_line.shrink_to_fit();
    this->textures.clear();
    this->textures.shrink_to_fit();
    this->text_content.clear();
    this->text_content.shrink_to_fit();
    for(std::string line : lines) {
        this->width_per_line.push_back(line.size() * this->h_char_width);
        int width, height;
        this->textures.push_back( TextureManager::LoadTextTexture(line.c_str(), this->text_color, width, height, path) );
        this->text_content.push_back(line);
    }
}
void setRenderRects(float x, float y, float width, float height) {
    this->x = x; 
    this->y = y;
    this->w = width; 
    this->h = height;
    this->inner_x = x + this->border_thickness; 
    this->inner_y = y + this->border_thickness;
    this->inner_w = width - (this->border_thickness + this->border_thickness); 
    this->inner_h = height - (this->border_thickness + this->border_thickness);

    this->init();
}

// runs callback for onMouseDown if mouse_pos is inside the button area
// returns true if callback was run
bool onMousePress(const Vector2D& mouse_pos) {
    if(this->onMouseDown == nullptr) { return false; }
    bool pressed_on_button = false;
    bool clicked_zone = false;
    if(this->character_limit > 0) { 
        clicked_zone = Collision::pointInRect(
            mouse_pos.x, mouse_pos.y, 
            this->x, this->y, 
            (this->character_limit * this->h_char_width) + (this->h_line_gap<<1) + (this->border_thickness<<1),
            this->h
        );
    } else {
        clicked_zone = Collision::pointInRect(mouse_pos.x, mouse_pos.y, this->x, this->y, this->w, this->h);
    }

    if(clicked_zone) {
        this->mouse_down = true;
        this->onMouseDown(*this);
        pressed_on_button = true;
    }
    return pressed_on_button;
}

// runs callback for onMouseUp if mouse_pos is inside the button area
// returns true if callback was run
bool onMouseRelease(const Vector2D& mouse_pos) {
    if(this->onMouseUp == nullptr) { return false; }
    bool released_on_button = false;
    bool clicked_zone = false;
    if(this->character_limit > 0) { 
        clicked_zone = Collision::pointInRect(
            mouse_pos.x, mouse_pos.y, 
            this->x, this->y, 
            (this->character_limit * this->h_char_width) + (this->h_line_gap<<1) + (this->border_thickness<<1),
            this->h
        );
    } else {
        clicked_zone = Collision::pointInRect(mouse_pos.x, mouse_pos.y, this->x, this->y, this->w, this->h);
    }

    if(this->mouse_down && clicked_zone) {
        this->onMouseUp(*this);
        released_on_button = true;
    } else {
        this->editing = false;
    }
    this->mouse_down = false;
    return released_on_button;
}

// handles text 
void handleTyping(const std::string& input_text) {
    if(this->key_already_pressed) {    
        if(timeDiffMs(this->key_frame_ref, Game::FRAME_COUNT, Game::MAX_FRAME_DELAY) >= 20) {
            this->key_already_pressed = false;
            this->key_frame_ref = Game::FRAME_COUNT;
        }
    } else {
        std::string left_side = this->text_content[0].substr(0, this->cursor_pos);
        std::string right_side = this->text_content[0].substr(this->cursor_pos);

        bool char_allowed = false;
        switch(edit_style) {
            case TextFieldEditStyle::IP: {
                char_allowed = std::find(
                    Game::allowed_ip_characters.begin(), 
                    Game::allowed_ip_characters.end(), 
                    input_text[0]
                ) != Game::allowed_ip_characters.end();
            } break;
            case TextFieldEditStyle::GENERAL: { 
                char_allowed = std::find(
                    Game::allowed_username_characters.begin(), 
                    Game::allowed_username_characters.end(), 
                    input_text[0]
                ) != Game::allowed_username_characters.end();
            } break;
            default: break;
        }

        bool changed_text = false;
        if(
            char_allowed && 
            this->text_content[0].size() < character_limit && 
            input_text.size() <= (character_limit - this->text_content[0].size())
        ) {
            this->key_already_pressed = true;
            this->key_frame_ref = Game::FRAME_COUNT;
            this->text_content[0] = left_side + input_text + right_side;
            changed_text = true;
            ++this->cursor_pos;
        } else {
            std::cout << "input_text not allowed:" << input_text << "||\n";
        }

        if(changed_text) {
            this->setProportions(this->x, this->y, this->text_content, this->border_thickness);
            this->setText(this->text_content[0]);
            this->init();
        }
    }
}

void handleKeyUp() {
    this->key_already_pressed = false;
}

void handleEventsPostPoll(const uint8_t* keystates) {
    if(this->key_already_pressed) {    
        if(timeDiffMs(this->key_frame_ref, Game::FRAME_COUNT, Game::MAX_FRAME_DELAY) >= 120) {
            this->key_already_pressed = false;
            this->key_frame_ref = Game::FRAME_COUNT;
        }
    } else {
        std::string& text = this->text_content[0];
        std::string left_side = text.substr(0, this->cursor_pos);
        std::string right_side = text.substr(this->cursor_pos);
        this->key_already_pressed = true;

        bool changed_text = false;
        if(keystates[SDL_SCANCODE_ESCAPE] || keystates[SDL_SCANCODE_RETURN] || keystates[SDL_SCANCODE_RETURN2]) {
            this->editing = false;
            SDL_StopTextInput();
        }
        // Ctrl+C
        else if((keystates[SDL_SCANCODE_LCTRL] || keystates[SDL_SCANCODE_RCTRL]) && keystates[SDL_SCANCODE_C]) {
            SDL_SetClipboardText(text.c_str());
        }
        // Ctrl+V
        else if((keystates[SDL_SCANCODE_LCTRL] || keystates[SDL_SCANCODE_RCTRL]) && keystates[SDL_SCANCODE_V]) {
            std::string clipboard_text = SDL_GetClipboardText();
            bool clipboard_allowed = false;
            switch(edit_style) {
                case TextFieldEditStyle::IP: {
                    for(char& c : clipboard_text) {
                        if(
                            std::find(
                                Game::allowed_ip_characters.begin(), 
                                Game::allowed_ip_characters.end(), 
                                c
                            ) == Game::allowed_ip_characters.end()
                        ) {
                            return;
                        }
                    }
                    clipboard_allowed = true;
                } break;
                case TextFieldEditStyle::GENERAL: { 
                    for(char& c : clipboard_text) {
                        if(
                            std::find(
                                Game::allowed_username_characters.begin(), 
                                Game::allowed_username_characters.end(), 
                                c
                            ) == Game::allowed_username_characters.end()
                        ) {
                            return;
                        }
                    }
                    clipboard_allowed = true;
                } break;
                default: break;
            }
            std::string temp_text = left_side + clipboard_text + right_side;
            if(temp_text.size() >= character_limit) { return ; }
            text = temp_text;
            this->cursor_pos += clipboard_text.size();
            changed_text = true;

        } else if(keystates[SDL_SCANCODE_BACKSPACE]) {
            if(this->cursor_pos > 0) {
                left_side.pop_back(); 
                text = left_side + right_side;
                --this->cursor_pos;
                changed_text = true;
            }
        } else if(keystates[SDL_SCANCODE_DELETE]) {
            if(this->cursor_pos < text.size()) {
                text = left_side + right_side.substr(1);
                changed_text = true;
            }
        } else if(keystates[SDL_SCANCODE_HOME]) {
            this->cursor_pos = 0;
        } else if(keystates[SDL_SCANCODE_END]) {
            this->cursor_pos = text.size();
        } else if(keystates[SDL_SCANCODE_LEFT]) {
            if(this->cursor_pos > 0) { 
                --this->cursor_pos; 
            }
        } else if(keystates[SDL_SCANCODE_RIGHT]) {
            if(this->cursor_pos < text.size()) { 
                ++this->cursor_pos; 
            }
        } else {
            this->key_already_pressed = false;
        }

        if(this->key_already_pressed) { 
            this->key_frame_ref = Game::FRAME_COUNT;
        }

        if(changed_text) {
            this->setProportions(this->x, this->y, this->text_content, this->border_thickness);
            this->setText(this->text_content[0]);
            this->init();
        }
    }
}

void init() override {
    this->srcRect.x = 0;                  this->srcRect.y = 0;
    this->srcRect.w = Game::SCREEN_WIDTH; this->srcRect.h = Game::SCREEN_HEIGHT;

    this->borderRect.x = this->x; this->borderRect.y = this->y;
    this->borderRect.w = this->w; this->borderRect.h = this->h;

    this->bgRect.x = this->inner_x; this->bgRect.y = this->inner_y;
    this->bgRect.w = this->inner_w; this->bgRect.h = this->inner_h;
    
    // always used for 1 line at a time
    this->destRect.x = this->inner_x + this->v_line_gap; 
    this->destRect.y = this->inner_y + this->v_line_gap;
    this->destRect.w = this->inner_w - (this->v_line_gap<<1);
    
    if(this->single_line) {
        this->destRect.h = this->inner_h - (this->v_line_gap<<1);
    } else {           
                                                    // top gap
        this->line_thickness = ((this->inner_h - this->v_line_gap) / this->lines_amount);
        this->destRect.h = this->line_thickness - this->v_line_gap; // bottom/inner gaps
    }
    
    this->cursorRect.x = this->destRect.x; this->cursorRect.y = this->destRect.y;
    this->cursorRect.w = 4;                this->cursorRect.h = this->destRect.h;
}
void update() override {
    if (this->rotating) {
        this->rotation = (static_cast<int>(this->rotation + this->rotation_tick) % 360);
    }
}
void draw() override {
    // UI elements are shown on screen, thus drawn by default
    SDL_Color color1 = this->border_color;
    SDL_Color color2 = this->bg_color;
    SDL_Color color3 = this->text_color;
    if(this->mouse_down) { 
        color1 = this->mouse_down_border_color;
        color2 = this->mouse_down_bg_color;
        color3 = this->mouse_down_text_color;
    }

    TextureManager::DrawRect(&this->borderRect, color1);
    TextureManager::DrawRect(&this->bgRect, color2);  // drawing over the background is faster than making a special routine for it

    if(this->single_line) {
        // srcRect is the entire screen, destRect is the inner portion of TextBox
        TextureManager::DrawText(color3, this->texture, &this->srcRect, &this->destRect, this->rotation, SDL_FLIP_NONE);
    } else {
        for(int i=0; i<this->lines_amount; ++i) {
            this->destRect.w = this->width_per_line[i];
            TextureManager::DrawText(color3, this->textures[i], &this->srcRect, &this->destRect, this->rotation, SDL_FLIP_NONE);
            this->destRect.y += this->line_thickness;
        }
        this->destRect.y = this->inner_y + this->v_line_gap;
    }

    if(this->editing) {
        this->cursorRect.x = this->destRect.x + (this->cursor_pos * this->h_char_width);
        TextureManager::DrawRect(&this->cursorRect, COLORS_GREEN);
    }
}
};