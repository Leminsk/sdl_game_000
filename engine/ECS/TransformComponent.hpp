#pragma once

#include "ECS.hpp"
#include "../Vector2D.hpp"

// Deals with geometric transformations: translation, rotation, etc
class TransformComponent : public Component {
    public:
        Vector2D position;
        Vector2D velocity;

        int speed = 3;

        TransformComponent() {
            this->position.x = 0.0f;
            this->position.y = 0.0f;
        }
        TransformComponent(float x, float y) {
            this->position.x = x;
            this->position.y = y;
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