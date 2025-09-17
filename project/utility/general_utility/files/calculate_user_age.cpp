//
// Created by jeremiah on 9/12/21.
//

#include "general_utility.h"

//year needs to be a (4 digit most likely) value ex: 20xx
//day of year needs to be a value [0-365] (same as tm_yday, ideally extracted from tm_yday)
//NOTE: returns -1 if function fails
int calculateAge(const std::chrono::milliseconds& currentTime, int birthYear, int birthDayOfYear) {

    const time_t timeObject = currentTime.count() / 1000;

    tm* dateTimeStruct = gmtime(&timeObject);

    if (dateTimeStruct == nullptr || dateTimeStruct->tm_year == 0) { //failed to calculate proper date time
        //handle the error in calling function
        return -1;
    }

    int currentYear = dateTimeStruct->tm_year + 1900;

    if (birthYear < 1900 || currentYear < birthYear
    || birthDayOfYear < 0 || 365 < birthDayOfYear) {
        return -1;
    }

    //LEAP YEARS
    //if a year is divisible by 4
    //AND it is not divisible by 100
    //UNLESS it is divisible by 400
    //it is a leap year
    //NOTE: 2000 is a leap year and its not like we are going to have multiple centuries this thing spans... but I will leave the logic line below for completion sake
    //if((today.get(Calendar.YEAR) % 4 == 0 && today.get(Calendar.YEAR) % 100 != 0) || (today.get(Calendar.YEAR) % 4 == 0 && today.get(Calendar.YEAR) % 400 == 0))

    int age = currentYear - birthYear;

    if (0 > dateTimeStruct->tm_yday - birthDayOfYear) {
        age--;
    }

    return age;
}