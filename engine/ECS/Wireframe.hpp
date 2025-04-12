#pragma once

#include "ECS.hpp"
#include "Colliders/ColliderTypes.hpp"
#include "Colliders/Collider.hpp"
#include "TransformComponent.hpp"
#include "../TextureManager.hpp"
#include "../Vector2D.hpp"


// for debugging hitboxes/colliders
class Wireframe : public Component {
    private:
        TransformComponent *transform;
    public:
        SDL_FPoint* points;
        Uint8 color[3] = { 255, 0, 0 };
        int amount;

        Wireframe() {}
        Wireframe(Uint8 r, Uint8 g, Uint8 b) {
            this->color[0] = r;
            this->color[1] = g;
            this->color[2] = b;
        }
        
        void init() override {
            this->transform = &entity->getComponent<TransformComponent>();

            if(!entity->hasComponent<Collider>()) {
                return;
            }

            collider_type t = entity->getComponent<Collider>().type;


            std::vector<Vector2D> hull;

            switch(t) {
                case COLLIDER_HEXAGON:
                    hull = entity->getComponent<HexagonCollider>().hull;
                    break;
                case COLLIDER_RECTANGLE:
                    hull = entity->getComponent<RectangleCollider>().hull;
                    break;
                default:
                    hull = {};
            }

            this->amount = hull.size();
            this->points = (SDL_FPoint*)malloc(sizeof(SDL_FPoint) * (amount+1));
            
            SDL_FPoint p;
            int i;
            for(i=0; i<amount; ++i) {
                p.x = hull[i].x;
                p.y = hull[i].y;
                this->points[i] = p;
            }
            // close the circuit
            this->points[i].x = hull[0].x;
            this->points[i].y = hull[0].y;
            this->amount++;
        }

        void update() override {
            for(int i=0; i<amount; ++i) {
                this->points[i].x += this->transform->velocity.x * this->transform->speed;
                this->points[i].y += this->transform->velocity.y * this->transform->speed;
            }
        }

        void draw() override {
            TextureManager::DrawWireframe(this->points, this->amount, this->color);
        }
};