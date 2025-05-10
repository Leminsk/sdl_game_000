#pragma once

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

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
