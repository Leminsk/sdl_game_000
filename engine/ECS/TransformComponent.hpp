#pragma once

#include "../Vector2D.hpp"

// Deals with rendering transformations: translation, scaling
class TransformComponent : public Component {
    public:
        Vector2D position; // should be world coordinates
        Vector2D velocity;

        float height = 32.0f;
        float width = 32.0f;
        float scale = 1.0f;

        float speed = 100.0f; // 100 pixels per second
        float acceleration = 0;

        TransformComponent() {
            this->position.Zero();
        }
        TransformComponent(float sc) {
            this->position.Zero();
            this->scale = sc;
        }
        TransformComponent(float x, float y) {
            this->position.x = x;
            this->position.y = y;
        }
        TransformComponent(float x, float y, float w, float h, float sc) {
            this->position.x = x;
            this->position.y = y;
            this->height = h;
            this->width = w;
            this->scale = sc;
        }
        ~TransformComponent() {}

        Vector2D getCenter() {
            return Vector2D(
                this->position.x + this->width/2, 
                this->position.y + this->height/2
            );
        }

        void init() override {
            this->velocity.Zero();
        }

        void update() override {
            this->speed += this->acceleration * Game::FRAME_DELTA * 0.5f;
            this->position.x += this->velocity.x * speed * Game::FRAME_DELTA;
            this->position.y += this->velocity.y * speed * Game::FRAME_DELTA;
            this->speed += this->acceleration * Game::FRAME_DELTA * 0.5f;                        
        }

        
};