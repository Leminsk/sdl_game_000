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
        TransformComponent *camera_transform;

        void refreshHull() {
            switch(t) {
                case COLLIDER_HEXAGON:   this->hull = entity->getComponent<  HexagonCollider>().hull; break;
                case COLLIDER_RECTANGLE: this->hull = entity->getComponent<RectangleCollider>().hull; break;
                case COLLIDER_CIRCLE:    this->hull = entity->getComponent<   CircleCollider>().hull; break;
                default:
                    this->hull = {};
            }
        }

        void refreshDrawPoints() {            
            SDL_FPoint p;
            Vector2D screen_pos;
            camera_transform = &Game::camera.getComponent<TransformComponent>();
            int i;
            for(i=0; i<amount; ++i) {
                screen_pos = applyZoom(
                    *camera_transform,
                    convertWorldToScreen(camera_transform->position, this->hull[i])
                );
                p.x = screen_pos.x;
                p.y = screen_pos.y;
                this->draw_points[i] = p;
            }
            // close the circuit
            this->draw_points[i] = this->draw_points[0];
        }

    public:
        collider_type t;
        // purely screen coordinates
        std::vector<Vector2D> hull;
        int amount;
        Uint8 color[3] = { 255, 0, 0 };      

        Wireframe() {}
        Wireframe(Uint8 r, Uint8 g, Uint8 b) {
            this->color[0] = r;
            this->color[1] = g;
            this->color[2] = b;
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
            refreshDrawPoints();
        }

        void update() override {
            refreshHull();
            refreshDrawPoints();
        }

        void draw() override {
            TextureManager::DrawWireframe(this->draw_points, this->amount+1, this->color);
        }
};