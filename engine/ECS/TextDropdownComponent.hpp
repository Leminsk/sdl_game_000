#pragma once
#include <string>
#include <vector>
#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "../GroupLabels.hpp"
#include "../Colors.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "SpriteComponent.hpp"
#include "TextBoxComponent.hpp"


//
class TextDropdownComponent : public Component {
private:
    void setOptions(
        const std::vector<std::string>& op_vec,
        const int options_amount,
        float pos_x, float pos_y,
        const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c
    ) {
        // default is always the first
        // create one first and let it calculate the correct position for the base of the dropdown
        this->selected_option_label = op_vec[0];
        this->selected_option = new TextBoxComponent(
            this->padded_labels[0], 
            pos_x, pos_y, 
            t_c, bg_c, b_c
        );
        this->selected_option->init();

        this->w = this->selected_option->w;
        this->h = this->selected_option->h;
        this->x = this->selected_option->x;
        this->y = this->selected_option->y;

        this->options.resize(options_amount);
        for(int i=0; i<options_amount; ++i) {
            this->options[i] = new TextBoxComponent(
                this->padded_labels[i], 
                this->x, this->y + (this->h * (i+1)), 
                t_c, bg_c, b_c
            );
            this->options[i]->init();
        }
    }

public:
bool is_color_dropdown = false;
TextBoxComponent* selected_option;
TransformComponent* selected_option_transform; // only used because of SpriteComponent
SpriteComponent* selected_option_sprite;

std::string selected_option_label;
std::vector<TextBoxComponent*> options = {};
// for color dropdown selector
std::vector<SDL_Color> options_colors = {
    COLORS_SPAWN,
    COLORS_WHITE,
    COLORS_BLACK,
    COLORS_RED,
    COLORS_GREEN,
    COLORS_BLUE,
    COLORS_YELLOW,
    COLORS_CYAN,
    COLORS_MAGENTA
};
std::vector<Entity*> options_entities = {};

// default to color dropdown selector
std::vector<std::string> options_labels = { "Random", "White", "Black", "Red", "Green", "Blue", "Yellow", "Cyan", "Magenta" };
std::vector<std::string> padded_labels = {};

int border_thickness;

// SDL_Color text_color, bg_color, border_color;
// SDL_Color mouse_down_text_color, mouse_down_bg_color, mouse_down_border_color;
bool display_dropdown = false;

float w = 0.0f;
float h = 0.0f;
float x = 0.0f;
float y = 0.0f;

TextDropdownComponent(
    const std::string& id,
    float pos_x, float pos_y,
    const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c
) {
    this->is_color_dropdown = true;
    const int options_amount = this->options_labels.size();
    const int max_chars = 7;
    this->padded_labels.resize(options_amount);
    // special case for "Random"
    int chars_to_pad = max_chars - this->options_labels[0].size();
    std::string padded = this->options_labels[0];
    for(int i=0; i<chars_to_pad; ++i) { padded += ' '; }
    // escaping '?'s because of trigraph warning
    this->padded_labels[0] = "\?\?-" + padded;

    for(int i=1; i<options_amount; ++i) {
        chars_to_pad = max_chars - this->options_labels[i].size();
        std::string padded = this->options_labels[i];
        for(int j=0; j<chars_to_pad; ++j) { padded += ' '; }
        this->padded_labels[i] = "  -" + padded;
    }

    setOptions(this->options_labels, options_amount, pos_x, pos_y, t_c, bg_c, b_c);

    this->options_entities.resize(options_amount);
    
    // for the base of the dropdown
    auto& selected_entity( Game::manager->addEntity(id + "_selected") );
    this->selected_option_transform = &selected_entity.addComponent<TransformComponent>(
        this->selected_option->destRect.x, this->selected_option->destRect.y, 
        32.0f, this->selected_option->destRect.h, 1.0f
    );
    this->selected_option_sprite = &selected_entity.addComponent<SpriteComponent>();

    // special case for the first option "Random"
    auto& first_option_entity( Game::manager->addEntity(id + "_" + this->options_labels[0]) );
    first_option_entity.addComponent<TransformComponent>(
        this->options[0]->destRect.x, this->options[0]->destRect.y, 
        32.0f, this->options[0]->destRect.h, 1.0f
    );
    first_option_entity.addComponent<SpriteComponent>();
    this->options_entities[0] = &first_option_entity;

    for(int i=1; i<options_amount; ++i) {
        auto& option_entity( Game::manager->addEntity(id + "_" + this->options_labels[i]) );
        option_entity.addComponent<TransformComponent>(
            this->options[i]->destRect.x, this->options[i]->destRect.y, 
            32.0f, this->options[i]->destRect.h, 1.0f
        );
        option_entity.addComponent<SpriteComponent>(Game::unit_tex, this->options_colors[i]);        
        // don't add to any group since the draw cycle must be controlled from this Component
        this->options_entities[i] = &option_entity;
    }

}

TextDropdownComponent(
    const std::vector<std::string>& labels,
    float pos_x, float pos_y,
    const SDL_Color& t_c, const SDL_Color& bg_c, const SDL_Color& b_c
) {
    this->options_labels = labels;

    // padding the labels is easier than recalculating the correct texture rendering widths. but maybe it is computationally worse
    const int options_amount = labels.size();
    int max_chars = 0;
    for(const std::string& op : labels) {
        if(op.size() > max_chars) { max_chars = op.size(); }
    }
    this->padded_labels.resize(options_amount);
    for(int i=0; i<options_amount; ++i) {
        int chars_to_pad = max_chars - labels[i].size();
        std::string padded = labels[i];
        for(int j=0; j<chars_to_pad; ++j) { padded += ' '; }
        this->padded_labels[i] = padded;
    }
    
    setOptions(labels, options_amount, pos_x, pos_y, t_c, bg_c, b_c);
}

~TextDropdownComponent() {
    for(auto& o : this->options_entities) { o->destroy(); o = nullptr; }
    for(auto& o : this->options) { delete o; o = nullptr; }
    this->options_entities = {};
    this->options = {};
    delete this->selected_option; this->selected_option = nullptr;
}

void setSelectedOption(const int option_index=0) {
    if(option_index >= 0 && option_index < this->options_labels.size()) {
        if(this->is_color_dropdown) {
            if(option_index == 0) {
                this->selected_option_sprite->texture = NULL;
            } else {
                this->selected_option_sprite->texture = this->options_entities[option_index]->getComponent<SpriteComponent>().texture;
                this->selected_option_sprite->color_modulation = this->options_colors[option_index];
            }
        }        
        this->selected_option_label = this->options_labels[option_index];
        this->selected_option->setText(this->padded_labels[option_index]);
        this->display_dropdown = false;
    }
}

void init() override {
}
void update() override {
}
void draw() override {
    this->selected_option->draw();

    if(this->is_color_dropdown) {
        if(this->selected_option_sprite->texture != NULL) { this->selected_option_sprite->draw(); }    
        if(display_dropdown) {
            for(TextBoxComponent*& option : this->options) { option->draw(); }
            for(int i=1; i<this->options_entities.size(); ++i) { this->options_entities[i]->draw(); }
        }
    } else {
        if(display_dropdown) {
            for(TextBoxComponent*& option : this->options) { option->draw(); }
        }
    }
}
};