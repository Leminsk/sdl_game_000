#pragma once

#include "ECS/ECS.hpp"
#include "Game.hpp"
#include "Vector2D.hpp"
#include "AudioManager.hpp"
#include "SceneTypes.hpp"
#include "Scene_utils.hpp"

class SceneLobby {
    private:
    public:
        SceneLobby() {}
        ~SceneLobby() {}

        void setScene() {}
        void handleEventsPrePoll() {}
        void handleEventsPollEvent() {}
        void handleEventsPostPoll() {}
        void update() {}
        void render() {}
        void clean() {}
};