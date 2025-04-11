#pragma once
#include <SDL2/SDL.h>
#include "ECS.hpp"
#include "Components.hpp"
#include "../Vector2D.hpp"
#include "../TextureManager.hpp"


// for debugging hitboxes/colliders
class Wireframe : public Component {
    public:
        SDL_FPoint* points;
        int amount;
        
        void init() override {
            if(!entity->hasComponent<HexagonCollider>()) {
                entity->addComponent<HexagonCollider>();
            }

            std::vector<Vector2D> list = entity->getComponent<HexagonCollider>().hull;

            this->amount = list.size();
            this->points = (SDL_FPoint*)malloc(sizeof(SDL_FPoint) * (amount+1));
            
            SDL_FPoint p;
            int i;
            for(i=0; i<amount; ++i) {
                p.x = list[i].x;
                p.y = list[i].y;
                this->points[i] = p;
            }
            this->points[i].x = list[0].x;
            this->points[i].y = list[0].y;
            this->amount++;
        }

        void draw() override {
            TextureManager::DrawWireframe(this->points, this->amount);
        }
};