#include "AudioManager.hpp"

Mix_Music* AudioManager::LoadMusic(const char* file_path) {
    Mix_Music* music = Mix_LoadMUS(file_path);
    if(music == NULL) {
        SDL_Log("Failed to load music. SDL_mixer error: %s\n", Mix_GetError());
        return NULL;
    }
    return music;
}

Mix_Chunk* AudioManager::LoadSound(const char* file_path) {
    Mix_Chunk* sound = Mix_LoadWAV(file_path);
    if(sound == NULL) {
        SDL_Log("Failed to load sound. SDL_mixer error: %s\n", Mix_GetError());
        return NULL;
    }
    return sound;
}