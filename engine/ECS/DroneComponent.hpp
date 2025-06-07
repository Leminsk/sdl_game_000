#pragma once
#include <string>
#include "../Vector2D.hpp"
#include "ECS.hpp"
#include "TransformComponent.hpp"
#include "SpriteComponent.hpp"
#include "Colliders/Collider.hpp"
#include "../theta_star.hpp"


// "drone" as in a combat unit/soldier
class DroneComponent : public Component {
    private:
        Vector2D starting_position;
        SDL_Texture *sprite_texture;

    public:
        TransformComponent *transform;
        SpriteComponent *sprite;
        Collider *collider;

        bool selected = false;
        std::vector<Vector2D> path = {};
        std::vector<int> visited_indices = {};
        int current_path_index = -1;
        int last_path_index;
        float radius;
        float diameter;

        DroneComponent(const Vector2D& starting_position, float diameter, SDL_Texture* sprite_texture) {
            this->starting_position = starting_position;
            this->sprite_texture = sprite_texture;
            this->diameter = diameter;
            this->radius = diameter/2;
            ++Game::UNIT_COUNTER;
        }

        ~DroneComponent() {}

        // get center (x,y)
        Vector2D getPosition() {
            return this->collider->getCenter();
        }

        void moveToPoint(const Vector2D& destination) {
            this->path = find_path(getPosition(), destination);
            last_path_index = path.size() - 1;
            this->current_path_index = last_path_index;
            this->visited_indices = {};
            this->transform->velocity = Vector2D(0,0);
        }

        void init() override {
            entity->addComponent<TransformComponent>(this->starting_position.x, this->starting_position.y, this->diameter, this->diameter, 1.0f);
            entity->addComponent<SpriteComponent>(this->sprite_texture);
            entity->addComponent<Collider>("Drone_"+std::to_string(Game::UNIT_COUNTER), COLLIDER_CIRCLE);
            this->transform = &entity->getComponent<TransformComponent>();
            this->sprite = &entity->getComponent<SpriteComponent>();
            this->collider = &entity->getComponent<Collider>();
        }

        void preUpdate() override {
            // no more points to follow
            if(this->current_path_index == -1 && path.size() > 0 && visited_indices.size() == path.size()) {
                this->path = {};
                this->visited_indices = {};
                this->current_path_index = -1;
                this->transform->velocity = Vector2D(0,0);
            }

            // has reached new point. Leaving these 2* because of the offset when sliding over blocked tiles. Also it kinda makes the trajectory "look smoother"
            if(
                this->current_path_index >= 0 && 
                Distance(getPosition(), path[this->current_path_index]) <= 2*(this->radius * this->radius)
            ) {
                float d;
                std::vector<int> candidate_indices = {};

                // mark every node inside the circle as visited
                for(int i=last_path_index; i>=0; --i) {
                    if(std::find(visited_indices.begin(), visited_indices.end(), i) == visited_indices.end()) {
                        d = Distance(getPosition(), path[i]);
                        if(d <= 2*(this->radius * this->radius)) {
                            visited_indices.push_back(i);
                            candidate_indices.push_back(i);
                        }
                    }
                }
                
                int min_idx = last_path_index;
                // get the min index of the nodes inside the circle -> the node which is farther along the path
                for(int i=0; i<candidate_indices.size(); ++i) {
                    if(candidate_indices[i] < min_idx) { min_idx = candidate_indices[i];}
                }
                // the next one should thus be outside the circle and should serve as the next destination
                this->current_path_index = min_idx - 1;
                this->transform->velocity = (this->path[current_path_index] - getPosition()).Normalize() * 2.0f;
            }
        }
        
};