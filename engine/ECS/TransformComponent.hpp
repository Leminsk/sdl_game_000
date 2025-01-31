#pragma once

#include "ECS.hpp"
#include "../Vector2D.hpp"

// Deals with geometric transformations: translation, rotation, etc
class TransformComponent : public Component {
    public:
        Vector2D position;
        Vector2D velocity;

        int height = 32;
        int width = 32;
        int scale = 1;

        int speed = 3;

        TransformComponent() {
            this->position.x = 0.0f;
            this->position.y = 0.0f;
        }
        TransformComponent(int sc) {
            this->position.x = 0.0f;
            this->position.y = 0.0f;
            this->scale = sc;
        }
        TransformComponent(float x, float y) {
            this->position.x = x;
            this->position.y = y;
        }
        TransformComponent(float x, float y, int h, int w, int sc) {
            this->position.x = x;
            this->position.y = y;
            this->height = h;
            this->width = w;
            this->scale = sc;
        }

        void init() override {
            this->velocity.x = 0;
            this->velocity.y = 0;
        }

        void update() override {
            this->position.x += this->velocity.x * speed;
            this->position.y += this->velocity.y * speed;
        }

        
};