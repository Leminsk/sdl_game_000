#pragma once

#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "SpriteComponent.hpp"
#include <SDL2/SDL.h>

class TileComponent : public Component {
    public:
        TransformComponent *transform;
        SpriteComponent *sprite;

        SDL_Rect tileRect;
        int tileID;
        const char *path;

        TileComponent() = default;

        TileComponent(int x, int y, int w, int h, int id) {
            this->tileRect.x = x;
            this->tileRect.y = y;
            this->tileRect.w = w;
            this->tileRect.h = h;
            this->tileID = id;

            switch(this->tileID) {
                case 0:
                    this->path = "assets/tiles/water.png";
                    break;
                case 1:
                    this->path = "assets/tiles/dirt.png";
                    break;
                case 2:
                    this->path = "assets/tiles/grass.png";
                    break;
                default:
                    break;
            }
        }

        void init() override {
            entity->addComponent<TransformComponent>(
                static_cast<float>(this->tileRect.x), static_cast<float>(this->tileRect.y),
                this->tileRect.w, this->tileRect.h,
                1
            );
            this->transform = &entity->getComponent<TransformComponent>();
            entity->addComponent<SpriteComponent>(this->path);
            this->sprite = &entity->getComponent<SpriteComponent>();
        }

};