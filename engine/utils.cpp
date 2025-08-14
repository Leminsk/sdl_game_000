#include "utils.hpp"


/*
read bit value from a byte. LSB=0, MSB=7
return bit value as 0x00 or 0x01
*/ 
uint8_t readBit(uint8_t byte, uint8_t bit) {
    if(bit > 0x07) { 
        printf("Invalid bit index.\n");
        return byte;
    }
    return ((byte >> bit) & 0x01);
}

/*
set nth bit value in a byte. LSB=0, MSB=7
values above 0x00 will be considered as '1';
*/
void setBit(uint8_t* byte, uint8_t nbit, uint8_t value) {
    if(value) {
        *byte = (*byte  & ~(0x01 << nbit)) | (0x01 << nbit);
    } else {
        *byte = (*byte  & ~(0x01 << nbit));
    }
}

std::string left_pad_int(int i, int total_digits, const char digit, bool can_be_negative) {
    std::string number_str = std::to_string(std::abs(i));
    int digits = number_str.size();
    char sign = ' ';
    if(i < 0) {
        if(digits - 1 > total_digits) {
            return number_str;
        }
        --digits;
        sign = '-';
    } else if(digits > total_digits) {
        return number_str;
    }
    
    std::string left_padding = "";
    int amount_to_pad = total_digits - digits;
    for(int i=0; i<amount_to_pad; ++i) {
        left_padding += digit;
    }

    if(can_be_negative) {
        return sign + left_padding + number_str;
    }
    return left_padding + number_str;
}

std::string format_decimal(float f, int integer_precision, int decimal_precision, bool can_be_negative) {

    if(std::isnan(f)) {
        std::string space_padding = "";
        int spaces = integer_precision - 2;
        for(int i=0; i<spaces; ++i) { space_padding += ' '; }
        std::string right_space_padding = "";
        int right_spaces = 1 + decimal_precision;
        for(int i=0; i<right_spaces; ++i) { right_spaces += ' '; }
        return space_padding+"NaN"+right_space_padding;
    }

    std::stringstream number;
    number << std::setprecision(decimal_precision) << std::fixed << f;
    std::string number_str = number.str();
    int integer_digits = 0;
    int is_negative = 0;
    for(char& c : number_str) {
        if(c == '-') { is_negative = 1; continue; }
        else if(c == '.') { break; }
        else { ++integer_digits; }
    }

    std::string space_padding = "";
    // add 1 (can_be_negative) for the '-' sign
    int spaces = can_be_negative + integer_precision - (integer_digits + is_negative);
    for(int i=0; i<spaces; ++i) { space_padding += ' '; }

    return space_padding + number_str;
}

// range is inclusive on both ends
int randomInt(std::mt19937 *generator, int min, int max) {
    std::uniform_int_distribution<int> distribute(min, max);
    return distribute(*generator);
}

// range is inclusive on both ends
float randomFloat(std::mt19937 *generator, float min, float max) {
    std::uniform_real_distribution<float> distribute(min, max);
    return distribute(*generator);
}


// https://stackoverflow.com/questions/216823/how-can-i-trim-a-stdstring
// Trim from the start (in place)
void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}
// Trim from the end (in place)
void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
// Trim from both ends (in place)
void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}
// Trim from the start (copying)
std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}
// Trim from the end (copying)
std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}
// Trim from both ends (copying)
std::string trim_copy(std::string s) {
    trim(s);
    return s;
}



/*
Returns a list with all files in the directory. Optional file_format will filter the files and return file names that match the file extension provided.
Example: file_format="BMP" will only return files which have the ".bmp" extension.
Trailing slashes for directory are ignored.
*/
std::vector<std::string> getFileNamesInDirectory(const std::string& directory, const std::string& file_format) {
    std::vector<std::string> file_names = {};
    std::string current_path;
    const int format_size = file_format.size();

    if(format_size > 0) {
        const int last_ext = format_size-1;
        int last_index;
        bool match_format;
        for(const auto& entry : std::filesystem::directory_iterator(directory)) {
            current_path = entry.path().u8string();
            last_index = current_path.size() - 1;
            match_format = true;
            for(int i=0; i<format_size; ++i) {
                // in theory I shouldn't need to worry about file extensions which are not ASCII-like
                if( std::tolower(current_path[last_index-i]) != std::tolower(file_format[last_ext-i]) ) {
                    match_format = false;
                    break;
                }
            }
            if(match_format) {
                file_names.push_back( entry.path().u8string() );
            }        
        }
    } else {
        for(const auto& entry : std::filesystem::directory_iterator(directory)) {
            file_names.push_back( entry.path().u8string() );
        }
    }
    
    std::string full_path;
    int name_begin, name_end;
    bool check_slashes, check_dot;
    for(int i=0; i<file_names.size(); ++i) {
        full_path = file_names[i];
        // search for folder/directory separator and for the '.' extension separator
        name_begin = 0;
        check_slashes = true; 
        check_dot = true;
        for(int j=full_path.size()-1; j>=0; --j) {
            if(check_dot && full_path[j] == '.') {
                name_end = j;
                check_dot = false;
            }
            if(check_slashes && (full_path[j] == '\\' || full_path[j] == '/')) {
                name_begin = j+1;
                check_slashes = false;
            }
            if(!check_dot && !check_slashes) { break; }
        }
        file_names[i] = file_names[i].substr(name_begin, name_end-name_begin);
    }
    
    return file_names;
}



/*
Output writes width and height of the original BMP image.
Returns true on success, and false on failure.
Similar to getBMPPixels(), but does not parse the pixels.
*/
bool getBMPProperties(const std::string& path, uint32_t* bmp_width, uint32_t* bmp_height) {
    static constexpr size_t HEADER_SIZE = 54;
    std::ifstream bmp_file(path, std::ios::binary);
    if(!bmp_file) {
        std::cout << "Failed to open file: " << path << '\n';
        return false;
    }
    std::vector<char> header(HEADER_SIZE);
    bmp_file.read(header.data(), HEADER_SIZE);
    *bmp_width = *reinterpret_cast<uint32_t *>(&header[18]);
    if(*bmp_width % 4 !=0) {
        printf("Invalid BMP: width not multiple of 4.\n");
        return false;
    }
    *bmp_height = *reinterpret_cast<uint32_t *>(&header[22]);
    return true;
}

/*
Output writes to a SDL_Color matrix in RGB with alpha OPAQUE. Pixels are stored by "rows" so pixel at position (x,y) is stored in pixels[y][x] (reverse).
Output writes width and height of the original BMP image.
Returns true on success, and false on failure.
This function is very similar to Map::LoadMapFile(), but it's independent from the Map class and from the color values.
*/ 
bool getBMPPixels(const std::string& path, std::vector<std::vector<SDL_Color>>& pixels, uint32_t* bmp_width, uint32_t* bmp_height) {
    // https://stackoverflow.com/questions/9296059/read-pixel-value-in-bmp-file
    static constexpr size_t HEADER_SIZE = 54;

    std::ifstream bmp_file(path, std::ios::binary);
    if(!bmp_file) {
        std::cout << "Failed to open file: " << path << '\n';
        return false;
    }

    std::vector<char> header(HEADER_SIZE);
    bmp_file.read(header.data(), HEADER_SIZE);

    auto fileSize   = *reinterpret_cast<uint32_t *>(&header[2]);
    auto dataOffset = *reinterpret_cast<uint32_t *>(&header[10]);
    *bmp_width      = *reinterpret_cast<uint32_t *>(&header[18]);
    if(*bmp_width % 4 !=0) {
        printf("Invalid BMP: width not multiple of 4.\n");
        return false;
    }
    *bmp_height = *reinterpret_cast<uint32_t *>(&header[22]);
    auto depth  = *reinterpret_cast<uint16_t *>(&header[28]);

    auto dataSize = ((*bmp_width * 3 + 3) & (~3))  *  *bmp_height;
    std::vector<char> img(dataSize);
    bmp_file.read(img.data(), img.size());
    bmp_file.close();

    pixels.resize(*bmp_height);

    char temp;
    int line_offset = 0;
    int line_index = *bmp_width;
    int current_line;
    SDL_Color current_pixel = { 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE }; // just to set alpha to every pixel
    int tile_type;
    // reads "bottom" row first
    for(auto i = 0; i < dataSize; i += 3) {
        // BMP goes BGR
        current_pixel.r = img[i+2];
        current_pixel.g = img[i+1];
        current_pixel.b = img[i];

        if(line_index == *bmp_width) {
            ++line_offset;
            current_line = *bmp_height - line_offset;
            pixels[current_line].resize(*bmp_width);
            line_index = 0;
        }

        pixels[current_line][line_index] = current_pixel;
        ++line_index;
    }
    return true;
}

