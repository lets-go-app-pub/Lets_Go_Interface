//
// Created by jeremiah on 11/16/21.
//

#include "general_utility.h"

void trimLeadingWhitespace(std::string& str) {
    str.erase(std::begin(str), std::find_if(std::begin(str), std::end(str), [](char c) -> bool {
        return !isspace(c);
    }));
}

void trimTrailingWhitespace(std::string& str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char c) -> bool {
        return !isspace(c);
    }).base(), str.end());
}

void trimWhitespace(std::string& str) {
    trimLeadingWhitespace(str);
    trimTrailingWhitespace(str);
}