#pragma once
#include <cstdint>
#include <string>
#include <random>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <SDL2/SDL.h>


uint8_t readBit(uint8_t byte, uint8_t bit);
void setBit(uint8_t* byte, uint8_t bit, uint8_t value);

std::string format_decimal(float f, int integer_precision, int decimal_precision, bool can_be_negative=true);
std::string left_pad_int(int i, int total_digits, const char digit='0', bool can_be_negative=false);

int randomInt(std::mt19937 *generator, int min=0, int max=100);
float randomFloat(std::mt19937 *generator, float min=0.0f, float max=1.0f);

// https://stackoverflow.com/questions/216823/how-can-i-trim-a-stdstring
void ltrim(std::string &s);
void rtrim(std::string &s);
void trim(std::string &s);
std::string ltrim_copy(std::string s);
std::string rtrim_copy(std::string s);
std::string trim_copy(std::string s);

std::vector<std::string> getFileNamesInDirectory(const std::string& directory, const std::string& file_format="");
bool getPixelsBMP(const std::string& path, std::vector<std::vector<SDL_Color>>& pixels, uint32_t* layout_width, uint32_t* layout_height);
