#pragma once
#include <cstdint>
#include <string>

uint8_t readBit(uint8_t byte, uint8_t bit);
void setBit(uint8_t* byte, uint8_t bit, uint8_t value);
std::string format_decimal(float f, int integer_precision, int decimal_precision, bool can_be_negative=true);
