#pragma once

#include "Game.hpp"

class TextureManager {
    public:
        static SDL_Texture* LoadTexture(const char* fileName);
        static void Draw(SDL_Texture* tex, SDL_Rect src, SDL_FRect dest, double rotation_degrees, SDL_RendererFlip flip);
        static void DrawWireframe(const SDL_FPoint* points, int count, const Uint8* color);
};