#pragma once

#include "../Camera.hpp"
#include "ECS.hpp"
#include "Colliders/ColliderTypes.hpp"
#include "Colliders/Collider.hpp"
#include "../TextureManager.hpp"
#include "../Vector2D.hpp"


// for debugging hitboxes/colliders
class Wireframe : public Component {
    private:
        SDL_FPoint* draw_points;

        void refreshHull() {
            switch(t) {
                case ColliderType::HEXAGON:   this->hull = entity->getComponent<  HexagonCollider>().hull; break;
                case ColliderType::RECTANGLE: this->hull = entity->getComponent<RectangleCollider>().hull; break;
                case ColliderType::CIRCLE:    this->hull = entity->getComponent<   CircleCollider>().hull; break;
                default:
                    this->hull = {};
            }
        }

        void refreshDrawPoints() {            
            SDL_FPoint p;
            Vector2D screen_pos;
            int i;
            for(i=0; i<amount; ++i) {
                screen_pos = convertWorldToScreen(this->hull[i]);
                p.x = screen_pos.x;
                p.y = screen_pos.y;
                this->draw_points[i] = p;
            }
            // close the circuit
            this->draw_points[i] = this->draw_points[0];
        }

    public:
        ColliderType t;
        // purely screen coordinates
        std::vector<Vector2D> hull;
        int amount;
        SDL_Color color = { 255, 0, 0, SDL_ALPHA_OPAQUE };

        Wireframe() {}
        Wireframe(Uint8 r, Uint8 g, Uint8 b) {
            this->color.r = r;
            this->color.g = g;
            this->color.b = b;
        }

        ~Wireframe() {
            free(draw_points);
        }
        
        void init() override {
            if(!entity->hasComponent<Collider>()) {
                return;
            }

            this->t = entity->getComponent<Collider>().type;

            refreshHull();
            this->amount = this->hull.size();
            this->draw_points = (SDL_FPoint*)malloc(sizeof(SDL_FPoint) * (amount+1));
        }

        void update() override {
            
        }

        void draw() override {
            refreshHull();
            refreshDrawPoints();
            TextureManager::DrawWireframe(this->draw_points, this->amount+1, this->color);
        }
};