//
// Created by jeremiah on 3/19/22.
//

#include "set_default_categories.h"

void set_default_categories(std::vector<SetServerCategoriesType>& set_server_categories) {

    auto add_server_category = [&set_server_categories](DefaultCategoryParameters&& params) {
        set_server_categories.emplace_back(
                std::move(params),
                std::make_shared<SetServerCategory>()
        );
    };

    //category_index 0
    add_server_category(
            DefaultCategoryParameters(
                    "Unknown",
                    0,
                    121,
                    "#000000"
            )
    );

    //category_index 1
    add_server_category(
            DefaultCategoryParameters(
                    "Pick-Up Sports",
                    1,
                    13,
                    "#a5002d"
            )
    );

    //category_index 2
    add_server_category(
            DefaultCategoryParameters(
                    "Working Out",
                    2,
                    13,
                    "#c83d00"
            )
    );

    //category_index 3
    add_server_category(
            DefaultCategoryParameters(
                    "Food",
                    3,
                    13,
                    "#228B22"
            )
    );

    //category_index 4
    add_server_category(
            DefaultCategoryParameters(
                    "Adventures",
                    4,
                    13,
                    "#2188ff"
            )
    );

    //category_index 5
    add_server_category(
            DefaultCategoryParameters(
                    "Social Drinking & Tastings",
                    5,
                    21,
                    "#ff5f42"
            )
    );

    //category_index 6
    add_server_category(
            DefaultCategoryParameters(
                    "Sporting Entertainment",
                    6,
                    13,
                    "#9e64b4"
            )
    );

    //category_index 7
    add_server_category(
            DefaultCategoryParameters(
                    "Play Dates",
                    7,
                    18,
                    "#5aa900"
            )
    );

    //category_index 8
    add_server_category(
            DefaultCategoryParameters(
                    "Beauty",
                    8,
                    13,
                    "#FF1694"
            )
    );

    //category_index 9
    add_server_category(
            DefaultCategoryParameters(
                    "Concerts",
                    9,
                    13,
                    "#003a8d"
            )
    );

    //category_index 10
    add_server_category(
            DefaultCategoryParameters(
                    "Poker",
                    10,
                    18,
                    "#b4554b"
            )
    );

    //category_index 11
    add_server_category(
            DefaultCategoryParameters(
                    "Collectable Card Games",
                    11,
                    13,
                    "#a400b3"
            )
    );

    //category_index 12
    add_server_category(
            DefaultCategoryParameters(
                    "Movies & TV",
                    12,
                    13,
                    "#c7953d"
            )
    );

    //category_index 13
    add_server_category(
            DefaultCategoryParameters(
                    "Personal Development",
                    13,
                    13,
                    "#6500ff"
            )
    );

    //category_index 14
    add_server_category(
            DefaultCategoryParameters(
                    "Video Games",
                    14,
                    13,
                    "#0d35f9"
            )
    );

    //category_index 15
    add_server_category(
            DefaultCategoryParameters(
                    "DIY",
                    15,
                    13,
                    "#fe6400"
            )
    );

    //category_index 16
    add_server_category(
            DefaultCategoryParameters(
                    "Board Games",
                    16,
                    13,
                    "#e555ff"
            )
    );

}