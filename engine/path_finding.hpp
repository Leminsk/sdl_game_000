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


float nodeTypeCost(const MeshNode& n, const std::vector<std::vector<uint8_t>>& mesh) {
    // FAR FUTURE TODO: make these values related to techs
    switch(mesh[n.y][n.x]) {
        case TILE_PLAIN: return 0.0f;
        case TILE_ROUGH: return 20.0f;
        case TILE_NAVIGABLE: return 10.0f;
        // case TILE_BASE_SPAWN: <-
        // case TILE_IMPASSABLE: <- these two never get to be walkable
        default: return 0.0f;
    }
}

// https://idm-lab.org/bib/abstracts/papers/jair10b.pdf -> page 26 Algorithm 5
// I changed my mind again. 
// Maybe I'll reintroduce tile costs in some other way, 
// but if I just add it up here, the average runtime of path finding for 1 drone jumps to 2.3ms which is kinda bad. (graph exploration grows a lot more)
// If I leave it with just the node distance, it usually stays under 1ms.
float heuristicCost(const MeshNode& n, const MeshNode& dest, const std::vector<std::vector<uint8_t>>& mesh) {
    return NodeDistance(n, dest);// + nodeTypeCost(n, mesh);
}


void update_vertex_mesh(
    const MeshNode& s, const MeshNode& neighbor, 
    std::unordered_map<MeshNode, MeshNode, Node2Hash>& parent, 
    std::unordered_map<MeshNode, float, Node2Hash>& gscore, 
    std::unordered_map<MeshNode, float, Node2Hash>& fscore, 
    std::priority_queue<MeshNode, std::vector<MeshNode>, NodeCompareByFScore>& open_queue,
    std::unordered_set<MeshNode, Node2Hash>& open_set, 
    const MeshNode& destination,
    const std::vector<std::vector<uint8_t>>& mesh
) {
    float total_cost = gscore[s] + heuristicCost(s, neighbor, mesh);
    if(total_cost < gscore[neighbor]) {
        gscore[neighbor] = total_cost;
        parent[neighbor] = s;
        fscore[neighbor] = gscore[neighbor] + heuristicCost(neighbor, destination, mesh);
        if(open_set.find(neighbor) == open_set.end()) {
            open_set.insert(neighbor);
        }
        open_queue.push(neighbor);
    }
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

// https://idm-lab.org/bib/abstracts/papers/jair10b.pdf -> page 37 Branching factor
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


bool walkableInMesh(int x, int y, const std::vector<std::vector<uint8_t>>& mesh) {
    // mesh is indexed HEIGHT first
    // FAR FUTURE TODO: change this to receive input of a tech level that unlocks TILE_NAVIGABLE
    return (mesh[y][x] != TILE_IMPASSABLE && mesh[y][x] != TILE_NAVIGABLE && mesh[y][x] != TILE_BASE_SPAWN);
}

bool meshDiagonalOK(const MeshNode& s, const MeshNode& n, const std::vector<std::vector<uint8_t>>& mesh) {
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


// Leaving this here in case I want to refactor it. But I might trash this later
// go around the blocked tile searching for a walkable tile (like Dijkstra)
MeshNode findClosestWalkable(
    const MeshNode& origin, 
    const std::vector<std::vector<uint8_t>>& mesh, const int branching_factor, 
    const int mesh_width_limit, const int mesh_height_limit
) {
    std::unordered_map<MeshNode, float, Node2Hash> distances;
    distances[origin] = 0.0f;

    NodeCompareByFScore cmp(distances);
    std::priority_queue<MeshNode, std::vector<MeshNode>, NodeCompareByFScore> pq(cmp);
    

    std::vector<std::vector<uint8_t>> visited = mesh;
    for(int i=0; i<visited.size(); ++i) {
        for(int j=0; j<visited.size(); ++j) {
            visited[i][j] = 0;
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
        visited[curr.y][curr.x] = 1;

        neighbors = getMeshNeighbors(curr.x, curr.y, mesh_width_limit, mesh_height_limit, branching_factor);
        for(MeshNode& n : neighbors) {
            if(visited[n.y][n.x] = 0) {
                distances[n] = Distance(Vector2D(origin.x, origin.y), Vector2D(n.x, n.y));
                pq.push(n);
            }
        }
    }
    return curr;
}



// receives a mesh of nodes to use as reference for path finding
// will return a vector of points in which the FIRST(index:0) element is the DESTINATION with the following elements a path up until the start point
std::vector<Vector2D> a_star_mesh(
    const MeshNode& start, const MeshNode& destination, 
    const std::vector<std::vector<uint8_t>>& mesh, const int branching_factor,
    const int mesh_width_limit, const int mesh_height_limit, const int density,
    const std::chrono::steady_clock::time_point& begin
) {
    std::unordered_map<MeshNode, MeshNode, Node2Hash> parent;
    std::unordered_map<MeshNode, float, Node2Hash> gscore;
    std::unordered_map<MeshNode, float, Node2Hash> fscore;
    parent[start] = start;
    gscore[start] = 0.0f;
    fscore[start] = heuristicCost(start, destination, mesh);

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
                        update_vertex_mesh(s, n, parent, gscore, fscore, open_queue, open_set, destination, mesh);
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
    std::vector<std::vector<uint8_t>> *mesh;
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
