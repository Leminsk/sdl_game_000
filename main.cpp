#define SDL_MAIN_HANDLED // https://stackoverflow.com/questions/32342285/undefined-reference-to-winmain16-c-sdl-2
#include <SDL2/SDL.h>
#include "engine/Game.hpp"

Game *game = nullptr;

int main() {
    uint64_t elapsed_time;
    uint64_t old_elapsed_time = 0;
    float frame_delta;

    game = new Game();
    game->init("Engine", 800, 600, false);

    while (true) {
        elapsed_time = SDL_GetTicks64();
        game->frame_delta = static_cast<float>(elapsed_time - old_elapsed_time)/1000.0f;
        old_elapsed_time = elapsed_time;

        game->handleEvents();
        if(!game->running()) { break; }

        game->update();
        game->render();
    }

    game->clean();

    return 0;
}