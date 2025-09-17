//
// Created by jeremiah on 10/30/21.
//

#include <ctime>

time_t extractTimeTFromDate(
        int year,
        int month,
        int dayOfMonth,
        int hour,
        int minutes,
        tm& setupTime
        ) {

    setupTime.tm_sec = 0;
    setupTime.tm_min = minutes;
    setupTime.tm_hour = hour;
    setupTime.tm_mday = dayOfMonth;
    setupTime.tm_mon = month - 1;
    setupTime.tm_year = year - 1900;
    setupTime.tm_isdst = -1;

    return mktime(&setupTime);
}