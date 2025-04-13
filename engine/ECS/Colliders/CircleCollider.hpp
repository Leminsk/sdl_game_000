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
        void setHull() {
            this->center = Vector2D(
                this->transform->position.x + this->radius, 
                this->transform->position.y + this->radius
            );
            
            for(int i=0; i<this->amount; ++i) {
                this->hull[i] = 
                    AddVecs(
                        this->center,
                        Vector2D(
                            this->radius * cosf(this->fraction*i), 
                            this->radius * sinf(this->fraction*i)
                        )
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
            this->radius = this->transform->width/2 * this->transform->scale;
            setHull();
        }

        void update() override {
            setHull();
        }
};