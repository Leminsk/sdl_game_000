#include "GameObject.hpp"
#include "TextureManager.hpp"

GameObject::GameObject(const char* textureSheet,int x, int y) {
    this->objTexture = TextureManager::LoadTexture(textureSheet);

    this->xpos = x;
    this->ypos = y;
}

void GameObject::Update() {
    this->srcRect.h = 32;
    this->srcRect.w = 32;
    this->srcRect.x = 0;
    this->srcRect.y = 0;

    this->destRect.x = this->xpos;
    this->destRect.y = this->ypos;
    this->destRect.w = this->srcRect.w * 2;
    this->destRect.h = this->srcRect.h * 2;
}

void GameObject::Render() {
    SDL_RenderCopy(Game::renderer, this->objTexture, &this->srcRect, &this->destRect);
}