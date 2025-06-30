#include "utils.hpp"
#include <cstdint>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <random>

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