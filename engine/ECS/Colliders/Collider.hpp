#pragma once
#include "../Components.hpp"
#include "../../Vector2D.hpp"

#include "ColliderTypes.hpp"

#include "CircleCollider.hpp"
#include "HexagonCollider.hpp"
#include "RectangleCollider.hpp"



class Collider : public Component {
    public:
        collider_type type;

        TransformComponent* transform;

        Collider(collider_type t) {
            this->type = t;
        }

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            transform = &entity->getComponent<TransformComponent>();

            switch(this->type) {
                case COLLIDER_CIRCLE:    entity->addComponent<   CircleCollider>(); break;
                case COLLIDER_HEXAGON:   entity->addComponent<  HexagonCollider>(); break;
                case COLLIDER_RECTANGLE: entity->addComponent<RectangleCollider>(); break;
                default: break;
            }

            Game::colliders.push_back(this);
        }

        void update() override {
            switch(this->type) {
                case COLLIDER_CIRCLE:    entity->getComponent<   CircleCollider>().update(); break;
                case COLLIDER_HEXAGON:   entity->getComponent<  HexagonCollider>().update(); break;
                case COLLIDER_RECTANGLE: entity->getComponent<RectangleCollider>().update(); break;
                default: break;
            }
        }
};