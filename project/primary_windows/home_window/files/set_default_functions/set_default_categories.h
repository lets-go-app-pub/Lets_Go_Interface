//
// Created by jeremiah on 3/19/22.
//

#pragma once

#include <set_server_category.h>

struct DefaultCategoryParameters {
    const std::string category_name;
    double order_number;
    int min_age;
    const std::string color;
    const std::string icon_display_name;

    DefaultCategoryParameters() = delete;

    DefaultCategoryParameters(
            std::string category_name,
            double order_number,
            int min_age,
            std::string color,
            std::string icon_display_name = ""
    ) :
            category_name(std::move(category_name)),
            order_number(order_number),
            min_age(min_age),
            color(std::move(color)),
            icon_display_name(std::move(icon_display_name)) {}
};

struct SetServerCategoriesType {
    std::unique_ptr<DefaultCategoryParameters> parameters;
    std::shared_ptr<SetServerCategory> category_ptr;

    SetServerCategoriesType() = delete;

    SetServerCategoriesType(
            DefaultCategoryParameters&& parameters,
            std::shared_ptr<SetServerCategory> category_ptr) :
            parameters(std::make_unique<DefaultCategoryParameters>(std::move(parameters))),
            category_ptr(std::move(category_ptr)) {}

    friend void swap(SetServerCategoriesType& a, SetServerCategoriesType& b) {
        using std::swap; // bring in swap for built-in types

        swap(a.parameters, b.parameters);
        swap(a.category_ptr, b.category_ptr);
    }
};

void set_default_categories(std::vector<SetServerCategoriesType>& set_server_categories);