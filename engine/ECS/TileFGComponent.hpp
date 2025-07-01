#pragma once

#include <cmath>
#include "../utils.hpp"
#include "../Vector2D.hpp"
#include "TileTypes.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "SpriteComponent.hpp"


class TileFGComponent : public Component {
    public:        
        SDL_Texture* texture;
        SDL_FRect tileRect;
        int tileID;
        const char *path;

        Vector2D rotation_center;
        const int amount_of_points = 64;
        const float fraction = 2*M_PI/this->amount_of_points;

        TileFGComponent() = default;

        TileFGComponent(float x, float y, float w, float h, int id, SDL_Texture* t) {
            this->tileRect.x = x;
            this->tileRect.y = y;
            this->tileRect.w = w;
            this->tileRect.h = h;
            this->tileID = id;
            this->texture = t;
            rotation_center.x = x; rotation_center.y = y;
        }

        void init() override {
            entity->addComponent<TransformComponent>(
                this->tileRect.x, this->tileRect.y,
                this->tileRect.w, this->tileRect.h,
                1
            );
            entity->addComponent<SpriteComponent>(this->texture);
        }

        void update() override {
            // yes it's related to the frame rate but for this thing I don't care that much
            float f = this->fraction * ((Game::FRAME_COUNT>>1) % this->amount_of_points);
            entity->getComponent<TransformComponent>().position = rotation_center + (Vector2D(5.0f * cosf(f), 5.0f * sinf(f)) * randomFloat(Game::RNG, 0.95f, 1.05f));
        }

};