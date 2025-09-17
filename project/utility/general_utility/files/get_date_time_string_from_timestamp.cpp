//
// Created by jeremiah on 9/11/21.
//

#include <iomanip>

#include "general_utility.h"

std::string getDateTimeStringFromTimestamp(const std::chrono::milliseconds& timestamp) {

    time_t timeObject = timestamp.count() / 1000;
    tm* dateTimeStruct = std::localtime(&timeObject);

    std::ostringstream oss;
    //oss << std::put_time(dateTimeStruct, "%Y-%m-%d %H-%M-%S");
    oss << std::put_time(dateTimeStruct, "%m/%d/%Y %H:%M");
    return oss.str();

}