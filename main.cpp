#define SDL_MAIN_HANDLED // https://stackoverflow.com/questions/32342285/undefined-reference-to-winmain16-c-sdl-2
#include <SDL2/SDL.h>
#include <SDL2/SDL_version.h>
#include "engine/Game.hpp"

Game *game = nullptr;

int main() {
    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    SDL_Log("COMPILED against SDL version %u.%u.%u\n", compiled.major, compiled.minor, compiled.patch);
    SDL_Log("  LINKED against SDL version %u.%u.%u\n",   linked.major,   linked.minor,   linked.patch);

    const bool LIMIT_FPS = true;
    const int MAX_FPS = 60;
    const int MAX_FRAME_DELAY = 1000.0f / MAX_FPS;

    const int SERVER_BROADCAST_RATE = 20;
    
    uint64_t time_spent_on_frame;
    uint64_t elapsed_time;
    uint64_t old_elapsed_time = 0;
    uint64_t frame = 0;

    game = new Game();
    game->init("Engine", 800, 600, false);
    game->SERVER_STATE_SHARE_RATE = MAX_FPS / SERVER_BROADCAST_RATE;
    game->CLIENT_PING_RATE = MAX_FPS * 3;

    while (true) {
        elapsed_time = SDL_GetTicks64();
        game->FRAME_COUNT = frame;
        game->AVERAGE_FPS = frame / (elapsed_time / 1000.0f);
        game->FRAME_DELTA = static_cast<float>(elapsed_time - old_elapsed_time)/1000.0f;
        old_elapsed_time = elapsed_time;

        game->handleEvents();
        if(!game->running()) { break; }

        game->update();
        game->render();

        if(LIMIT_FPS) {
            time_spent_on_frame = SDL_GetTicks64() - elapsed_time;

            if(time_spent_on_frame < MAX_FRAME_DELAY) {
                SDL_Delay(MAX_FRAME_DELAY - time_spent_on_frame);
            }
        }
        ++frame;
    }

    game->clean();

    return 0;
}