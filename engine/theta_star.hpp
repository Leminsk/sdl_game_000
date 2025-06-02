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
        return std::hash<float>{}(v.x) ^ (std::hash<float>{}(v.y) << 1);
    }
};

struct CompareByFScore {
    const std::unordered_map<Vector2D, float, Vec2Hash>& fscore;
    CompareByFScore(const std::unordered_map<Vector2D, float, Vec2Hash>& fs) : fscore(fs) {}
    bool operator()(const Vector2D& a, const Vector2D& b) const {
        return fscore.at(a) > fscore.at(b);
    }
};

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

bool lineOfSight(const Vector2D& a, const Vector2D& b, const std::vector<Entity*>& tiles) {
    float x0 = a.x;
    float y0 = a.y;
    float x1 = b.x;
    float y1 = b.y;
    float dy = -std::fabs(y1 - y0);
    float dx = std::fabs(x1 - x0);
    float f = 0.0f;
    float sy = -1;
    float sx = -1;
    if(x0 < x1) { sx = 1; }
    if(y0 < y1) { sy = 1; }

    float e = dx + dy;
    float e2;
    while(true) {
        if(isBlocked(x0, y0, tiles)) { return false; }
        if(x0 == x1 && y0 == y1) { return true; }
        e2 = 2*e;
        if(e2 >= dy) {
            if(x0 == x1) { return true; }
            e += dy;
            x0 += sx;
        }
        if(e2 <= dx) {
            if(y0 == y1) { return true; }
            e += dx;
            y0 += sy;
        }
    }
}

float cost(const Vector2D& a, const Vector2D& b) {
    return Distance(a, b);
}

// https://idm-lab.org/bib/abstracts/papers/jair10b.pdf -> page 26 Algorithm 5
float heuristic(const Vector2D& p, const Vector2D& destination) {
    return cost(p, destination);

    float dx = std::fabs(p.x - destination.x);
    float dy = std::fabs(p.y - destination.y);
    float largest = std::max(dx, dy);
    float smallest = std::min(dx, dy);
    const float sqrt_of_2 = 1.4142135f;
    return (sqrt_of_2 * smallest) + (largest - smallest);
}

std::string getStringV2(const Vector2D& v) {
    std::stringstream ss;
    ss << v.x << ',' << v.y;
    return ss.str();
}

std::vector<Vector2D>::const_iterator iterInVec(const Vector2D& pos, const std::vector<Vector2D>& vec) {
    return std::find_if(
        vec.begin(), 
        vec.end(), 
        [&](const Vector2D& n) { 
            return n == pos;
        }
    );
}

void update_vertex_theta(
    const Vector2D& s, const Vector2D& neighbor, 
    std::unordered_map<std::string, Vector2D>& parent, 
    std::unordered_map<std::string, float>& gscore, 
    std::unordered_map<std::string, float>& fscore, 
    const std::vector<Entity*>& tiles, 
    std::vector<Vector2D>& open, 
    const Vector2D& destination
) {
    std::vector<Vector2D>::const_iterator it;
    Vector2D s_parent = parent[getStringV2(s)];
    std::string neighbor_str, p_or_s_str;

    neighbor_str = getStringV2(neighbor);

    if(lineOfSight(s_parent, neighbor, tiles)) {
        p_or_s_str = getStringV2(s_parent);
        if(gscore[p_or_s_str] + cost(s_parent, neighbor) < gscore[neighbor_str]) {
            gscore[neighbor_str] = gscore[p_or_s_str] + cost(s_parent, neighbor);
            parent[neighbor_str] = s_parent;
            it = iterInVec(neighbor, open);
            if(it != open.end()) {
                Vector2D v_copy = neighbor;
                open.erase(it);
                fscore[neighbor_str] = gscore[neighbor_str] + heuristic(v_copy, destination);
                open.push_back(v_copy);
            } else {
                fscore[neighbor_str] = gscore[neighbor_str] + heuristic(neighbor, destination);
                open.push_back(neighbor);
            }
        }
    } else {
        p_or_s_str = getStringV2(s);
        if(gscore[p_or_s_str] + cost(s, neighbor) < gscore[neighbor_str]) {
            gscore[neighbor_str] = gscore[p_or_s_str] + cost(s, neighbor);
            parent[neighbor_str] = s;
            it = iterInVec(neighbor, open);
            if(it != open.end()) {
                Vector2D v_copy = neighbor;
                open.erase(it);
                fscore[neighbor_str] = gscore[neighbor_str] + heuristic(v_copy, destination);
                open.push_back(v_copy);
            } else {
                fscore[neighbor_str] = gscore[neighbor_str] + heuristic(neighbor, destination);
                open.push_back(neighbor);
            }
        }
    }
}

void update_vertex_a(
    const Vector2D& s, const Vector2D& neighbor, 
    std::unordered_map<Vector2D, Vector2D, Vec2Hash>& parent, 
    std::unordered_map<Vector2D, float, Vec2Hash>& gscore, 
    std::unordered_map<Vector2D, float, Vec2Hash>& fscore, 
    const std::vector<Entity*>& tiles, 
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
        case 16: float s2 = 2*s; return {
                                         Vector2D(p.x - s, p.y + s2),                        Vector2D(p.x + s, p.y + s2),
            Vector2D(p.x - s2, p.y + s), Vector2D(p.x - s, p.y + s), Vector2D(p.x, p.y + s), Vector2D(p.x + s, p.y + s), Vector2D(p.x + s2, p.y + s),
                                         Vector2D(p.x - s, p.y),                             Vector2D(p.x + s, p.y),
            Vector2D(p.x - s2, p.y - s), Vector2D(p.x - s, p.y - s), Vector2D(p.x, p.y - s), Vector2D(p.x + s, p.y - s), Vector2D(p.x + s2, p.y - s),
                                         Vector2D(p.x - s, p.y - s2),                        Vector2D(p.x + s, p.y - s2)
        };
    }
    return {};
    
}


std::vector<Vector2D> theta_star(const Vector2D& start, const Vector2D& destination, const std::vector<Entity*>& tiles) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
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
    Vector2D s;

    while(!open_queue.empty()) {
        s = open_queue.top();
        open_queue.pop();
        open_set.erase(s);
        
        if(s == destination) {
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[us]" << std::endl;
            return reconstruct_path(s, parent);
        }

        closed[s] = 1;
        curr_neighbors = getNeighborsPos(s, 16, 50.0f);

        for(Vector2D& n : curr_neighbors) {
            if(!isBlocked(n.x, n.y, tiles)) {
                if(closed.find(n) == closed.end()) {
                    if(open_set.find(n) == open_set.end()) {
                        gscore[n] = std::numeric_limits<float>::infinity();
                    }
                    update_vertex_a(s, n, parent, gscore, fscore, tiles, open_queue, open_set, destination);
                }
            }
        }
    }

    return {};
}