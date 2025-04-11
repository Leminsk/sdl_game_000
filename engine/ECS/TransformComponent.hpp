#pragma once

#include "ECS.hpp"
#include "../Vector2D.hpp"

// Deals with rendering transformations: translation, scaling
class TransformComponent : public Component {
    public:
        Vector2D position;
        Vector2D velocity;

        float height = 32.0f;
        float width = 32.0f;
        float scale = 1;

        float speed = 3;

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

        void init() override {
            this->velocity.Zero();
        }

        void update() override {
            this->position.x += this->velocity.x * speed;
            this->position.y += this->velocity.y * speed;
        }

        
};