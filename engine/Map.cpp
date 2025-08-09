#include <fstream>
#include "Map.hpp"


struct simple_pixel {
    Uint8 r, g, b;
};


bool Map::LoadMapFile(std::string& path) {
    // https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file
    static constexpr size_t HEADER_SIZE = 54;

    std::ifstream mapFile(path, std::ios::binary);
    if(!mapFile) {
        std::cout << "Failed to open file: " << path << '\n';
        return false;
    }

    std::vector<char> header(HEADER_SIZE);
    mapFile.read(header.data(), HEADER_SIZE);

    auto fileSize       = *reinterpret_cast<uint32_t *>(&header[2]);
    auto dataOffset     = *reinterpret_cast<uint32_t *>(&header[10]);
    this->layout_width  = *reinterpret_cast<uint32_t *>(&header[18]);
    if(this->layout_width % 4 !=0) {
        std::cout << "Invalid BMP width.\n";
        return false;
    }
    this->layout_height = *reinterpret_cast<uint32_t *>(&header[22]);
    auto depth          = *reinterpret_cast<uint16_t *>(&header[28]);

    this->world_layout_width = this->layout_width * this->tile_width;
    this->world_layout_height = this->layout_height * this->tile_width;

    auto dataSize = ((this->layout_width * 3 + 3) & (~3)) * this->layout_height;
    std::vector<char> img(dataSize);
    mapFile.read(img.data(), img.size());
    mapFile.close();

    this->layout.resize(this->layout_height);

    char temp;
    int line_offset = 0;
    int line_index = this->layout_width;
    int current_line;
    simple_pixel current_pixel;
    int tile_type;
    // reads "bottom" row first
    for(auto i = 0; i < dataSize; i += 3) {
        // BMP goes BGR
        current_pixel.r = img[i+2];
        current_pixel.g = img[i+1];
        current_pixel.b = img[i];

        if(line_index == this->layout_width) {
            ++line_offset;
            current_line = this->layout_height - line_offset;
            this->layout[current_line].resize(this->layout_width);
            line_index = 0;
        }
        
        if(
            current_pixel.r == COLORS_SPAWN.r && 
            current_pixel.g == COLORS_SPAWN.g && 
            current_pixel.b == COLORS_SPAWN.b
        ) { // base spawn
            tile_type = TILE_BASE_SPAWN;
        } else if(
            current_pixel.r == COLORS_NAVIGABLE.r && 
            current_pixel.g == COLORS_NAVIGABLE.g && 
            current_pixel.b == COLORS_NAVIGABLE.b
        ) { // navigable terrain
            tile_type = TILE_NAVIGABLE;
        } else if(
            current_pixel.r == COLORS_IMPASSABLE.r && 
            current_pixel.g == COLORS_IMPASSABLE.g && 
            current_pixel.b == COLORS_IMPASSABLE.b
        ) { // impassable terrain
            tile_type = TILE_IMPASSABLE;
        } else if(
            current_pixel.r == COLORS_ROUGH.r && 
            current_pixel.g == COLORS_ROUGH.g && 
            current_pixel.b == COLORS_ROUGH.b
        ) { // rough terrain
            tile_type = TILE_ROUGH;
        } else { // plain terrain
            tile_type = TILE_PLAIN;
        }

        this->layout[current_line][line_index] = tile_type;
        ++line_index;
    }
    return true;
}
