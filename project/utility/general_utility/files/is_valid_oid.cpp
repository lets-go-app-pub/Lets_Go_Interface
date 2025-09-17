//
// Created by jeremiah on 11/3/21.
//

#include "general_utility.h"

QString isValidOid(const std::string& oid_string) {

    if (oid_string.size() != 24) {
        return QString("Error, ID is too ")
                .append(oid_string.size() < 24 ? QString("short") : QString("long"))
                .append('.');
    }

    if (!std::all_of(oid_string.begin(), oid_string.end(), ::isxdigit)) {
        return "Invalid character inside of ID.";
    }

    return "";
}