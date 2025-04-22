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
        collider_type type;
        std::string identifier;

        TransformComponent* transform;

        Collider(const char* id, collider_type t, std::vector<Collider*>& colliders) {
            this->identifier = id;
            this->type = t;
            colliders.push_back(this);
        }

        Vector2D getCenter() {
            switch(this->type) {
                case COLLIDER_CIRCLE:    return entity->getComponent<   CircleCollider>().center;
                case COLLIDER_HEXAGON:   return entity->getComponent<  HexagonCollider>().center;
                case COLLIDER_RECTANGLE: return entity->getComponent<RectangleCollider>().center;
                default: return Vector2D(0,0);
            }
        }

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            transform = &entity->getComponent<TransformComponent>();

            switch(this->type) {
                case COLLIDER_CIRCLE:    entity->addComponent<   CircleCollider>(transform); break;
                case COLLIDER_HEXAGON:   entity->addComponent<  HexagonCollider>(transform); break;
                case COLLIDER_RECTANGLE: entity->addComponent<RectangleCollider>(transform); break;
                default: break;
            }
        }

        void update(const float& frame_delta) override {
            switch(this->type) {
                case COLLIDER_CIRCLE:    entity->getComponent<   CircleCollider>().update(frame_delta); break;
                case COLLIDER_HEXAGON:   entity->getComponent<  HexagonCollider>().update(frame_delta); break;
                case COLLIDER_RECTANGLE: entity->getComponent<RectangleCollider>().update(frame_delta); break;
                default: break;
            }
        }
};