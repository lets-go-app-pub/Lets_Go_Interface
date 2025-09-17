//
// Created by jeremiah on 3/19/22.
//

#pragma once

#include <set_server_activity.h>

#include <utility>

struct DefaultActivityParameters {
    const std::string display_name;
    int min_age;
    int category_index;
    int icon_index;
    const std::string icon_display_name;

    DefaultActivityParameters() = delete;

    DefaultActivityParameters(
            std::string display_name,
            int min_age,
            int category_index,
            int icon_index,
            std::string icon_display_name = ""
            ) :
            display_name(std::move(display_name)),
            min_age(min_age),
            category_index(category_index),
            icon_index(icon_index),
            icon_display_name(std::move(icon_display_name)) {}
};

struct SetServerActivitiesType {
    std::unique_ptr<DefaultActivityParameters> parameters;
    std::shared_ptr<SetServerActivity> activities_ptr;

    SetServerActivitiesType() = delete;

    SetServerActivitiesType(
            DefaultActivityParameters&& parameters,
            std::shared_ptr<SetServerActivity> activities_ptr) :
            parameters(std::make_unique<DefaultActivityParameters>(std::move(parameters))),
            activities_ptr(std::move(activities_ptr)) {}

            friend void swap(SetServerActivitiesType& a, SetServerActivitiesType& b) {
        using std::swap; // bring in swap for built-in types

        swap(a.parameters, b.parameters);
        swap(a.activities_ptr, b.activities_ptr);
    }
};

void set_default_activities(std::vector<SetServerActivitiesType>& set_server_activities);