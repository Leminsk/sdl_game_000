#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <bitset>
#include <array>
#include <SDL2/SDL.h>

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
        virtual void preUpdate() {}
        virtual void update() {}
        virtual void draw() {}

        virtual ~Component() {}
};

// An Entity holds many components together in a cohesive manner.
class Entity {
    private:
        std::string identifier;
        Manager& manager;
        bool active = true;
        std::vector< std::unique_ptr<Component> > components;

        std::array<Component*, maxComponents> componentArray;
        std::bitset<maxComponents> componentBitSet;
        std::bitset<maxGroups> groupBitSet;
        

    public:
        Entity(Manager& mManager, std::string id=NULL) : manager(mManager) {
            this->identifier = id;
        }
        ~Entity() {}

        void preUpdate() {
            for (auto& c : this->components) { c->preUpdate(); }
        }
        void update() {
            for (auto& c : this->components) { c->update(); }
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

        std::string getIdentifier() {
            return this->identifier;
        }
};

// A Manager holds many Entities. Mostly a helper class.
class Manager {
    private:
        std::vector< std::unique_ptr<Entity> > entities;
        std::array< std::vector<Entity*>, maxGroups > groupedEntities;

    public:
        Manager() {}
        ~Manager() {
            for(auto& e : this->entities) { delete &e; }
        }
        void preUpdate() {
            for (auto& e : this->entities) { e->preUpdate(); }
        }
        void update() {
            for (auto& e : this->entities) { e->update(); }
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

        Entity& addEntity(std::string id) {
            Entity *e = new Entity(*this, id);
            std::unique_ptr<Entity> uPtr{ e };
            this->entities.push_back(std::move(uPtr));
            return *e;
        }

        Entity* getEntity(std::string id) {
            for(int i=0; this->entities.size(); ++i) {
                if(this->entities[i]->getIdentifier() == id) {
                    return &(*this->entities[i]);
                }
            }
            return nullptr;
        }

        Entity* getEntityFromGroup(std::string id, Group mGroup) {
            std::vector<Entity*>& group = this->groupedEntities[mGroup];
            for(int i=0; group.size(); ++i) {
                if(group[i]->getIdentifier() == id) {
                    return group[i];
                }
            }
            return nullptr;
        }
};