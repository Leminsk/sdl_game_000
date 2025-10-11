#ifdef _WIN32
    #define SDL_MAIN_HANDLED // https://stackoverflow.com/questions/32342285/undefined-reference-to-winmain16-c-sdl-2
#endif

#include <chrono>
#include <random>
#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_version.h>
#include "engine/Game.hpp"
#include "engine/utils.hpp"
#include "engine/json.hpp"

using json = nlohmann::json;

Game *game = nullptr;

// return true if config is properly formatted with the required fields and if their values are viable
bool isValidConfigJson(const json& json_data, std::vector<std::string>& error_messages) {
    bool valid = true;
    try {
        if(static_cast<int>(json_data["SCREEN_WIDTH"]) < 800) {
            error_messages.push_back("SCREEN_WIDTH cannot be less than 800.");
            valid = false;
        }
    } catch (...) {
        error_messages.push_back("SCREEN_WIDTH must be a number (without quotation marks).");
        valid = false;
    }
    
    try {
        if(static_cast<int>(json_data["SCREEN_HEIGHT"]) < 600) {
            error_messages.push_back("SCREEN_HEIGHT cannot be less than 600.");
            valid = false;
        }
    } catch (...) {
        error_messages.push_back("SCREEN_HEIGHT must be a number (without quotation marks).");
        valid = false;
    }


    if(json_data["FULLSCREEN"] != true && json_data["FULLSCREEN"] != false) {
        error_messages.push_back("FULLSCREEN can only be either true or false.");
        valid = false;
    }
    
    try {
        if(static_cast<int>(json_data["FRAME_RATE"]) < 30) {
            error_messages.push_back("FRAME_RATE cannot be less than 30.");
            valid = false;
        }
    } catch(...) {
        error_messages.push_back("FRAME_RATE must be a number (without quotation marks).");
        valid = false;
    }

    return valid;
}

int main() {
    SDL_version compiled, linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);
    SDL_Log("COMPILED against SDL version %u.%u.%u\n", compiled.major, compiled.minor, compiled.patch);
    SDL_Log("  LINKED against SDL version %u.%u.%u\n",   linked.major,   linked.minor,   linked.patch);

    std::vector<std::string> json_files = getFileNamesInDirectory(".", "JSON");
    bool has_config = false;
    for(std::string& name : json_files) {
        if(name == "config") { has_config = true; break; }
    }
    json config_data;
    if(!has_config) {
        config_data = {
            {"SCREEN_WIDTH", 1280},
            {"SCREEN_HEIGHT", 720},
            {"FULLSCREEN", false},
            {"FRAME_RATE", 60},
        };
        std::ofstream o("config.json");
        o << config_data.dump(4);
        o.close();
    } else {
        std::ifstream f("config.json");
        if(!f.is_open()) { std::cerr << "Error: could not open config.json\n"; return 3; }

        try {
            config_data = json::parse(f);
            f.close();
        } catch(const json::parse_error& e) {
            f.close();
            std::cerr << "Invalid config.json: " << e.what() << '\n';
            return 2;
        }
    }
    std::vector<std::string> error_messages = {};
    if(!isValidConfigJson(config_data, error_messages)) {
        for(std::string& error : error_messages) {
            std::cerr << "Invalid config file: " << error << '\n';
        }
        return 1;
    }
    
    uint64_t time_spent_on_frame;
    uint64_t elapsed_time;
    uint64_t old_elapsed_time = 0;
    uint64_t frame = 0;

    std::random_device os_seed;
    uint32_t seed = os_seed();
    std::mt19937 generator(seed);

    game = new Game();
    game->init("Bétula Engine", config_data["SCREEN_WIDTH"], config_data["SCREEN_HEIGHT"], config_data["FULLSCREEN"], config_data["FRAME_RATE"], 20, &generator);

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

        ++frame;
        ++small_frame_counter;

        // if(LIMIT_FPS) {
        time_spent_on_frame = SDL_GetTicks64() - elapsed_time;
        if(time_spent_on_frame < game->MAX_FRAME_DELAY) {
            SDL_Delay(game->MAX_FRAME_DELAY - time_spent_on_frame);
        }
        // }
    }

    game->clean();
    delete game;

    return 0;
}