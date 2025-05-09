#pragma once

#include "Game.hpp"

class TextureManager {
    public:
        static SDL_Texture* LoadTexture(const char* fileName);
        static SDL_Texture* LoadTextTexture(const char* text, const SDL_Color& color,  int& output_w, int& output_h, const char* font_path=nullptr);
        static void Draw(SDL_Texture* tex, SDL_Rect src, SDL_FRect dest, double rotation_degrees, SDL_RendererFlip flip);
        static void DrawText(SDL_Color color, SDL_Texture* tex, SDL_Rect src, SDL_FRect dest, double rotation_degrees, SDL_RendererFlip flip);
        static void DrawWireframe(const SDL_FPoint* points, int count, const Uint8* color);
};