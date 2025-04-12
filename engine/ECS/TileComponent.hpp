#pragma once
class TileComponent : public Component {
    public:
        TransformComponent *transform;
        SpriteComponent *sprite;

        SDL_FRect tileRect;
        int tileID;
        const char *path;

        TileComponent() = default;

        TileComponent(float x, float y, float w, float h, float id) {
            this->tileRect.x = x;
            this->tileRect.y = y;
            this->tileRect.w = w;
            this->tileRect.h = h;
            this->tileID = id;

            switch(this->tileID) {
                case 1:
                    this->path = "assets/tiles/dirt.png";
                    break;
                case 2:
                    this->path = "assets/tiles/mountain.png";
                    break;
                case 3:
                    this->path = "assets/tiles/water.png";
                    break;
                default:
                    this->path = "assets/tiles/grass.png";
                    break;
            }
        }

        void init() override {
            entity->addComponent<TransformComponent>(
                this->tileRect.x, this->tileRect.y,
                this->tileRect.w, this->tileRect.h,
                1
            );
            this->transform = &entity->getComponent<TransformComponent>();
            entity->addComponent<SpriteComponent>(this->path);
            this->sprite = &entity->getComponent<SpriteComponent>();
        }

};