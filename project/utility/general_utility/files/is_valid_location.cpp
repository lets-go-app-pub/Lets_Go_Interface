//
// Created by jeremiah on 3/24/23.
//

#include "general_utility.h"

bool isValidLocation(double longitude, double latitude) {
    if (longitude < -180.0 || 180.0 < longitude ||
        latitude <= -90.0 || 90.0 <= latitude) {
        return false;
    }

    return true;
}