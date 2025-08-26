#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>

enum class MainColors : uint8_t {
    NONE,
    WHITE,
    BLACK,
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA
};

enum class MapColors : uint8_t {
    NONE,
    SPAWN,
    NAVIGABLE,
    IMPASSABLE,
    ROUGH,
    PLAIN
};

// drone colors
const SDL_Color COLORS_WHITE   = { 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_BLACK   = { 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_RED     = { 0xFF, 0x00, 0x00, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_GREEN   = { 0x00, 0xFF, 0x00, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_BLUE    = { 0x00, 0x00, 0xFF, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_YELLOW  = { 0xFF, 0xFF, 0x00, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_CYAN    = { 0x00, 0xFF, 0xFF, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_MAGENTA = { 0xFF, 0x00, 0xFF, SDL_ALPHA_OPAQUE };

// terrain/tile colors
const SDL_Color COLORS_SPAWN      = { 0xFF, 0xAE, 0xC9, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_NAVIGABLE  = { 0x0A, 0x05, 0x64, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_IMPASSABLE = { 0x8F, 0x8F, 0x8F, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_ROUGH      = { 0x32, 0x05, 0x0A, SDL_ALPHA_OPAQUE };
const SDL_Color COLORS_PLAIN      = { 0x7B, 0x52, 0x23, SDL_ALPHA_OPAQUE };
