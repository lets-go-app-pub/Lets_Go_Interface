//
// Created by jeremiah on 9/12/21.
//

#include <iostream>
#include "general_utility.h"

tm initializeTmByDate(
        int year,
        int month,
        int dayOfMonth,
        int hour,
        int minutes
) {

    tm setupTime{};

    time_t lTimeEpoch = extractTimeTFromDate(
            year,
            month,
            dayOfMonth,
            hour,
            minutes,
            setupTime
    );

    return *gmtime(&lTimeEpoch);
}