#pragma once

#include <vector>
#include <SDL2/SDL.h>
#include "Game.hpp"
#include "Vector2D.hpp"
#include "ECS/Colliders/Collision.hpp"

const int HEX_SIDE_LENGTH = Game::UNIT_SIZE;

const int HEX_RECT_TILE_WIDTH = HEX_SIDE_LENGTH<<1;
const float one_and_half_HEX_SIDE_LENGTH = 1.5f * HEX_SIDE_LENGTH;
const float      sqrt_3 = 1.73205f;
const float half_sqrt_3 = 0.86602f;
const float HEX_X_WIDTH = sqrt_3 * HEX_SIDE_LENGTH;

const float cos_30 = 0.8660254f;

/*
https://www.redblobgames.com/grids/hexagons/#coordinates-axial
Hexagon tiles are arranged axial with the basis q = {sqrt_3, 0} vs r = {sqrt_3/2, 3/2}
*/ 
struct HexPos {
    int q;
    int r;
};

// get world coord of the center of a hex grid tile
Vector2D convertHexToWorld(const HexPos& hex) {
    float x = (sqrt_3 * hex.q) + (half_sqrt_3 * hex.r);
    float y =                             1.5f * hex.r;
    return { x * HEX_SIDE_LENGTH, y * HEX_SIDE_LENGTH };
}

// get hex coord from a given world point
HexPos convertWorldToHex(const Vector2D& world_pos) {
    float x = world_pos.x / HEX_SIDE_LENGTH;
    float y = world_pos.y / HEX_SIDE_LENGTH;
    float fq = ((sqrt_3 * x) - y)/3.0f;
    float fr = 0.6666667f * y;

    // axial cube round https://www.redblobgames.com/grids/hexagons/#rounding
    // axial_to_cube()
    float fs = -fq-fr;

    // cube_round()
    int q = std::round(fq);
    int r = std::round(fr);
    int s = std::round(fs);

    float q_diff = std::fabs(q - fq);
    float r_diff = std::fabs(r - fr);
    float s_diff = std::fabs(s - fs);

    if( q_diff > r_diff && q_diff > s_diff) {
        q = -r-s;
    } else if(r_diff > s_diff) {
        r = -q-s;
    } else {
        s = -q-r;
    }
    // cube_to_axial()
    return HexPos{ q, r };
}

// get Vector2D world positions of the six points that would compose the hexagon in the grid
std::vector<Vector2D> getPointsFromHexPos(const HexPos& hex) {
    Vector2D center = convertHexToWorld(hex);
    const float x_delta = HEX_SIDE_LENGTH * cos_30;
    const float y_delta = HEX_SIDE_LENGTH;
    const float y_delta_small = HEX_SIDE_LENGTH>>1;

    std::vector<Vector2D> points; points.reserve(6);
    points.push_back({ center.x + x_delta, center.y + y_delta_small });
    points.push_back({ center.x          , center.y + y_delta       });
    points.push_back({ center.x - x_delta, center.y + y_delta_small });
    points.push_back({ center.x - x_delta, center.y - y_delta_small });
    points.push_back({ center.x          , center.y - y_delta       });
    points.push_back({ center.x + x_delta, center.y - y_delta_small });
    return points;
}

// return positions of hexagon tiles neighboring a given hex
std::vector<HexPos> hexNeighbors(const HexPos& hex, const SDL_FRect& map_hex_rect) {
    std::vector<HexPos> candidates = {
        { hex.q + 1, hex.r     }, // →
        { hex.q    , hex.r + 1 }, // ↘
        { hex.q - 1, hex.r + 1 }, // ↙
        { hex.q - 1, hex.r     }, // ←
        { hex.q    , hex.r - 1 }, // ↖
        { hex.q + 1, hex.r - 1 }, // ↗
    };

    std::vector<HexPos> neighbors = {};
    for(HexPos& c : candidates) {
        Vector2D pos = convertHexToWorld(c);
        // neighbors must be inside the map
        if(Collision::pointInRect(pos.x, pos.y, map_hex_rect.x, map_hex_rect.y, map_hex_rect.w, map_hex_rect.h)) {
            std::cout << "neighbor hex:" << " { " << c.q << " , " << c.r << " }\n";
            neighbors.push_back(c);
        }
    }

    std::cout << "ref hex:" << " { " << hex.q << ',' << hex.r << "}\n";

    return neighbors;
}