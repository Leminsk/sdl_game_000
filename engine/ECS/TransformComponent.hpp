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
        TransformComponent(float x, float y, float h, float w, float sc) {
            this->position.x = x;
            this->position.y = y;
            this->height = h;
            this->width = w;
            this->scale = sc;
        }
        ~TransformComponent() {}

        void init() override {
            this->velocity.Zero();
        }

        void update() override {
            this->speed += this->acceleration * Game::frame_delta * 0.5f;
            this->position.x += this->velocity.x * speed * Game::frame_delta;
            this->position.y += this->velocity.y * speed * Game::frame_delta;
            this->speed += this->acceleration * Game::frame_delta * 0.5f;                        
        }

        
};