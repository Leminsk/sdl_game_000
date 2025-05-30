#pragma once
#include <vector>
#include "Vector2D.hpp"
#include "ECS/ECS.hpp"
#include "ECS/TileTypes.hpp"
#include "ECS/Components.hpp"

bool isBlocked(float x, float y, const std::vector<Entity*>& tiles) {
    int tileID;
    Vector2D pos;
    float w;
    for(auto& t : tiles) {
        tileID = t->getComponent<TileComponent>().tileID;
        if(tileID == TILE_IMPASSABLE) {
            pos = t->getComponent<TransformComponent>().position;
            w = t->getComponent<TransformComponent>().width;
            if(
                pos.x <= x && x <= pos.x + w &&
                pos.y <= y && y <= pos.y + w
            ) {
                return true;
            }
        }
    }
    return false;
}

bool lineOfSight(const Vector2D& s, const Vector2D& s_prime, const std::vector<Entity*>& tiles) {
    float x0 = s.x;
    float y0 = s.y;
    float x1 = s_prime.x;
    float y1 = s_prime.y;
    float dy = y1 - y0;
    float dx = x1 - x0;
    float f = 0.0f;
    float sy, sx;

    if(dy < 0.0f) {
        dy = -dy;
        sy = -1;
    } else {
        sy = 1;
    }

    if(dx < 0.0f) {
        dx = -dx;
        sx = -1;
    } else {
        sx = 1;
    }

    if(dx >= dy) {
        while(x0 != x1) {
            f += dy;
            if(f >= dx) {
                if(isBlocked(x0 + ((sx - 1)/2), y0 + ((sy - 1)/2), tiles)) { 
                    return false; 
                }
                y0 += sy;
                f -= dx;
            }
            if(
                f != 0.0f && 
                isBlocked(x0 + ((sx - 1)/2), y0 + ((sy - 1)/2), tiles)
            ) { 
                return false; 
            }
            if(
                dy == 0.0f && 
                isBlocked(x0 + ((sx - 1)/2), y0, tiles) && 
                isBlocked(x0 + ((sx - 1)/2), y0 - 1, tiles)
            ) { 
                return false; 
            }
            x0 += sx;
        }
    } else {
        while(y0 != y1) {
            f += dx;
            if(f >= dy) {
                if(isBlocked(x0 + ((sx - 1)/2), y0 + ((sy - 1)/2), tiles)) { 
                    return false; 
                }
                x0 += sx;
                f -= dy;
            }
            if(
                f != 0.0f && 
                isBlocked(x0 + ((sx - 1)/2), y0 + ((sy - 1)/2), tiles)
            ) { 
                return false; 
            }
            if(
                dy == 0.0f && 
                isBlocked(x0, y0 + ((sy - 1)/2), tiles) && 
                isBlocked(x0 - 1, y0 + ((sy - 1)/2), tiles)
            ) { 
                return false; 
            }
            y0 += sy;
        }
    }
    
    return true;
}