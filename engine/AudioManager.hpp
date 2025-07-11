#pragma once

#include "Game.hpp"

class AudioManager {
    public:
        static Mix_Music* LoadMusic(const char* file_path);
        static Mix_Chunk* LoadSound(const char* file_path);
};