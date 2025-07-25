#ifdef _WIN32
    #define SDL_MAIN_HANDLED // https://stackoverflow.com/questions/32342285/undefined-reference-to-winmain16-c-sdl-2
#endif

#include <chrono>
#include <random>
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

    std::random_device os_seed;
    uint32_t seed = os_seed();
    std::mt19937 generator(seed);

    game = new Game();
    game->init("Engine", 800, 600, false);
    game->RNG = &generator;
    game->SERVER_STATE_SHARE_RATE = MAX_FPS / SERVER_BROADCAST_RATE;
    game->CLIENT_PING_RATE = MAX_FPS * 3;

    int small_frame_counter = 0;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;
    int ms_passed;

    while (true) {
        end = std::chrono::steady_clock::now();
        ms_passed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if(ms_passed >= 1000) {
            game->AVERAGE_FPS = small_frame_counter / (ms_passed / 1000.0f);
            small_frame_counter = 0;
            start = std::chrono::steady_clock::now();
        }
        elapsed_time = SDL_GetTicks64();
        game->FRAME_COUNT = frame;
        
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
        ++small_frame_counter;
    }

    game->clean();

    return 0;
}