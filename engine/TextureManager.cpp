#include "TextureManager.hpp"

// When in doubt: https://stackoverflow.com/questions/21007329/what-is-an-sdl-renderer

SDL_Texture* TextureManager::LoadTexture(const char* texture) {
    SDL_Surface* tempSurface = IMG_Load(texture);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return tex;
}


void TextureManager::Draw(SDL_Texture* tex, SDL_Rect src, SDL_FRect dest, double rotation_degrees, SDL_RendererFlip flip) {
    SDL_RenderCopyExF(
        Game::renderer, tex, &src,
        &dest, rotation_degrees, NULL, flip
    );
}

void TextureManager::DrawWireframe(const SDL_FPoint* points, int count, const Uint8* color) {
    SDL_SetRenderDrawColor(Game::renderer, color[0], color[1], color[2], SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLinesF(Game::renderer, points, count);
    SDL_SetRenderDrawColor(Game::renderer, 200, 200, 200, SDL_ALPHA_OPAQUE);
}