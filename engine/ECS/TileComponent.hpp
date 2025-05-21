#pragma once

#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "SpriteComponent.hpp"
#include "Wireframe.hpp"

class TileComponent : public Component {
    public:        
        SDL_Texture* texture;
        SDL_FRect tileRect;
        int tileID;
        const char *path;

        TileComponent() = default;

        TileComponent(float x, float y, float w, float h, float id, SDL_Texture* t) {
            this->tileRect.x = x;
            this->tileRect.y = y;
            this->tileRect.w = w;
            this->tileRect.h = h;
            this->tileID = id;
            this->texture = t;
        }

        void init() override {
            entity->addComponent<TransformComponent>(
                this->tileRect.x, this->tileRect.y,
                this->tileRect.w, this->tileRect.h,
                1
            );
            entity->addComponent<SpriteComponent>(this->texture);
            switch(this->tileID) {
                case 1: break;
                case 2: 
                    entity->addComponent<Collider>("mountain_collider", COLLIDER_RECTANGLE);
                    entity->addComponent<Wireframe>();
                    break;
                case 3: break;
                default:break;
            }
        }

};