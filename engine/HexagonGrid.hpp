#pragma once

#include <vector>
#include "Game.hpp"
#include "Vector2D.hpp"

const int HEX_SIDE_LENGTH = Game::UNIT_SIZE;

const int HEX_RECT_TILE_WIDTH = HEX_SIDE_LENGTH<<1;
const float one_and_half_HEX_SIDE_LENGTH = 1.5f * HEX_SIDE_LENGTH;
const float      sqrt_3 = 1.73205f;
const float half_sqrt_3 = 0.86602f;
const float HEX_X_WIDTH = sqrt_3 * HEX_SIDE_LENGTH;

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
    float y = 1.5f * hex.r;
    return { x * HEX_SIDE_LENGTH, y * HEX_SIDE_LENGTH };
}

// get hex coord from a given world point
HexPos convertWorldToHex(const Vector2D& world_pos) {
    float x = world_pos.x / HEX_SIDE_LENGTH;
    float y = world_pos.y / HEX_SIDE_LENGTH;
    float fq = ((sqrt_3 * x) - y)/3.0f;
    float fr = 0.66666f * y;

    // axial cube round https://www.redblobgames.com/grids/hexagons/#rounding
    int q = std::round(fq);
    int r = std::round(fr);
    float s = -q-r;
    int q_diff = std::fabs(q - fq);
    int r_diff = std::fabs(r - fr);
    int s_diff = std::fabs(std::round(s) - s);

    if( q_diff > r_diff && q_diff > s_diff) {
        q = -r-s;
    } else if(r_diff > s_diff) {
        r = -q-s;
    }

    return HexPos{ q, r };
}

// TODO: return positions of hexagon tiles neighboring pos
std::vector<HexPos> hexNeighbors(const HexPos& hex, const int& grid_x_max, const int& grid_y_max) {
    return {};
}