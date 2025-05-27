#pragma once

#include "Collider.hpp"
#include "../ECS.hpp"
#include "../TransformComponent.hpp"
#include "../../Vector2D.hpp"

class RectangleCollider : public Component {        
    public:
        float x, y, w, h;
        float sc = 1.0f;
        Vector2D center;
        std::vector<Vector2D> hull = std::vector<Vector2D>(4);
        /* not to scale
        2      3
         +----+
         | c. |
         |  ^ |
         +----+
        1      0
        */

       /* used if aligned in a grid to deal with corners in collisions
        bits correspond to this layout (7=MSB to 0=LSB)
        0, 1, 2,
        3, x  4,
        5, 6, 7,
        where x is this RectangleCollider, and the bits indices are its neighbors
       */
        uint8_t adjacent_rectangles = 0x00;

        TransformComponent* transform;

        void setHull() {
            this->center = Vector2D(x + (w/2), y + (h/2));
            this->hull[0] = Vector2D(x + (w*sc), y + (h*sc));
            this->hull[1] = Vector2D(         x, y + (h*sc));
            this->hull[2] = Vector2D(         x,          y);
            this->hull[3] = Vector2D(x + (w*sc),          y);
        }

        RectangleCollider() {}

        RectangleCollider(TransformComponent* transf) {
            this->transform = transf;
        }

        void init() override {
            x = this->transform->position.x;
            y = this->transform->position.y;
            w = this->transform->width;
            h = this->transform->height;
        }

        void update() override {
            x = this->transform->position.x;
            y = this->transform->position.y;
            setHull();
        }

        void draw() override {
            x = this->transform->position.x;
            y = this->transform->position.y;
            setHull();
        }
};