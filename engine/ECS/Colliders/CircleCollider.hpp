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
        void setHull(std::vector<Vector2D>& h, bool screen_coord=false, float sc=1.0f) {
            float r = this->radius;
            if(screen_coord) { r *= sc; }

            this->center = Vector2D(
                this->transform->position.x + r, 
                this->transform->position.y + r
            );
            
            for(int i=0; i<this->amount; ++i) {
                h[i] = 
                    AddVecs(
                        this->center,
                        Vector2D(
                            r * cosf(this->fraction*i), 
                            r * sinf(this->fraction*i)
                        )
                    );
            }
        }

    public:
        float radius;
        Vector2D center;
        std::vector<Vector2D> world_hull = std::vector<Vector2D>(this->amount);
        std::vector<Vector2D> screen_hull = std::vector<Vector2D>(this->amount);

        TransformComponent *transform;

        CircleCollider(TransformComponent* transf) {
            this->transform = transf;
        }

        void init() override {
            this->radius = this->transform->width/2;
            setHull(this->world_hull);
            setHull(this->screen_hull, true, this->transform->scale);
        }

        void update(const float& frame_delta) override {
            setHull(this->world_hull);
            setHull(this->screen_hull, true, this->transform->scale);
        }
};