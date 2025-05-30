#include "utils.hpp"
#include <cstdint>
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

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

std::string format_decimal(float f, int integer_precision, int decimal_precision) {

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
    // add 1 for the '-' sign
    int spaces = 1 + integer_precision - (integer_digits + is_negative);
    for(int i=0; i<spaces; ++i) { space_padding += ' '; }

    return space_padding + number_str;
}