#pragma once

#include <cmath>
#include "../ECS.hpp"
#include "../TransformComponent.hpp"
#include "../../Vector2D.hpp"

class CircleCollider : public Component {
    private:
        // TODO: make amount of points part of the constructor input
        const int amount = 32;
        const float fraction = 2*M_PI/this->amount;

        // this hull uses the center as its reference
        void setHull(float sc=1.0f) {
            float r = this->radius * sc;

            this->center = Vector2D(
                this->transform->position.x + r, 
                this->transform->position.y + r
            );
            
            for(int i=0; i<this->amount; ++i) {
                this->hull[i] = this->center + Vector2D(
                                                    r * cosf(this->fraction*i), 
                                                    r * sinf(this->fraction*i)
                                                );
            }
        }

    public:
        float radius;
        Vector2D center;
        std::vector<Vector2D> hull = std::vector<Vector2D>(this->amount);

        TransformComponent *transform;

        CircleCollider(TransformComponent* transf) {
            this->transform = transf;
        }

        void init() override {
            this->radius = this->transform->width/2;
        }

        void update() override {
            setHull();
        }

        void draw() override {
            setHull();
        }
};