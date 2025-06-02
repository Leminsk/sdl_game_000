#pragma once
#include <sstream>
#include <unordered_map>
#include <vector>
#include <limits>
#include <chrono>
#include <queue>
#include <unordered_set>
#include "Vector2D.hpp"
#include "ECS/ECS.hpp"
#include "ECS/TileTypes.hpp"
#include "ECS/Components.hpp"

// https://en.wikipedia.org/wiki/Theta*
// https://idm-lab.org/bib/abstracts/papers/jair10b.pdf

void printVec(const std::vector<Vector2D>& v) {
    std::cout << "{\n";
    for(auto& n : v) { std::cout << n << ' '; }
    std::cout << "\n}\n";
}

struct Vec2Hash {
    std::size_t operator()(const Vector2D& v) const {
        return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
    }
};


struct CompareByFScore {
    const std::unordered_map<Vector2D, float, Vec2Hash>& fscore;
    CompareByFScore(const std::unordered_map<Vector2D, float, Vec2Hash>& fs) : fscore(fs) {}
    bool operator()(const Vector2D& a, const Vector2D& b) const {
        return fscore.at(a) > fscore.at(b);
    }
};

bool isBlocked(float x, float y, const std::vector<Entity*>& blocked_tiles) {
    int tileID;
    Vector2D pos;
    float w;
    for(auto& t : blocked_tiles) {
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

// I think I screwed this one up; might need to ditch it
// bool lineOfSight(const Vector2D& a, const Vector2D& b, const std::unordered_map<std::pair<int, int>, bool, PairHash>& blocked_tiles) {
//     float x0 = static_cast<int>(std::floor(a.x));
//     float y0 = static_cast<int>(std::floor(a.y));
//     float x1 = static_cast<int>(std::floor(b.x));
//     float y1 = static_cast<int>(std::floor(b.y));
//     float dy = -std::abs(y1 - y0);
//     float dx = std::abs(x1 - x0);
//     float f = 0.0f;
//     float sy = -1;
//     float sx = -1;
//     if(x0 < x1) { sx = 1; }
//     if(y0 < y1) { sy = 1; }

//     float e = dx + dy;
//     float e2;
//     while(true) {
//         if(isBlocked(x0, y0, blocked_tiles)) { return false; }
//         if(x0 != x1 && y0 != y1) {
//             if(isBlocked(x0 + sx, y0, blocked_tiles) || isBlocked(x0, y0 + sy, blocked_tiles)) { 
//                 return false; 
//             }
//         }
//         if(x0 == x1 && y0 == y1) { return true; }
//         e2 = 2*e;
//         if(e2 >= dy) {
//             e += dy;
//             x0 += sx;
//         }
//         if(e2 <= dx) {
//             e += dx;
//             y0 += sy;
//         }
//     }
// }


float cost(const Vector2D& a, const Vector2D& b) {
    return Distance(a, b);
}

// https://idm-lab.org/bib/abstracts/papers/jair10b.pdf -> page 26 Algorithm 5
// leaving this as is in case I change my mind
float heuristic(const Vector2D& p, const Vector2D& destination) {
    return cost(p, destination);
}


// too expensive, might ditch it later
// void update_vertex_theta(
//     const Vector2D& s, const Vector2D& neighbor, 
//     std::unordered_map<Vector2D, Vector2D, Vec2Hash>& parent, 
//     std::unordered_map<Vector2D, float, Vec2Hash>& gscore, 
//     std::unordered_map<Vector2D, float, Vec2Hash>& fscore, 
//     const std::unordered_map<std::pair<int, int>, bool, PairHash>& blocked_tiles, 
//     std::priority_queue<Vector2D, std::vector<Vector2D>, CompareByFScore>& open_queue,
//     std::unordered_set<Vector2D, Vec2Hash>& open_set, 
//     const Vector2D& destination
// ) {
//     Vector2D s_parent = parent[s];

//     if(lineOfSight(s_parent, neighbor, blocked_tiles)) {
//         if(gscore[s_parent] + cost(s_parent, neighbor) < gscore[neighbor]) {
//             gscore[neighbor] = gscore[s] + cost(s_parent, neighbor);
//             parent[neighbor] = s_parent;
//             fscore[neighbor] = gscore[neighbor] + heuristic(neighbor, destination);
//             if(open_set.find(neighbor) == open_set.end()) {
//                 open_set.insert(neighbor);
//             }
//             open_queue.push(neighbor);
//         }
//     } else {
//         if(gscore[s] + cost(s, neighbor) < gscore[neighbor]) {
//             gscore[neighbor] = gscore[s] + cost(s, neighbor);
//             parent[neighbor] = s;
//             fscore[neighbor] = gscore[neighbor] + heuristic(neighbor, destination);
//             if(open_set.find(neighbor) == open_set.end()) {
//                 open_set.insert(neighbor);
//             }
//             open_queue.push(neighbor);
//         }
//     }
// }

void update_vertex_a(
    const Vector2D& s, const Vector2D& neighbor, 
    std::unordered_map<Vector2D, Vector2D, Vec2Hash>& parent, 
    std::unordered_map<Vector2D, float, Vec2Hash>& gscore, 
    std::unordered_map<Vector2D, float, Vec2Hash>& fscore, 
    std::priority_queue<Vector2D, std::vector<Vector2D>, CompareByFScore>& open_queue,
    std::unordered_set<Vector2D, Vec2Hash>& open_set, 
    const Vector2D& destination
) {
    if(gscore[s] + cost(s, neighbor) < gscore[neighbor]) {
        gscore[neighbor] = gscore[s] + cost(s, neighbor);
        parent[neighbor] = s;
        fscore[neighbor] = gscore[neighbor] + heuristic(neighbor, destination);
        if(open_set.find(neighbor) == open_set.end()) {
            open_set.insert(neighbor);
        }
        open_queue.push(neighbor);
    }
}


std::vector<Vector2D> reconstruct_path(const Vector2D& s, std::unordered_map<Vector2D, Vector2D, Vec2Hash>& parent) {
    std::vector<Vector2D> total_path = { s };
    Vector2D curr_point = s;
    Vector2D curr_parent = parent[s];
    while(curr_parent != curr_point) {
        total_path.push_back(curr_parent);
        curr_point = curr_parent;
        curr_parent = parent[curr_point];
    }
    return total_path;
}

// https://idm-lab.org/bib/abstracts/papers/jair10b.pdf -> page 37 Branching factor 16
std::vector<Vector2D> getNeighborsPos(const Vector2D& p, const int branching_factor=16, const float step=10.0f) {
    float s = step;
    switch(branching_factor) {
        case 4: return {
                                                                     Vector2D(p.x, p.y + s),
                                         Vector2D(p.x - s, p.y),                             Vector2D(p.x + s, p.y),
                                                                     Vector2D(p.x, p.y - s)
        };
        case 8: return {
                                         Vector2D(p.x - s, p.y + s), Vector2D(p.x, p.y + s), Vector2D(p.x + s, p.y + s),
                                         Vector2D(p.x - s, p.y),                             Vector2D(p.x + s, p.y),
                                         Vector2D(p.x - s, p.y - s), Vector2D(p.x, p.y - s), Vector2D(p.x + s, p.y - s),
        };
        // TODO: refactor this to shorten the bigger rays on 0, 1, 2, 6, 9, 13, 14, 15
        case 16: {
            float vsx = 0.75f * step;
            float hsx = 1.5f * step;
            float vsy = 1.5f * step;
            float hsy = 0.75f * step;
            return {
                                          Vector2D(p.x - vsx, p.y + vsy),                         Vector2D(p.x + vsx, p.y + vsy),
            Vector2D(p.x - hsx, p.y + hsy), Vector2D(p.x - s,   p.y + s), Vector2D(p.x, p.y + s), Vector2D(p.x + s, p.y + s), Vector2D(p.x + hsx, p.y + hsy),
                                          Vector2D(p.x - s,   p.y),                             Vector2D(p.x + s, p.y),
            Vector2D(p.x - hsx, p.y - hsy), Vector2D(p.x - s,   p.y - s), Vector2D(p.x, p.y - s), Vector2D(p.x + s, p.y - s), Vector2D(p.x + hsx, p.y - hsy),
                                          Vector2D(p.x - vsx, p.y - vsy),                         Vector2D(p.x + vsx, p.y - vsy)
            };
        }
    } 
    return {};
}

bool diagonalOK(const Vector2D& s, const Vector2D& n, std::vector<Vector2D>& neighbors, const std::vector<Entity*>& tiles, int branching_factor, float step, int n_index) {
    switch(branching_factor) {
        case 4: return true; // no diagonals
        case 8:
            switch(n_index) {
                case 0: { // check top and left of s
                    Vector2D& top = neighbors[1]; Vector2D& left = neighbors[3];
                    return !isBlocked(top.x, top.y, tiles) || !isBlocked(left.x, left.y, tiles);
                }
                case 2: { // check top and right of s
                    Vector2D& top = neighbors[1]; Vector2D& right = neighbors[4];
                    return !isBlocked(top.x, top.y, tiles) || !isBlocked(right.x, right.y, tiles);
                }
                case 5: { // check bottom and left of s
                    Vector2D& bottom = neighbors[6]; Vector2D& left = neighbors[3];
                    return !isBlocked(bottom.x, bottom.y, tiles) || !isBlocked(left.x, left.y, tiles);
                }
                case 7: { // check bottom and right of s
                    Vector2D& bottom = neighbors[6]; Vector2D& right = neighbors[4];
                    return !isBlocked(bottom.x, bottom.y, tiles) || !isBlocked(right.x, right.y, tiles);
                }
                default:
                    return true;
            }
        case 16:
            switch(n_index) {
                case 0:
                case 2:
                case 3: { // check top and left of s
                    Vector2D& top = neighbors[4]; Vector2D& left = neighbors[7];
                    return !isBlocked(top.x, top.y, tiles) || !isBlocked(left.x, left.y, tiles);
                }
                case 1: 
                case 5:
                case 6: { // check top and right of s
                    Vector2D& top = neighbors[4]; Vector2D& right = neighbors[8];
                    return !isBlocked(top.x, top.y, tiles) || !isBlocked(right.x, right.y, tiles);
                }
                case 9:
                case 10:
                case 14: { // check bottom and left of s
                    Vector2D& bottom = neighbors[11]; Vector2D& left = neighbors[7];
                    return !isBlocked(bottom.x, bottom.y, tiles) || !isBlocked(left.x, left.y, tiles);
                }
                case 12:
                case 13:
                case 15: { // check bottom and right of s
                    Vector2D& bottom = neighbors[11]; Vector2D& right = neighbors[8];
                    return !isBlocked(bottom.x, bottom.y, tiles) || !isBlocked(right.x, right.y, tiles);
                }
                default:
                    return true;
            }
    }
    return true;
}

std::vector<Vector2D> theta_star(const Vector2D& start, const Vector2D& destination, const std::vector<Entity*>& tiles) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    const int bf = 8;
    const float step = 30.0f;

    std::unordered_map<Vector2D, Vector2D, Vec2Hash> parent;
    std::unordered_map<Vector2D, float, Vec2Hash> gscore;
    std::unordered_map<Vector2D, float, Vec2Hash> fscore;
    parent[start] = start;
    gscore[start] = 0.0f;
    fscore[start] = heuristic(start, destination);

    std::unordered_set<Vector2D, Vec2Hash> open_set;
    CompareByFScore cmp(fscore);
    std::priority_queue<Vector2D, std::vector<Vector2D>, CompareByFScore> open_queue(cmp);

    open_queue.push(start);
    open_set.insert(start);

    std::unordered_map<Vector2D, uint8_t, Vec2Hash> closed;

    std::vector<Vector2D> curr_neighbors;
    Vector2D s, n;

    int i, limit;
    while(!open_queue.empty()) {
        s = open_queue.top();
        open_queue.pop();
        open_set.erase(s);
        
        if(Distance(s, destination) <= 1024) {
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
            return reconstruct_path(s, parent);
        }

        closed[s] = 1;
        curr_neighbors = getNeighborsPos(s, bf, step);
        limit = curr_neighbors.size();
        for(i=0; i<limit; ++i) {
            n = curr_neighbors[i];
            if(!isBlocked(n.x, n.y, tiles)) {
                if(diagonalOK(s, n, curr_neighbors, tiles, bf, step, i)) {
                    if(closed.find(n) == closed.end()) {
                        if(open_set.find(n) == open_set.end()) {
                            gscore[n] = std::numeric_limits<float>::infinity();
                        }
                        update_vertex_a(s, n, parent, gscore, fscore, open_queue, open_set, destination);
                    }
                }                
            }
        }
    }

    return {};
}