#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "Vector2D.hpp"

// camera_pos is the upper left point of the SDL_FRect

// screen = world - camera
Vector2D convertWorldToScreen(SDL_FRect& camera, const Vector2D& world_pos) {
    return Vector2D(
        world_pos.x - camera.x,
        world_pos.y - camera.y
    );
}

Vector2D convertScreenToWorld(SDL_FRect& camera, const Vector2D& screen_pos) {
    return Vector2D(
        screen_pos.x + camera.x, 
        screen_pos.y + camera.y
    );
}

// should only be used after panning (i.e. after convertWorldToScreen())
Vector2D applyZoom(SDL_FRect& camera, const Vector2D& screen_pos, float zoom) {
    Vector2D focus_point = Vector2D(
        (camera.x + (camera.w/2.0f)), 
        (camera.y + (camera.h/2.0f))
    );
    return Vector2D(
        zoom * screen_pos.x - (focus_point.x * (zoom-1.0f)),
        zoom * screen_pos.y - (focus_point.y * (zoom-1.0f))
    );
}