#include "TextureManager.hpp"

SDL_Texture* TextureManager::LoadTexture(const char* texture) {
    SDL_Surface* tempSurface = IMG_Load(texture);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(Game::renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return tex;
}


void TextureManager::Draw(SDL_Texture* tex, SDL_Rect src, SDL_Rect dest, double rotation_degrees, SDL_RendererFlip flip) {
    SDL_RenderCopyEx(
        Game::renderer, tex, &src, &dest, 
        rotation_degrees, NULL, flip
    );
}