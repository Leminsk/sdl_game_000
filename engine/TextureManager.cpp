#include "TextureManager.hpp"

// When in doubt: https://stackoverflow.com/questions/21007329/what-is-an-sdl-renderer

SDL_Texture* TextureManager::LoadTexture(const char* texture) {
    SDL_Surface* tempSurface = IMG_Load(texture);
    if(tempSurface == NULL) {
        SDL_Log("Unable to render surface. SDL_ttf Error: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
    if(tex == NULL) {
        SDL_Log("Unable to create texture. SDL Error: %s\n", SDL_GetError());
        return NULL;
    }
    SDL_FreeSurface(tempSurface);
    return tex;
}

SDL_Texture* TextureManager::LoadTextTexture(const char* text, const SDL_Color& color, int& output_w, int& output_h, const char* font_path) {
    SDL_Surface* tempSurface;
    if(font_path == nullptr) {
        tempSurface = TTF_RenderUTF8_Solid(Game::default_font, text, color);
    } else {
        TTF_Font *font = TTF_OpenFont(font_path, 28);
        tempSurface = TTF_RenderUTF8_Solid(font, text, color);
        TTF_CloseFont(font);
        font = NULL;
    }

    if(tempSurface == NULL) {
        SDL_Log("Unable to render surface. SDL_ttf Error: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
    if(tex == NULL) {
        SDL_Log("Unable to create texture. SDL Error: %s\n", SDL_GetError());
        return NULL;
    }
    output_w = tempSurface->w;
    output_h = tempSurface->h;
    return tex;
}




void TextureManager::Draw(SDL_Texture* tex, SDL_Rect src, SDL_FRect dest, double rotation_degrees, SDL_RendererFlip flip) {
    SDL_RenderCopyExF(
        Game::renderer, tex, &src,
        &dest, rotation_degrees, NULL, flip
    );
}

void TextureManager::DrawText(SDL_Color color, SDL_Texture* tex, SDL_Rect src, SDL_FRect dest, double rotation_degrees, SDL_RendererFlip flip) {
    SDL_SetRenderDrawColor(Game::renderer, color.r, color.g, color.b, color.a);
    TextureManager::Draw(tex, src, dest, rotation_degrees, flip);
    SDL_SetRenderDrawColor(Game::renderer, Game::bg_color.r, Game::bg_color.g, Game::bg_color.b, Game::bg_color.a);
}

void TextureManager::DrawWireframe(const SDL_FPoint* points, int count, const Uint8* color) {
    SDL_SetRenderDrawColor(Game::renderer, color[0], color[1], color[2], SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(Game::renderer, points, count);
    SDL_SetRenderDrawColor(Game::renderer, Game::bg_color.r, Game::bg_color.g, Game::bg_color.b, Game::bg_color.a);
}