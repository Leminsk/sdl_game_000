#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "Vector2D.hpp"
#include "ECS/TransformComponent.hpp"

// https://gamedev.stackexchange.com/questions/121421/how-to-use-the-sdl-viewport-properly
// camera_pos is the upper left point of the SDL_FRect

// screen = world - camera
Vector2D convertWorldToScreen(const Vector2D& camera_pos, const Vector2D& world_pos) {
    return Vector2D(
        world_pos.x - camera_pos.x,
        world_pos.y - camera_pos.y
    );
}

Vector2D convertScreenToWorld(const Vector2D& camera_pos, const Vector2D& screen_pos) {
    return Vector2D(
        screen_pos.x + camera_pos.x, 
        screen_pos.y + camera_pos.y
    );
}

// should only be used after panning (i.e. after convertWorldToScreen() => Pan first, Zoom Second)
// When going from screen to world, do the reverse (deZoom First, dePan second)
Vector2D applyZoom(const TransformComponent& camera_transform, const Vector2D& screen_pos) {
    Vector2D focus_point = Vector2D(Game::SCREEN_WIDTH>>1, Game::SCREEN_HEIGHT>>1);
    return Vector2D(
        camera_transform.scale * screen_pos.x - (focus_point.x * (camera_transform.scale - 1.0f)),
        camera_transform.scale * screen_pos.y - (focus_point.y * (camera_transform.scale - 1.0f))
    );
}