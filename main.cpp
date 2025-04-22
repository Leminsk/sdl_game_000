#define SDL_MAIN_HANDLED // https://stackoverflow.com/questions/32342285/undefined-reference-to-winmain16-c-sdl-2
#include <SDL2/SDL.h>
#include "engine/Game.hpp"

Game *game = nullptr;

int main() {
    const bool FORCE_FRAMERATE = true;
    const int FPS = 60;
    const float EXPECTED_DELAY = 1000.0f / FPS;

    uint64_t frame_start, frame_delta;
    float delta_s = -1.0f;

    game = new Game();
    game->init("Engine", 800, 600, false);

    while (true) {
        frame_start = SDL_GetTicks64();

        game->handleEvents(delta_s);
        if(!game->running()) { break; }
        
        game->update(delta_s);
        game->render(delta_s);

        frame_delta = SDL_GetTicks64() - frame_start;
        

        if (FORCE_FRAMERATE && (EXPECTED_DELAY > frame_delta)) {
            SDL_Delay(EXPECTED_DELAY - frame_delta);
        } else {
            delta_s = static_cast<float>(frame_delta)/1000.0f;
        }
    }

    game->clean();

    return 0;
}