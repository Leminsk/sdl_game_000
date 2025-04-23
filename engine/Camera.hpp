#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "Vector2D.hpp"

// camera_pos is the upper left point of the SDL_FRect

Vector2D convertWorldToScreen(SDL_FRect& camera, const Vector2D& world_pos) {
    Vector2D screen_pos;
    screen_pos.x = world_pos.x - camera.x;
    screen_pos.y = world_pos.y - camera.y;
    return screen_pos;
}

Vector2D convertScreenToWorld(SDL_FRect& camera, const Vector2D& screen_pos) {
    Vector2D world_pos;
    world_pos.x = screen_pos.x + camera.x;
    world_pos.y = screen_pos.y + camera.y;
    return world_pos;
}
