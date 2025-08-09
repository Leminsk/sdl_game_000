#pragma once
#include <string>
#include <vector>
#include "../Camera.hpp"
#include "../TextureManager.hpp"
#include "ECS.hpp"
#include "TextBoxComponent.hpp"

//
class TextDropdownComponent : public Component {
private:

public:
TextBoxComponent* selected_option;
std::string selected_option_label;
std::vector<TextBoxComponent*> options = {};
std::vector<std::string> options_labels = {};
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
    for(const std::string& op : labels) {
        int chars_to_pad = max_chars - op.size();
        std::string padded = op;
        for(int i=0; i<chars_to_pad; ++i) { padded += ' '; }
        this->padded_labels.push_back(padded);
    }
    

    // default is always the first
    // create one first and let it calculate the correct position for the base of the dropdown
    selected_option_label = labels[0];
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
        options[i] = new TextBoxComponent(
            this->padded_labels[i], 
            this->x, this->y + (this->h * (i+1)), 
            t_c, bg_c, b_c
        );
        options[i]->init();
    }
}
~TextDropdownComponent() {
}

void init() override {
}
void update() override {
}
void draw() override {
    this->selected_option->draw();
    if(display_dropdown) {
        for(TextBoxComponent*& option : this->options) { option->draw(); }
    }
}
};