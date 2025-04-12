#pragma once

class RectangleCollider : public Component {
    public:
        SDL_FRect collider;
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

        TransformComponent* transform;

        void init() override {
            if(!entity->hasComponent<TransformComponent>()) {
                entity->addComponent<TransformComponent>();
            }
            transform = &entity->getComponent<TransformComponent>();

            Vector2D offset = this->transform->position;

            float x = this->transform->position.x;
            float y = this->transform->position.y;
            float w = this->transform->width;
            float h = this->transform->height;
            float sc = this->transform->scale;
            
            this->center = AddVecs(Vector2D(x/2, y/2), offset).Scale(sc);
            this->hull[0] = AddVecs(Vector2D(x+w, y+h), offset).Scale(sc);
            this->hull[1] = AddVecs(Vector2D(  x, y+h), offset).Scale(sc);
            this->hull[2] = AddVecs(Vector2D(  x,   y), offset).Scale(sc);
            this->hull[3] = AddVecs(Vector2D(x+w,   y), offset).Scale(sc);
        }

        void update() override {
            this->collider.x = transform->position.x;
            this->collider.y = transform->position.y;
            this->collider.w = transform->width * transform->scale;
            this->collider.h = transform->height * transform->scale;
        }
};