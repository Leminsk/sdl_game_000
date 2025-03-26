#pragma once

#include "ECS.hpp"
#include "../Vector2D.hpp"

// Deals with rendering transformations: translation, scaling
class TransformComponent : public Component {
    public:
        Vector2D position;
        Vector2D velocity;

        int height = 32;
        int width = 32;
        int scale = 1;

        int speed = 3;

        TransformComponent() {
            this->position.Zero();
        }
        TransformComponent(int sc) {
            this->position.Zero();
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
            this->velocity.Zero();
        }

        void update() override {
            this->position.x += this->velocity.x * speed;
            this->position.y += this->velocity.y * speed;
        }

        
};