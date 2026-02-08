#pragma once

#include "Game.hpp"
#include "ECS/ECS.hpp"
#include "Colors.hpp"
#include "ECS/Components.hpp"
#include "ECS/Colliders/Collider.hpp"
#include "ECS/Colliders/Collision.hpp"
#include "GroupLabels.hpp"
#include "ModalContentType.hpp"
#include "TextFieldEditStyle.hpp"

Entity& createDrone(float pos_x, float pos_y, MainColors c) {
    auto& new_drone(Game::manager->addEntity("DRO" + left_pad_int(Game::UNIT_COUNTER, 5)));
    new_drone.addComponent<DroneComponent>(Vector2D(pos_x, pos_y), Game::UNIT_SIZE, Game::unit_tex, c);
    new_drone.addComponent<Wireframe>();
    new_drone.addComponent<TextComponent>("", 0, 0);
    new_drone.addGroup(groupDrones);
    return new_drone;
}
Entity& createUIImage(
    const std::string& id, SDL_Texture* image_texture,
    int pos_x=0, int pos_y=0, int width=32, int height=32
) {
    auto& new_ui_image(Game::manager->addEntity(id));
    int x = pos_x < 0 ? ((Game::SCREEN_WIDTH - width) + pos_x) : pos_x; 
    int y = pos_y < 0 ? ((Game::SCREEN_HEIGHT - height) + pos_y) : pos_y;
    new_ui_image.addComponent<TransformComponent>(pos_x, pos_y, width, height, 1);
    new_ui_image.addComponent<SpriteComponent>(image_texture);
    new_ui_image.addGroup(groupBackgroundUI);
    return new_ui_image;
}
Entity* createUISimpleText(
    const std::string& id, 
    int pos_x=0, int pos_y=0,
    const std::string& text="SIMPLE_TEXT",
    const SDL_Color& text_color=Game::default_text_color,
    groupLabels group=groupUI
) {
    auto& new_ui_text(Game::manager->addEntity(id));
    new_ui_text.addComponent<TextComponent>(
        text, pos_x, pos_y,
        text_color, true
    );
    new_ui_text.addGroup(group);
    return &new_ui_text;
}
// if pos_x < 0 -> offset from the right (analogous with pos_y from the bottom)
Entity& createUIButton(
    const std::string& id, 
    const std::string& text="UI_BUTTON",
    int pos_x=0, int pos_y=0,
    const SDL_Color& text_color=Game::default_text_color, 
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color,
    std::function<void(TextBoxComponent&)> onUp = [](TextBoxComponent&){},
    std::function<void(TextBoxComponent&)> onDown = [](TextBoxComponent&){},
    groupLabels group=groupUI
) {
    auto& new_text_box(Game::manager->addEntity(id));
    new_text_box.addComponent<TextBoxComponent>(
        text, pos_x, pos_y,
        text_color, bg_color, border_color, 3,
        onUp, onDown
    );
    new_text_box.addGroup(group);
    return new_text_box;
}
// single color rectangle background with a border
Entity* createUISimpleRectangle(
    const std::string& id, 
    int pos_x=0, int pos_y=0, int width=0, int height=0,
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color,
    groupLabels group=groupUI
) {
    // abusing the fact that it creates a button, but only using it as background rects. I'm sorry future Lemos
    Entity* background = &createUIButton(
        id, " ", pos_x, pos_y, 
        Game::default_text_color, bg_color, border_color,
        nullptr, nullptr,
        group
    );
    background->getComponent<TextBoxComponent>().setRenderRects(pos_x, pos_y, width, height);
    return background;
}
// if pos_x < 0 -> offset from the right (analogous with pos_y from the bottom)
// returns { text_field, background }
std::vector<Entity*> createUITextField(
    const std::string& id, 
    const std::string& text="UI_TEXTFIELD",
    int pos_x=0, int pos_y=0,
    TextFieldEditStyle edit_style=TextFieldEditStyle::NONE,
    int fixed_character_width=1,
    const SDL_Color& text_color=Game::default_text_color, 
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color,
    std::function<void(TextBoxComponent&)> onUp = [](TextBoxComponent&){},
    groupLabels group=groupUI
) {
    Entity& new_text_box(Game::manager->addEntity(id));
    new_text_box.addComponent<TextBoxComponent>(
        text, pos_x, pos_y,
        text_color, bg_color, border_color, 0,
        onUp, [](TextBoxComponent&){}, edit_style,
        fixed_character_width
    );
    
    const int border_thickness = 3;
    const int double_thickness = border_thickness<<1;
    TextBoxComponent& t_textbox = new_text_box.getComponent<TextBoxComponent>();
    Entity* background = createUISimpleRectangle(
        id + "_background", t_textbox.x, t_textbox.y, t_textbox.w + double_thickness, t_textbox.h + double_thickness,
        bg_color, border_color, group
    );
    // since the proper textfield was created without a border, we have to manually reset its position to fit "inside" the background
    t_textbox.setRenderRects(t_textbox.x + border_thickness, t_textbox.y + border_thickness, t_textbox.w, t_textbox.h);
    background->addGroup(group);
    new_text_box.addGroup(group);
    return { &new_text_box, background };
}

// if pos_x < 0 -> offset from the right (analogous with pos_y from the bottom)
Entity& createUIMultilineText(
    const std::string& id, 
    const std::vector<std::string>& lines,
    int pos_x=0, int pos_y=0,
    const SDL_Color& text_color=Game::default_text_color, 
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color
) {
    auto& new_multiline_text(Game::manager->addEntity(id));
    new_multiline_text.addComponent<TextBoxComponent>(
        lines, pos_x, pos_y,
        text_color, bg_color, border_color
    );
    new_multiline_text.addGroup(groupUI);
    return new_multiline_text;            
}
// if pos_x < 0 -> offset from the right (analogous with pos_y from the bottom)
Entity& createUIDropdown(
    const std::string& id,
    const std::vector<std::string>& labels,
    Mix_Chunk* selection_sound_effect,
    int pos_x=0, int pos_y=0,
    const SDL_Color& text_color=Game::default_text_color, 
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color,
    std::function<void(TextDropdownComponent&, int)> onUp = [](TextDropdownComponent&, int){},
    std::function<void(TextDropdownComponent&)> onDown = [](TextDropdownComponent&){}
) {
    auto& new_dropdown(Game::manager->addEntity(id));
    new_dropdown.addComponent<TextDropdownComponent>(
        labels, pos_x, pos_y,
        text_color, bg_color, border_color,
        selection_sound_effect,
        onUp, onDown
    );
    new_dropdown.addGroup(groupPriorityUI);
    return new_dropdown;
}
Entity& createUIDropdownColors(
    const std::string& id,
    Mix_Chunk* selection_sound_effect,
    int pos_x=0, int pos_y=0,
    const SDL_Color& text_color=Game::default_text_color, 
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color,
    std::function<void(TextDropdownComponent&, int)> onUp = [](TextDropdownComponent&, int){},
    std::function<void(TextDropdownComponent&)> onDown = [](TextDropdownComponent&){}
) {
    auto& new_dropdown_colors(Game::manager->addEntity(id));
    new_dropdown_colors.addComponent<TextDropdownComponent>(
        id, pos_x, pos_y,
        text_color, bg_color, border_color,
        selection_sound_effect,
        onUp, onDown
    );
    new_dropdown_colors.addGroup(groupPriorityUI);
    return new_dropdown_colors;
}
Entity& createUIMapThumbnail(
    const std::string& id, 
    const std::string map_dir="assets/maps/",
    const std::string map_name="test3",
    int pos_x=0, int pos_y=0, float scale=1.0f,
    bool custom_dimensions=false,
    float width=200.0f, float height=200.0f
) {
    auto& new_map_thumbnail(Game::manager->addEntity(id));

    if(custom_dimensions) {
        new_map_thumbnail.addComponent<MapThumbnailComponent>(
            map_dir, map_name, 
            pos_x, pos_y, width, height
        );
    } else {
        new_map_thumbnail.addComponent<MapThumbnailComponent>(
            map_dir, map_name, 
            pos_x, pos_y, scale
        );
    }    
    new_map_thumbnail.addGroup(groupUI);
    return new_map_thumbnail;
}
/**
 * `id`: base id used as prefix to all other ids created by this function
 * `content`: list of what Entities are going to be rendered in the Modal
 * this assumes that all content Entities have been created as part of the groupModalForeground
 * `content_types`: necessary enums describing main objects to be rendered by each Entity
 * `offsets`: the relative render position for each Entity in relation to the Modal's inner position (inside the border)
 * `pos_x`: Modal x coordinate
 * `pos_y`: Modal y coordinate
 * As with many other components, Modal's (x,y) is its outermost "top-left" corner (defaults to top-left corner of the screen)
 * `width`: Modal inner most width, excluding border thickness(3) and inner spacing(10)
 * `height`: Modal inner most height, excluding border thickness(3) and inner spacing(10)
 * `onUpCancel`: callback function to be run when the "Cancel" button gets clicked
 * `onUpConfirm`: callback function to be run when the "Confirm" button gets clicked
 * `bg_color`: Modal's main color
 * `border_color`: Modal's border color
 * 
 * returns a set of entities including the ones created by this function as well as the original content entities 
 * { modal_background, cancel_button, confirm_button, content_0, content_1, ... }
 */
std::vector<Entity*> createUIModal(
    const std::string& id,
    const std::vector<Entity*> content,
    const std::vector<ModalContentType> content_types,
    const std::vector<Vector2D> offsets,
    int pos_x=0, int pos_y=0, int width=0, int height=0,
    std::function<void(TextBoxComponent&)> onUpCancel = [](TextBoxComponent&){},
    std::function<void(TextBoxComponent&)> onUpConfirm = [](TextBoxComponent&){},
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color
) {
    Entity* cancel_button = &createUIButton(
        id + "_button_left", 
        "Cancel", 
        0, 0, 
        COLORS_RED, COLORS_BLACK, COLORS_RED, 
        onUpCancel, [](TextBoxComponent&){},
        groupModalForeground
    );
    Entity* confirm_button = &createUIButton(
        id + "_button_right", 
        "  OK  ", 
        0, 0, 
        COLORS_GREEN, COLORS_BLACK, COLORS_GREEN, 
        onUpConfirm, [](TextBoxComponent&){},
        groupModalForeground
    );
    TextBoxComponent& t_cancel = cancel_button->getComponent<TextBoxComponent>();
    TextBoxComponent& t_confirm = confirm_button->getComponent<TextBoxComponent>();

    const int border_thickness = 3;
    const int spacing = 10;
    const int base_x = pos_x + border_thickness + spacing;
    const int base_y = pos_y + border_thickness + spacing;

    for(int i=0; i<content.size(); ++i) {
        switch(content_types[i]) {
            case ModalContentType::MODAL_TEXT: {
                TextComponent& text = content[i]->getComponent<TextComponent>();
                text.setRenderPos(base_x + offsets[i].x, base_y + offsets[i].y, text.w, text.h);
            } break;
            case ModalContentType::MODAL_TEXTBOX: {
                TextBoxComponent& text_box = content[i]->getComponent<TextBoxComponent>();
                text_box.setRenderRects(base_x + offsets[i].x, base_y + offsets[i].y, text_box.w, text_box.h);
            } break;
        }
    }

    const int buttons_y = base_y + height + spacing;
    t_cancel.setRenderRects(base_x, buttons_y, t_cancel.w, t_cancel.h);
    const int modal_width = width + (spacing<<1) + (border_thickness<<1);
    const int modal_height = height + (spacing<<1) + spacing + t_cancel.h + (border_thickness<<1);
    const int button_confirm_x = base_x + width - t_confirm.w;
    t_confirm.setRenderRects(button_confirm_x, buttons_y, t_confirm.w, t_confirm.h);

    Entity* modal_background = createUISimpleRectangle(
        id + "_background", pos_x, pos_y, modal_width, modal_height,
        bg_color, border_color, groupModalBackground
    );

    std::vector<Entity*> res = { modal_background, cancel_button, confirm_button };
    for(Entity* e : content) { res.push_back(e); }
    return res;
}
Entity& createBaseBuilding(
    std::string id, 
    float world_pos_x, float world_pos_y, 
    float width,
    const SDL_Color& color
) {
    std::cout << "createBaseBuilding:" << id << " color:{" << (int)color.r << ' ' << (int)color.g << ' ' << (int)color.b << "} \n";
    auto& building(Game::manager->addEntity(id));
    building.addComponent<TransformComponent>(world_pos_x, world_pos_y, width, width, 1.0);
    building.addComponent<SpriteComponent>(Game::building_tex, color);
    building.addComponent<Collider>(ColliderType::HEXAGON);
    building.addComponent<Wireframe>();
    building.addGroup(groupBuildings);
    return building;
}
void SetSolidTileNeighbors(uint8_t* neighbors, int map_x, int map_y, const std::vector<std::vector<int>>& layout) {
        int dec_map_x = map_x-1;
        int inc_map_x = map_x+1;
        int dec_map_y = map_y-1;
        int inc_map_y = map_y+1;

        bool top_left  = false;
        bool top_mid   = false;
        bool top_right = false;
        bool left      = false;
        bool right     = false;
        bool bot_left  = false;
        bool bot_mid   = false;
        bool bot_right = false;

        int layout_width = layout[0].size();
        int layout_height = layout.size();

        if(map_x == 0) {
            if(map_y == 0) { // top left corner
                right     = layout[    map_y][inc_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            } else if(map_y == layout_height-1) { // bottom left corner
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
            } else { // left column
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            }
        } else if(map_x == layout_width-1) {
            if(map_y == 0) { // top right corner
                left      = layout[    map_y][dec_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
            } else if(map_y == layout_height-1) { // bottom right corner
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
            } else { // right column
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
            }
        } else {
            if(map_y == 0) { // top row
                left      = layout[    map_y][dec_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            } else if(map_y == layout_height-1) { // bottom row
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
            } else { // middle of the layout (most cases)
                top_left  = layout[dec_map_y][dec_map_x] == 2;
                top_mid   = layout[dec_map_y][    map_x] == 2;
                top_right = layout[dec_map_y][inc_map_x] == 2;
                left      = layout[    map_y][dec_map_x] == 2;
                right     = layout[    map_y][inc_map_x] == 2;
                bot_left  = layout[inc_map_y][dec_map_x] == 2;
                bot_mid   = layout[inc_map_y][    map_x] == 2;
                bot_right = layout[inc_map_y][inc_map_x] == 2;
            }
        }

        setBit(neighbors, 0, top_left);
        setBit(neighbors, 1, top_mid);
        setBit(neighbors, 2, top_right);
        setBit(neighbors, 3, left);
        setBit(neighbors, 4, right);
        setBit(neighbors, 5, bot_left);
        setBit(neighbors, 6, bot_mid);
        setBit(neighbors, 7, bot_right);
}