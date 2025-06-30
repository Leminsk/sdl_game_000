#pragma once
#include <cstdint>
#include <string>
#include <random>

uint8_t readBit(uint8_t byte, uint8_t bit);
void setBit(uint8_t* byte, uint8_t bit, uint8_t value);
std::string format_decimal(float f, int integer_precision, int decimal_precision, bool can_be_negative=true);
std::string left_pad_int(int i, int total_digits, const char digit='0', bool can_be_negative=false);
int randomInt(std::mt19937 *generator, int min=0, int max=100);
float randomFloat(std::mt19937 *generator, float min=0.0f, float max=1.0f);