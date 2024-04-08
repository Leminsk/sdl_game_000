#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

class Component;
class Entity;

// using ComponentID = std::size_t; ???

// generate a new component ID
inline size_t getNewComponentID() {
    static size_t lastID = 0;
    return lastID++;
}

// get an ID for a component type
template <typename T> inline size_t getComponentTypeID() noexcept {
    static size_t typeID = getNewComponentID();
    return typeID;
}

constexpr size_t maxComponents = 32;


class Component {
    public: 
        Entity* entity;

        virtual void init() {}
        virtual void update() {}
        virtual void draw() {}

        virtual ~Component() {}
};

class Entity {
    private:
        bool active = true;
        std::vector< std::unique_ptr<Component> > components;

        std::bitset<maxComponents> componentArray;
        std::array<Component*, maxComponents> componentBitSet;

    public:
        void update() {
            for (auto& c : this->components) { c->update(); }
            for (auto& c : this->components) { c->draw(); }
        }
        void draw() {}
        bool isActive() const { return this->active; }
        void destroy() { this->active = false; }

        template <typename T> bool hasComponent() const {
            return this->componentBitSet[getComponentID<T>()];
        }

        template <typename T, typename... TArgs> T& addComponent(TArgs&&... mArgs) {
            T* component(new T(std::forward<TArgs>(mArgs)...));
            component->entity = this;
            std::unique_ptr<Component> uPtr{ component };
            components.emplace_back(std::move(uPtr));

            this->componentArray[getComponentTypeID<T>()] = c;
            this->componentBitSet[getComponentTypeID<T>()] = true;

            component->init();
            return *component;
        }

        template <typename T> T& getComponent() const {
            auto ptr(this->componentArray[getComponentTypeID<T>()]);
            return *static_cast<T*>(ptr);
        }
};

class Manager {
    private:
        std::vector<std::unique_ptr<Entity>> entities;

    public:
        void update() {
            for (auto& e : this->entities) { e->update(); }
        }
        void draw() {
            for (auto& e : this->entities) { e->draw(); }
        }
        void refresh() {
            this->entities.erase(
                std::remove_if(
                    std::begin(this->entities), 
                    std::end(this->entities), 
                    [](const std::unique_ptr<Entity> &mEntity) {
                        return !mEntity->isActive();
                    }
                ),
                std::end(this->entities)
            );
        }
        Entity& addEntity() {
            Entity *e = new Entity();
            std::unique_ptr<Entity> uPtr{ e };
            entities.emplace_back(std::move(uPtr));
            return *e;
        }
};