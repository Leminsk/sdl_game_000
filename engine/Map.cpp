#include "Map.hpp"
#include <fstream>

struct simple_pixel {
    Uint8 r, g, b;
};

Map::Map() {
}

Map::~Map() {
    
}

void Map::LoadMap(std::string path) {
    // https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file
    static constexpr size_t HEADER_SIZE = 54;

    std::ifstream mapFile(path, std::ios::binary);
    if(!mapFile) {
        std::cout << "Failed to open file: " << path << '\n';
        return;
    }

    std::vector<char> header(HEADER_SIZE);
    mapFile.read(header.data(), HEADER_SIZE);

    auto fileSize       = *reinterpret_cast<uint32_t *>(&header[2]);
    auto dataOffset     = *reinterpret_cast<uint32_t *>(&header[10]);
    this->layout_width  = *reinterpret_cast<uint32_t *>(&header[18]);
    this->layout_height = *reinterpret_cast<uint32_t *>(&header[22]);
    auto depth          = *reinterpret_cast<uint16_t *>(&header[28]);

    auto dataSize = ((this->layout_width * 3 + 3) & (~3)) * this->layout_height;
    std::vector<char> img(dataSize);
    mapFile.read(img.data(), img.size());
    mapFile.close();

    this->layout.resize(this->layout_height);

    char temp;
    int line_offset = 0;
    int line_index;
    int current_line;
    simple_pixel current_pixel;
    int tile_type;
    for(auto i = 0; i < dataSize; i += 3) {
        // BMP goes BGR
        current_pixel.r = img[i+2];
        current_pixel.g = img[i+1];
        current_pixel.b = img[i];

        if(i % this->layout_width == 0) {
            ++line_offset;
            current_line = this->layout_height - line_offset;
            this->layout[current_line].resize(this->layout_width);
            line_index = 0;
        }
        
        if(current_pixel.r == 0x00 && current_pixel.g == 0x00 && current_pixel.b == 0x00) { // base spawn
            tile_type = 4;
        } else if(current_pixel.r == 0xA4 && current_pixel.g == 0xC2 && current_pixel.b == 0xF4) { // navigable terrain
            tile_type = 3;
        } else if(current_pixel.r == 0xAC && current_pixel.g == 0xAC && current_pixel.b == 0xAC) { // impassable terrain
            tile_type = 2;
        } else if(current_pixel.r == 0xF9 && current_pixel.g == 0xCB && current_pixel.b == 0x9C) { // rough terrain
            tile_type = 1;
        } else { // plain terrain
            tile_type = 0;
        }

        Game::AddTile(tile_type, static_cast<float>(line_index*32), static_cast<float>(current_line*32));
        this->layout[current_line][line_index] = tile_type;
        ++line_index;
    }
}