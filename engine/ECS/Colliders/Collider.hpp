#pragma once

#include <string>
#include "../../Game.hpp"
#include "../../Vector2D.hpp"
#include "../ECS.hpp"
#include "../TransformComponent.hpp"
#include "ColliderTypes.hpp"
#include "CircleCollider.hpp"
#include "HexagonCollider.hpp"
#include "RectangleCollider.hpp"



class Collider : public Component {
    public:
        ColliderType type;

        TransformComponent* transform;

        Collider() {}

        Collider(ColliderType t) {
            this->type = t;
        }

        Vector2D getCenter() {
            switch(this->type) {
                case ColliderType::CIRCLE:    return entity->getComponent<   CircleCollider>().center;
                case ColliderType::HEXAGON:   return entity->getComponent<  HexagonCollider>().center;
                case ColliderType::RECTANGLE: return entity->getComponent<RectangleCollider>().center;
                default: return Vector2D(0,0);
            }
        }

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            transform = &entity->getComponent<TransformComponent>();

            switch(this->type) {
                case ColliderType::CIRCLE:    entity->addComponent<   CircleCollider>(transform); break;
                case ColliderType::HEXAGON:   entity->addComponent<  HexagonCollider>(transform); break;
                case ColliderType::RECTANGLE: entity->addComponent<RectangleCollider>(transform); break;
                default: break;
            }
        }

        void update() override {
            switch(this->type) {
                case ColliderType::CIRCLE:    entity->getComponent<   CircleCollider>().update(); break;
                case ColliderType::HEXAGON:   entity->getComponent<  HexagonCollider>().update(); break;
                case ColliderType::RECTANGLE: entity->getComponent<RectangleCollider>().update(); break;
                default: break;
            }
        }
};