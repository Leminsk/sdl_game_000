#define SDL_MAIN_HANDLED // https://stackoverflow.com/questions/32342285/undefined-reference-to-winmain16-c-sdl-2
#include <SDL2/SDL.h>
#include "engine/Game.hpp"

Game *game = nullptr;

int main() {
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    uint64_t frameStart;
    int frameTime;

    game = new Game();
    
    game->init("Engine", 800, 600, false);

    while (game->running()) {
        frameStart = SDL_GetTicks64();

        game->handleEvents();
        game->update();
        game->render();

        frameTime = SDL_GetTicks64() - frameStart;

        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    game->clean();

    return 0;
}