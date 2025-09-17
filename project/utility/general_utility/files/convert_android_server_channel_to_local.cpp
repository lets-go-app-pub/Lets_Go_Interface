//
// Created by jeremiah on 2/16/22.
//

#include "general_utility.h"

std::string convertLocalToAndroidIfRequired(const std::string& address) {
    return address=="0.0.0.0"?"10.0.2.2":address;
}

std::string convertAndroidToLocalIfRequired(const std::string& address) {
    return address=="10.0.2.2"?"0.0.0.0":address;
}