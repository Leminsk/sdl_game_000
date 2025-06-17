#pragma once
#include <sstream>
#include <unordered_map>
#include <vector>
#include <limits>
#include <chrono>
#include <queue>
#include <unordered_set>
#include "Game.hpp"
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

struct MeshNode {
    int x;
    int y;
    bool operator==(const MeshNode& b) const { return this->x == b.x && this->y == b.y; }
    bool operator!=(const MeshNode& b) const { return this->x != b.x || this->y != b.y; }
};
struct Node2Hash {
    std::size_t operator()(const MeshNode& n) const {
        return std::hash<float>()(n.x) ^ (std::hash<float>()(n.y) << 1);
    }
};
struct NodeCompareByFScore {
    const std::unordered_map<MeshNode, float, Node2Hash>& fscore;
    NodeCompareByFScore(const std::unordered_map<MeshNode, float, Node2Hash>& fs) : fscore(fs) {}
    bool operator()(const MeshNode& a, const MeshNode& b) const {
        return fscore.at(a) > fscore.at(b);
    }
};

void printNodes(const std::vector<MeshNode>& v) {
    std::cout << "{\n";
    for(auto& n : v) { std::cout << '{' << n.x << ',' << n.y << '}' << ' '; }
    std::cout << "\n}\n";
}

float NodeDistance(const MeshNode& a, const MeshNode& b) {
    float x = b.x - a.x;
    float y = b.y - a.y;
    return (y*y) + (x*x);
}

MeshNode convertVector2DToMeshNode(const Vector2D& v, const int mesh_density) {
    int shift;
    switch(mesh_density) {
        case 64: shift = 3; break;
        case 16: shift = 2; break;
        case 4:  shift = 1; break;
        case 1: 
        default: shift = 0;
    }
    int factor = (Game::UNIT_SIZE<<1) >> shift;
    return {
        static_cast<int>(std::floor(v.x / factor)),
        static_cast<int>(std::floor(v.y / factor))
    };
}

Vector2D convertMeshNodeToVector2D(const MeshNode& n, const int mesh_density) {
    int shift;
    switch(mesh_density) {
        case 64: shift = 3; break;
        case 16: shift = 2; break;
        case 4:  shift = 1; break;
        case 1: 
        default: shift = 0;
    }
    int factor = (Game::UNIT_SIZE<<1) >> shift;
    return Vector2D(
        static_cast<float>((n.x * factor) + (factor>>1)),
        static_cast<float>((n.y * factor) + (factor>>1))
    );
}

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

void update_vertex_mesh(
    const MeshNode& s, const MeshNode& neighbor, 
    std::unordered_map<MeshNode, MeshNode, Node2Hash>& parent, 
    std::unordered_map<MeshNode, float, Node2Hash>& gscore, 
    std::unordered_map<MeshNode, float, Node2Hash>& fscore, 
    std::priority_queue<MeshNode, std::vector<MeshNode>, NodeCompareByFScore>& open_queue,
    std::unordered_set<MeshNode, Node2Hash>& open_set, 
    const MeshNode& destination
) {
    if(gscore[s] + NodeDistance(s, neighbor) < gscore[neighbor]) {
        gscore[neighbor] = gscore[s] + NodeDistance(s, neighbor);
        parent[neighbor] = s;
        fscore[neighbor] = gscore[neighbor] + NodeDistance(neighbor, destination);
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

std::vector<Vector2D> reconstruct_path_mesh(const MeshNode& s, std::unordered_map<MeshNode, MeshNode, Node2Hash>& parent, const int mesh_density) {
    std::vector<Vector2D> total_path = { convertMeshNodeToVector2D(s, mesh_density) };
    MeshNode curr_point = s;
    MeshNode curr_parent = parent[s];
    while(curr_parent != curr_point) {
        total_path.push_back(convertMeshNodeToVector2D(curr_parent, mesh_density));
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

std::vector<MeshNode> getMeshNeighbors(const int x, const int y, const int width_limit, const int height_limit, const int branching_factor=8) {
    const int s = 1;
    switch(branching_factor) {
        case 4: {
            if(x == 0) {
                if(y == 0) {            return {
                                            {x, y + s}, 
                                                        {x + s, y}

                };}
                if(y == height_limit) { return {

                                                        {x + s, y}, 
                                            {x, y - s}
                };}
                return {
                                            {x, y + s}, 
                                                        {x + s, y}, 
                                            {x, y - s}
                };
            }
            if(x == width_limit) {
                if(y == 0) {            return {
                                            {x, y + s}, 
                                {x - s, y}

                };}
                if(y == height_limit) { return {

                                {x - s, y}, 
                                            {x, y - s}
                };}
                return {
                                            {x, y + s}, 
                                {x - s, y}, 
                                            {x, y - s}
                };
            }
            if(y == 0) { return {
                                            {x, y + s}, 
                                {x - s, y},             {x + s, y}

            };}
            if(y == height_limit) { return {
                                            
                                {x - s, y},             {x + s, y}, 
                                            {x, y - s}
            };}
            return {
                                            {x, y + s}, 
                                {x - s, y},             {x + s, y}, 
                                            {x, y - s}
            };
        }
        case 8: {
            if(x == 0) {
                if(y == 0) { return {
                                                {x, y + s}, {x + s, y + s},
                                                            {x + s, y    }
                };}
                if(y == height_limit) { return {
                                
                                                            {x + s, y    },
                                                {x, y - s}, {x + s, y - s}
                };}
                // else
                return {
                                                {x, y + s}, {x + s, y + s},
                                                            {x + s, y    },
                                                {x, y - s}, {x + s, y - s}
                };
            }
            if(x == width_limit) {
                if(y == 0) { return {
                                {x - s, y + s}, {x, y + s},
                                {x - s, y    }

                };}
                if(y == height_limit) { return {

                                {x - s, y    },             
                                {x - s, y - s}, {x, y - s}
                };}
                // else
                return {
                                {x - s, y + s}, {x, y + s},
                                {x - s, y    },            
                                {x - s, y - s}, {x, y - s}
                };
            }
            if(y == 0) { return {
                                {x - s, y + s}, {x, y + s}, {x + s, y + s},
                                {x - s, y    },             {x + s, y    }
                                
            };}
            if(y == height_limit) { return {
                                            
                                {x - s, y    },             {x + s, y    },
                                {x - s, y - s}, {x, y - s}, {x + s, y - s}
            };}
            return {
                                {x - s, y + s}, {x, y + s}, {x + s, y + s},
                                {x - s, y    },             {x + s, y    },
                                {x - s, y - s}, {x, y - s}, {x + s, y - s}
            };
        }
        case 16: {
            int s2 = s<<1;
            if(x == 0) {
                if(y == 0) { return {
                                                                 {x + s, y + s2},
                                                 {x    , y + s}, {x + s, y + s}, {x + s2, y + s},
                                                                 {x + s, y    }
                                

                };}
                if(y == height_limit) { return {
                    

                                                                 {x + s, y    },
                                                 {x    , y - s}, {x + s, y - s}, {x + s2, y - s},
                                                                 {x + s, y - s2}
                };}
                // else
                return {
                                                                {x + s, y + s2},
                                                {x    , y + s}, {x + s, y + s}, {x + s2, y + s},
                                                                {x + s, y    },
                                                {x    , y - s}, {x + s, y - s}, {x + s2, y - s},
                                                                {x + s, y - s2}
                };
            }
            if(x == width_limit) {
                if(y == 0) { return {
                                {x - s, y + s2},                 
                {x - s2, y + s}, {x - s, y + s}, {x    , y + s}, 
                                {x - s, y    }


                };}
                if(y == height_limit) { return {
                    

                                {x - s, y    },                 
                {x - s2, y - s}, {x - s, y - s}, {x    , y - s}, 
                                {x - s, y - s2}
                };}
                // else
                return {
                                {x - s, y + s2},                 
                {x - s2, y + s}, {x - s, y + s}, {x    , y + s}, 
                                {x - s, y    },                 
                {x - s2, y - s}, {x - s, y - s}, {x    , y - s}, 
                                {x - s, y - s2}
                };
            }
            if(y == 0) { return {
                                {x - s, y + s2},                 {x + s, y + s2},
                {x - s2, y + s}, {x - s, y + s}, {x    , y + s}, {x + s, y + s}, {x + s2, y + s},
                                {x - s, y    },                 {x + s, y    }
                                
                                
            };}
            if(y == height_limit) { return {            
                                
                
                                {x - s, y    },                 {x + s, y    },
                {x - s2, y - s}, {x - s, y - s}, {x    , y - s}, {x + s, y - s}, {x + s2, y - s},
                                {x - s, y - s2},                 {x + s, y - s2}
            };}
            return {
                                {x - s, y + s2},                 {x + s, y + s2},
                {x - s2, y + s}, {x - s, y + s}, {x    , y + s}, {x + s, y + s}, {x + s2, y + s},
                                {x - s, y    },                 {x + s, y    },
                {x - s2, y - s}, {x - s, y - s}, {x    , y - s}, {x + s, y - s}, {x + s2, y - s},
                                {x - s, y - s2},                 {x + s, y - s2}
            };
        }
    } 
    return {};
}

bool diagonalOK(const Vector2D& s, std::vector<Vector2D>& neighbors, const std::vector<Entity*>& tiles, int branching_factor, int n_index) {
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

bool walkableInMesh(int x, int y, const std::vector<std::vector<bool>>& mesh) {
    // mesh is indexed HEIGHT first
    return !mesh[y][x];
}

bool meshDiagonalOK(const MeshNode& s, const MeshNode& n, const std::vector<std::vector<bool>>& mesh) {
    // no diagonals
    if(s.y == n.y || s.x == n.x) { return true; }

    if(s.x < n.x) {
        if(s.y < n.y) { // check right and top of s
            return walkableInMesh(s.x + 1, s.y, mesh) || walkableInMesh(s.x, s.y + 1, mesh);
        }
        // check right and bottom of s
        return walkableInMesh(s.x + 1, s.y, mesh) || walkableInMesh(s.x, s.y - 1, mesh);
    }
    if(s.y < n.y) { // check left and top of s
        return walkableInMesh(s.x - 1, s.y, mesh) || walkableInMesh(s.x, s.y + 1, mesh);
    }
    // check left bototm of s
    return walkableInMesh(s.x - 1, s.y, mesh) || walkableInMesh(s.x, s.y - 1, mesh);
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
            std::cout << "theta_star() Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
            return reconstruct_path(s, parent);
        }

        closed[s] = 1;
        curr_neighbors = getNeighborsPos(s, bf, step);
        limit = curr_neighbors.size();
        for(i=0; i<limit; ++i) {
            n = curr_neighbors[i];
            if(!isBlocked(n.x, n.y, tiles)) {
                if(diagonalOK(s, curr_neighbors, tiles, bf, i)) {
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

// Leaving this here in case I want to refactor it. But I might trash this later
// go around the blocked tile searching for a walkable tile (like Dijkstra)
MeshNode findClosestWalkable(
    const MeshNode& origin, 
    const std::vector<std::vector<bool>>& mesh, const int branching_factor, 
    const int mesh_width_limit, const int mesh_height_limit
) {
    std::unordered_map<MeshNode, float, Node2Hash> distances;
    distances[origin] = 0.0f;

    NodeCompareByFScore cmp(distances);
    std::priority_queue<MeshNode, std::vector<MeshNode>, NodeCompareByFScore> pq(cmp);
    

    std::vector<std::vector<bool>> visited = mesh;
    for(int i=0; i<visited.size(); ++i) {
        for(int j=0; j<visited.size(); ++j) {
            visited[i][j] = false;
        }
    }

    std::vector<MeshNode> neighbors;
    MeshNode curr = origin;
    distances[curr] = 0.0f;
    pq.push(curr);

    while(!pq.empty()) {
        curr = pq.top();
        pq.pop();
        if(walkableInMesh(curr.x, curr.y, mesh)) { printf("FOUND!\n"); break; }
        visited[curr.y][curr.x] = true;

        neighbors = getMeshNeighbors(curr.x, curr.y, mesh_width_limit, mesh_height_limit, branching_factor);
        for(MeshNode& n : neighbors) {
            if(!visited[n.y][n.x]) {
                distances[n] = Distance(Vector2D(origin.x, origin.y), Vector2D(n.x, n.y));
                pq.push(n);
            }
        }
    }
    return curr;
}



// similar to theta_star() but receives a mesh of nodes to use as reference for path finding
std::vector<Vector2D> a_star_mesh(
    const MeshNode& start, const MeshNode& destination, 
    const std::vector<std::vector<bool>>& mesh, const int branching_factor,
    const int mesh_width_limit, const int mesh_height_limit, const int density,
    const std::chrono::steady_clock::time_point& begin
) {
    std::unordered_map<MeshNode, MeshNode, Node2Hash> parent;
    std::unordered_map<MeshNode, float, Node2Hash> gscore;
    std::unordered_map<MeshNode, float, Node2Hash> fscore;
    parent[start] = start;
    gscore[start] = 0.0f;
    fscore[start] = NodeDistance(start, destination);

    std::unordered_set<MeshNode, Node2Hash> open_set;
    NodeCompareByFScore cmp(fscore);
    std::priority_queue<MeshNode, std::vector<MeshNode>, NodeCompareByFScore> open_queue(cmp);

    open_queue.push(start);
    open_set.insert(start);

    std::unordered_map<MeshNode, uint8_t, Node2Hash> closed;

    std::vector<MeshNode> curr_neighbors;
    MeshNode s, n;


    int i, limit;
    while(!open_queue.empty()) {
        s = open_queue.top();
        open_queue.pop();
        open_set.erase(s);
        
        if(s == destination) {
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "a_star_mesh() Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
            return reconstruct_path_mesh(s, parent, density);
        }

        closed[s] = 1;
        curr_neighbors = getMeshNeighbors(s.x, s.y, mesh_width_limit, mesh_height_limit, branching_factor);
        limit = curr_neighbors.size();
        for(i=0; i<limit; ++i) {
            n = curr_neighbors[i];
            if(walkableInMesh(n.x, n.y, mesh)) {
                if(meshDiagonalOK(s, n, mesh)) {
                    if(closed.find(n) == closed.end()) {
                        if(open_set.find(n) == open_set.end()) {
                            gscore[n] = std::numeric_limits<float>::infinity();
                        }
                        update_vertex_mesh(s, n, parent, gscore, fscore, open_queue, open_set, destination);
                    }
                }                
            }
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "a_star_mesh() NO PATH Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
    return {};
}

std::vector<Vector2D> find_path(const Vector2D& start, const Vector2D& destination) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    const float           one_by_one =    8192.0f; //  1x1 tile area
    const float         four_by_four =  131072.0f; //  4x4 tile area
    const float threetwo_by_threetwo = 8388608.0f; // 32x32 tile area
    const float distance = Distance(start, destination);
    const int branching_factor = 8;
    MeshNode start_node;
    MeshNode dest_node;
    std::vector<std::vector<bool>> *mesh;
    int density, width_limit, height_limit;
    if(distance <= one_by_one) { // use very granular mesh (tile -> 64 nodes)
        density = 64; mesh = &Game::collision_mesh_64; 
        width_limit = Game::collision_mesh_64_width-1;
        height_limit = Game::collision_mesh_64_height-1;
    } else if(distance <= four_by_four) { // use granular mesh (tile -> 16 nodes)
        density = 16; mesh = &Game::collision_mesh_16;
        width_limit = Game::collision_mesh_16_width-1;
        height_limit = Game::collision_mesh_16_height-1;
    } else if(distance <= threetwo_by_threetwo) { // use standard mesh (tile -> 4 nodes)
        density = 4; mesh = &Game::collision_mesh_4;
        width_limit = Game::collision_mesh_4_width-1;
        height_limit = Game::collision_mesh_4_height-1;
    } else { // use big tile mesh (tile -> 1 node)
        density = 1; mesh = &Game::collision_mesh_1;
        width_limit = Game::collision_mesh_1_width-1;
        height_limit = Game::collision_mesh_1_height-1;
    }
    start_node = convertVector2DToMeshNode(start, density);
    dest_node = convertVector2DToMeshNode(destination, density);
    // clamp out of bounds
    if(dest_node.x > width_limit) { dest_node.x = width_limit; }
    else if(dest_node.x < 0) { dest_node.x = 0; }
    if(dest_node.y > height_limit) { dest_node.y = height_limit; }
    else if(dest_node.y < 0) { dest_node.y = 0; }
    if(walkableInMesh(dest_node.x, dest_node.y, *mesh)) {
        return a_star_mesh(start_node, dest_node, *mesh, branching_factor, width_limit, height_limit, density, begin);
    } 
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "a_star_mesh() PATH BLOCKED Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
    return {};    
}
