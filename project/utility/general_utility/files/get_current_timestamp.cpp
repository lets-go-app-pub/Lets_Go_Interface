//
// Created by jeremiah on 9/29/21.
//

#include "general_utility.h"

std::chrono::milliseconds getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
}