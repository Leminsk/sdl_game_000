#define SDL_MAIN_HANDLED // https://stackoverflow.com/questions/32342285/undefined-reference-to-winmain16-c-sdl-2
#include <SDL2/SDL.h>
#include "engine/Game.hpp"

Game *game = nullptr;

int main() {
    const bool FORCE_FRAMERATE = false;
    const int FPS = 60;
    const float EXPECTED_DELAY = 1000 / FPS;

    uint64_t frame_start, frame_delta;
    float delta = FORCE_FRAMERATE ? -1 : 0.0001f;

    game = new Game();
    game->init("Engine", 800, 600, false);

    while (game->running()) {
        frame_start = SDL_GetTicks64();

        game->handleEvents(delta);
        game->update(delta);
        game->render(delta);

        frame_delta = SDL_GetTicks64() - frame_start;
        delta = static_cast<float>(frame_delta)/1000.0f;

        if (FORCE_FRAMERATE && (EXPECTED_DELAY > frame_delta)) {
            SDL_Delay(EXPECTED_DELAY - frame_delta);
        }
    }

    game->clean();

    return 0;
}