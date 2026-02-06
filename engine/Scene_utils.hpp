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
Entity& createUISimpleText(
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
    return new_ui_text;
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
// if pos_x < 0 -> offset from the right (analogous with pos_y from the bottom)
Entity& createUITextField(
    const std::string& id, 
    const std::string& text="UI_TEXTFIELD",
    int pos_x=0, int pos_y=0,
    TextFieldEditStyle edit_style=TextFieldEditStyle::NONE,
    const SDL_Color& text_color=Game::default_text_color, 
    const SDL_Color& bg_color=Game::default_bg_color, 
    const SDL_Color& border_color=Game::default_text_color,
    std::function<void(TextBoxComponent&)> onUp = [](TextBoxComponent&){},
    groupLabels group=groupUI
) {
    auto& new_text_box(Game::manager->addEntity(id));
    new_text_box.addComponent<TextBoxComponent>(
        text, pos_x, pos_y,
        text_color, bg_color, border_color, 3,
        onUp, [](TextBoxComponent&){}, edit_style
    );
    new_text_box.addGroup(group);
    return new_text_box;
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
// this assumes that the content_entity has been created as part of the groupModalForeground
// will create a set of entities and return them as a vector of entity pointers
// { modal_background, cancel_button, confirm_button, content_entity }
std::vector<Entity*> createUIModal(
    const std::string& id, 
    ModalContentType content_type,
    Entity* content_entity,
    int pos_x=0, int pos_y=0, int width=100, int height=50, 
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
    const int content_x = pos_x + border_thickness + spacing;
    const int content_y = pos_y + border_thickness + spacing;
    int content_h;
    switch(content_type) {
        case ModalContentType::MODAL_TEXT:
            content_entity->getComponent<TextComponent>().x = content_x;
            content_entity->getComponent<TextComponent>().y = content_y;
            content_h = content_entity->getComponent<TextComponent>().h;
            break;
        case ModalContentType::MODAL_TEXTFIELD:
            content_entity->getComponent<TextBoxComponent>().setRenderRects(
                content_x, content_y, 
                content_entity->getComponent<TextBoxComponent>().w,
                content_entity->getComponent<TextBoxComponent>().h
            );
            content_h = content_entity->getComponent<TextBoxComponent>().h;
            break;
    }

    t_cancel.setRenderRects(
        content_x, content_y + content_h + spacing,
        t_cancel.w, t_cancel.h
    );
    t_confirm.setRenderRects(
        pos_x + width - (border_thickness + spacing + t_confirm.w), content_y + content_h + spacing,
        t_confirm.w, t_confirm.h
    );

    // abusing the fact that it creates a button, but only using it as background rects, I'm sorry future Lemos
    Entity* modal_background = &createUIButton(
        id + "_background", " ", pos_x, pos_y, 
        Game::default_text_color, bg_color, border_color,
        nullptr, nullptr,
        groupModalBackground
    );
    modal_background->getComponent<TextBoxComponent>().setRenderRects(pos_x, pos_y, width, height);
    return { modal_background, cancel_button, confirm_button, content_entity };
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