#pragma once

#include "../Vector2D.hpp"

// Deals with rendering transformations: translation, scaling
class TransformComponent : public Component {
    public:
        Vector2D position;
        Vector2D velocity;

        float height = 32.0f;
        float width = 32.0f;
        float scale = 1.0f;

        float speed = 500.0f;
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

        void init() override {
            this->velocity.Zero();
        }

        void update(const float& frame_delta) override {
            if(frame_delta > 0) {
                this->speed += this->acceleration * frame_delta * 0.5f;
                this->position.x += this->velocity.x * speed * frame_delta;
                this->position.y += this->velocity.y * speed * frame_delta;
                this->speed += this->acceleration * frame_delta * 0.5f;
            } else {
                this->position.x += this->velocity.x * speed;
                this->position.y += this->velocity.y * speed;
            }
                        
        }

        
};