#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "Game.hpp"
#include "Vector2D.hpp"

// https://gamedev.stackexchange.com/questions/121421/how-to-use-the-sdl-viewport-properly
// camera_pos is the upper left point of the SDL_FRect

// screen = world - camera
Vector2D panWorldToScreen(const Vector2D& world_pos) {
    return world_pos - Game::camera_position;
}

Vector2D panScreenToWorld(const Vector2D& screen_pos) {
    return screen_pos + Game::camera_position;
}

// should only be used AFTER panning (i.e. after panWorldToScreen() => Pan first, Zoom Second)
// When going from screen to world, do the reverse (deZoom First, convertScreenToWorld() second)
Vector2D applyZoom(const Vector2D& screen_pos, const Vector2D& focus_point) {
    return Vector2D(
        Game::camera_zoom * screen_pos.x - (focus_point.x * (Game::camera_zoom - 1.0f)),
        Game::camera_zoom * screen_pos.y - (focus_point.y * (Game::camera_zoom - 1.0f))
    );
}

// should only be used BEFORE panning (i.e. before panScreenToWorld() => deZoom first, dePan Second)
Vector2D deZoom(const Vector2D& zoomed_screen_pos, const Vector2D& focus_point) {
    return Vector2D(
        (zoomed_screen_pos.x + (focus_point.x * (Game::camera_zoom - 1.0f))) / Game::camera_zoom,
        (zoomed_screen_pos.y + (focus_point.y * (Game::camera_zoom - 1.0f))) / Game::camera_zoom
    );
}

Vector2D convertWorldToScreen(
    const Vector2D& world_pos, 
    const Vector2D& focus_point = Game::camera_focus
) {
    return applyZoom( panWorldToScreen(world_pos), focus_point );
}

Vector2D convertScreenToWorld(
    const Vector2D& screen_pos, 
    const Vector2D& focus_point = Game::camera_focus
) {
    return panScreenToWorld( deZoom(screen_pos, focus_point) );
}