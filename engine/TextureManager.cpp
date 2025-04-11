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