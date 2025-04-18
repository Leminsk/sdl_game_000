#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>

class Component;
class Entity;
class Manager;

// using ComponentID = std::size_t; ???
using Group = std::size_t;

// generate a new component ID
inline size_t getNewComponentTypeID() {
    static size_t lastID = 0u;
    return lastID++;
}

// get an ID for a component type
template <typename T> inline size_t getComponentTypeID() noexcept {
    static_assert (std::is_base_of<Component, T>::value, "");
    static size_t typeID = getNewComponentTypeID();
    return typeID;
}

constexpr size_t maxComponents = 32;
constexpr size_t maxGroups = 32; // render or collision layers


// A Component is a functionality structure
class Component {
    public: 
        Entity* entity;

        virtual void init() {}
        virtual void update(const float& frame_delta) {}
        virtual void draw() {}

        virtual ~Component() {}
};

// An Entity holds many components together in a cohesive manner.
class Entity {
    private:
        Manager& manager;
        bool active = true;
        std::vector< std::unique_ptr<Component> > components;

        std::array<Component*, maxComponents> componentArray;
        std::bitset<maxComponents> componentBitSet;
        std::bitset<maxGroups> groupBitSet;
        

    public:
        Entity(Manager& mManager) : manager(mManager) {}

        void update(const float& frame_delta) {
            for (auto& c : this->components) { c->update(frame_delta); }
        }
        void draw() {
            for (auto& c : this->components) { c->draw(); }
        }
        bool isActive() const { return this->active; }
        void destroy() { this->active = false; }

        bool hasGroup(Group mGroup) {
            return groupBitSet[mGroup];
        }

        void addGroup(Group mGroup);
        void delGroup(Group mGroup) {
            this->groupBitSet[mGroup] = false;
        }

        template <typename T> bool hasComponent() const {
            return this->componentBitSet[getComponentTypeID<T>()];
        }

        // add Component to Entity
        template <typename T, typename... TArgs> T& addComponent(TArgs&&... mArgs) {
            T* component(new T(std::forward<TArgs>(mArgs)...));
            component->entity = this;
            std::unique_ptr<Component> uPtr{ component };
            components.push_back(std::move(uPtr));

            this->componentArray[getComponentTypeID<T>()] = component;
            this->componentBitSet[getComponentTypeID<T>()] = true;

            component->init();
            return *component;
        }

        // access a Component belonging to this Entity
        template <typename T> T& getComponent() const {
            auto ptr(this->componentArray[getComponentTypeID<T>()]);
            return *static_cast<T*>(ptr);
        }
};

// A Manager holds many Entities... because reasons?
class Manager {
    private:
        std::vector< std::unique_ptr<Entity> > entities;
        std::array< std::vector<Entity*>, maxGroups > groupedEntities;

    public:
        void update(const float& frame_delta) {
            for (auto& e : this->entities) { e->update(frame_delta); }
        }
        void draw() {
            for (auto& e : this->entities) { e->draw(); }
        }
        void refresh() {
            for(auto i(0u); i < maxGroups; ++i) {
                auto& v(this->groupedEntities[i]);
                v.erase(
                    std::remove_if(
                        std::begin(v),
                        std::end(v),
                        [i](Entity* mEntity) {
                            return !mEntity->isActive() || !mEntity->hasGroup(i);
                        }
                    ),
                    std::end(v)
                );
            }

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

        void AddToGroup(Entity* mEntity, Group mGroup) {
            this->groupedEntities[mGroup].push_back(mEntity);
        }

        std::vector<Entity*>& getGroup(Group mGroup) {
            return this->groupedEntities[mGroup];
        }

        Entity& addEntity() {
            Entity *e = new Entity(*this);
            std::unique_ptr<Entity> uPtr{ e };
            this->entities.push_back(std::move(uPtr));
            return *e;
        }
};