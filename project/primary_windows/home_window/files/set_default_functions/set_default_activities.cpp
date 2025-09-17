//
// Created by jeremiah on 3/19/22.
//

#include "set_default_activities.h"

void set_up_unknown_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_pick_up_sports_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_working_out_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_food_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_activities_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_social_drinking_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_sporting_entertainment_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_beauty_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_play_dates_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_concerts_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_poker_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void
set_up_collectable_card_games_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_movies_and_tv_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_personal_development_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_video_games_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_diy_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_board_games_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

void set_up_activities_added_later(const std::function<void(DefaultActivityParameters&&)>& add_server_activity);

/** IMPORTANT: When these are changed they must also be changed on the Android client inside Utility.kt getAllPreLoadedIcons(). **/
void set_default_activities(std::vector<SetServerActivitiesType>& set_server_activities) {

    auto add_server_activity = [&set_server_activities](
            DefaultActivityParameters&& params) {

        set_server_activities.emplace_back(
                std::move(params),
                std::make_shared<SetServerActivity>()
        );
    };

    set_up_unknown_category(add_server_activity);

    set_up_pick_up_sports_category(add_server_activity);

    set_up_working_out_category(add_server_activity);

    set_up_food_category(add_server_activity);

    set_up_activities_category(add_server_activity);

    set_up_social_drinking_category(add_server_activity);

    set_up_sporting_entertainment_category(add_server_activity);

    set_up_beauty_category(add_server_activity);

    set_up_play_dates_category(add_server_activity);

    set_up_concerts_category(add_server_activity);

    set_up_poker_category(add_server_activity);

    set_up_collectable_card_games_category(add_server_activity);

    set_up_movies_and_tv_category(add_server_activity);

    set_up_personal_development_category(add_server_activity);

    set_up_video_games_category(add_server_activity);

    set_up_diy_category(add_server_activity);

    set_up_board_games_category(add_server_activity);

    set_up_activities_added_later(add_server_activity);
}

void set_up_unknown_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 0
    add_server_activity(
            DefaultActivityParameters(
                    "Unknown",
                    13,
                    0,
                    0
            )
    );
}

void set_up_pick_up_sports_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 1
    add_server_activity(
            DefaultActivityParameters(
                    "Baseball",
                    13,
                    1,
                    1
            )
    );

    //activity_index 2
    add_server_activity(
            DefaultActivityParameters(
                    "Basketball",
                    13,
                    1,
                    2
            )
    );

    //activity_index 3
    add_server_activity(
            DefaultActivityParameters(
                    "Football",
                    13,
                    1,
                    3
            )
    );

    //activity_index 4
    add_server_activity(
            DefaultActivityParameters(
                    "Golf",
                    13,
                    1,
                    4
            )
    );

    //activity_index 5
    add_server_activity(
            DefaultActivityParameters(
                    "Hockey",
                    13,
                    1,
                    5
            )
    );

    //activity_index 6
    add_server_activity(
            DefaultActivityParameters(
                    "Soccer",
                    13,
                    1,
                    6
            )
    );

    //activity_index 7
    add_server_activity(
            DefaultActivityParameters(
                    "Ultimate Frisbee",
                    13,
                    1,
                    7
            )
    );

    //activity_index 8
    add_server_activity(
            DefaultActivityParameters(
                    "Volleyball",
                    13,
                    1,
                    8
            )
    );
}

void set_up_working_out_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 9
    add_server_activity(
            DefaultActivityParameters(
                    "Crossfit",
                    13,
                    2,
                    9
            )
    );

    //activity_index 10
    add_server_activity(
            DefaultActivityParameters(
                    "Cycling",
                    13,
                    2,
                    10
            )
    );

    //activity_index 11
    add_server_activity(
            DefaultActivityParameters(
                    "Dance",
                    13,
                    2,
                    11
            )
    );

    //activity_index 12
    add_server_activity(
            DefaultActivityParameters(
                    "MMA",
                    13,
                    2,
                    12
            )
    );

    //activity_index 13
    add_server_activity(
            DefaultActivityParameters(
                    "Pilates",
                    13,
                    2,
                    13
            )
    );

    //activity_index 14
    add_server_activity(
            DefaultActivityParameters(
                    "Running",
                    13,
                    2,
                    14
            )
    );

    //activity_index 15
    add_server_activity(
            DefaultActivityParameters(
                    "Swimming",
                    13,
                    2,
                    15
            )
    );

    //activity_index 16
    add_server_activity(
            DefaultActivityParameters(
                    "Weight Training",
                    13,
                    2,
                    16
            )
    );

    //activity_index 17
    add_server_activity(
            DefaultActivityParameters(
                    "Yoga",
                    13,
                    2,
                    17
            )
    );
}

void set_up_food_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {

    //activity_index 18
    add_server_activity(
            DefaultActivityParameters(
                    "American",
                    13,
                    3,
                    18
            )
    );

    //activity_index 19
    add_server_activity(
            DefaultActivityParameters(
                    "Brunch",
                    13,
                    3,
                    19
            )
    );

    //activity_index 20
    add_server_activity(
            DefaultActivityParameters(
                    "Chinese",
                    13,
                    3,
                    20
            )
    );

    //activity_index 21
    add_server_activity(
            DefaultActivityParameters(
                    "Indian",
                    13,
                    3,
                    21
            )
    );

    //activity_index 22
    add_server_activity(
            DefaultActivityParameters(
                    "Italian",
                    13,
                    3,
                    22
            )
    );

    //activity_index 23
    add_server_activity(
            DefaultActivityParameters(
                    "Mexican",
                    13,
                    3,
                    23
            )
    );

    //activity_index 24
    add_server_activity(
            DefaultActivityParameters(
                    "Pho",
                    13,
                    3,
                    24
            )
    );

    //activity_index 25
    add_server_activity(
            DefaultActivityParameters(
                    "Sushi",
                    13,
                    3,
                    25
            )
    );

    //activity_index 26
    add_server_activity(
            DefaultActivityParameters(
                    "Thai",
                    13,
                    3,
                    26
            )
    );
}

void set_up_activities_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 27
    add_server_activity(
            DefaultActivityParameters(
                    "Band Jam Session",
                    13,
                    4,
                    27
            )
    );

    //activity_index 28
    add_server_activity(
            DefaultActivityParameters(
                    "Bar Hopping",
                    21,
                    4,
                    28
            )
    );

    //activity_index 29
    add_server_activity(
            DefaultActivityParameters(
                    "Boating",
                    13,
                    4,
                    29
            )
    );

    //activity_index 30
    add_server_activity(
            DefaultActivityParameters(
                    "Clubbing",
                    21,
                    4,
                    30
            )
    );

    //activity_index 31
    add_server_activity(
            DefaultActivityParameters(
                    "Dancing",
                    13,
                    4,
                    31
            )
    );

    //activity_index 32
    add_server_activity(
            DefaultActivityParameters(
                    "Hiking",
                    13,
                    4,
                    32
            )
    );

    //activity_index 33
    add_server_activity(
            DefaultActivityParameters(
                    "Shopping",
                    13,
                    4,
                    33
            )
    );

    //activity_index 34
    add_server_activity(
            DefaultActivityParameters(
                    "Skiing",
                    13,
                    4,
                    34
            )
    );

    //activity_index 35
    add_server_activity(
            DefaultActivityParameters(
                    //NOTE: the ' ' is important here so that Android will separate it at the '-'
                    "Snowboarding",
                    13,
                    4,
                    35,
                    "Snow-\nboarding"
            )
    );

    //activity_index 36
    add_server_activity(
            DefaultActivityParameters(
                    "Water Activities",
                    13,
                    4,
                    36
            )
    );

}

void set_up_social_drinking_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 37
    add_server_activity(
            DefaultActivityParameters(
                    "Beer",
                    21,
                    5,
                    37
            )
    );

    //activity_index 38
    add_server_activity(
            DefaultActivityParameters(
                    "Spirits",
                    21,
                    5,
                    38
            )
    );

    //activity_index 39
    add_server_activity(
            DefaultActivityParameters(
                    "Wine",
                    21,
                    5,
                    39
            )
    );
}

void set_up_sporting_entertainment_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 40
    add_server_activity(
            DefaultActivityParameters(
                    "Baseball",
                    13,
                    6,
                    1
            )
    );

    //activity_index 41
    add_server_activity(
            DefaultActivityParameters(
                    "Basketball",
                    13,
                    6,
                    2
            )
    );

    //activity_index 42
    add_server_activity(
            DefaultActivityParameters(
                    "Boxing",
                    13,
                    6,
                    40
            )
    );

    //activity_index 43
    add_server_activity(
            DefaultActivityParameters(
                    "Football",
                    13,
                    6,
                    3
            )
    );

    //activity_index 44
    add_server_activity(
            DefaultActivityParameters(
                    "Golf",
                    13,
                    6,
                    4
            )
    );

    //activity_index 45
    add_server_activity(
            DefaultActivityParameters(
                    "Hockey",
                    13,
                    6,
                    5
            )
    );

    //activity_index 46
    add_server_activity(
            DefaultActivityParameters(
                    "MMA",
                    13,
                    6,
                    12
            )
    );

    //activity_index 47
    add_server_activity(
            DefaultActivityParameters(
                    "Soccer",
                    13,
                    6,
                    6
            )
    );

    //activity_index 48
    add_server_activity(
            DefaultActivityParameters(
                    "Tennis",
                    13,
                    6,
                    41
            )
    );
}

void set_up_beauty_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 49
    add_server_activity(
            DefaultActivityParameters(
                    "Hair",
                    13,
                    8,
                    42
            )
    );

    //activity_index 50
    add_server_activity(
            DefaultActivityParameters(
                    "Makeup",
                    13,
                    8,
                    43
            )
    );

    //activity_index 51
    add_server_activity(
            DefaultActivityParameters(
                    "Mani Pedi",
                    13,
                    8,
                    44
            )
    );

    //activity_index 52
    add_server_activity(
            DefaultActivityParameters(
                    "Skin Care",
                    13,
                    8,
                    45
            )
    );

}

void set_up_play_dates_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 53
    add_server_activity(
            DefaultActivityParameters(
                    "Board Games",
                    18,
                    7,
                    46
            )
    );

    //activity_index 54
    add_server_activity(
            DefaultActivityParameters(
                    "Crafts",
                    18,
                    7,
                    47
            )
    );

    //activity_index 55
    add_server_activity(
            DefaultActivityParameters(
                    "Fun In The Kitchen",
                    18,
                    7,
                    48
            )
    );

    //activity_index 56
    add_server_activity(
            DefaultActivityParameters(
                    "Movies",
                    18,
                    7,
                    49
            )
    );

    //activity_index 57
    add_server_activity(
            DefaultActivityParameters(
                    "Outings",
                    18,
                    7,
                    50
            )
    );

    //activity_index 58
    add_server_activity(
            DefaultActivityParameters(
                    "Park Day",
                    18,
                    7,
                    51
            )
    );
}

void set_up_concerts_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 59
    add_server_activity(
            DefaultActivityParameters(
                    "Country",
                    13,
                    9,
                    52
            )
    );

    //activity_index 60
    add_server_activity(
            DefaultActivityParameters(
                    "EDM",
                    13,
                    9,
                    53
            )
    );

    //activity_index 61
    add_server_activity(
            DefaultActivityParameters(
                    "Hip-Hop",
                    13,
                    9,
                    54
            )
    );

    //activity_index 62
    add_server_activity(
            DefaultActivityParameters(
                    "Indie",
                    13,
                    9,
                    55
            )
    );

    //activity_index 63
    add_server_activity(
            DefaultActivityParameters(
                    "Jazz",
                    13,
                    9,
                    56
            )
    );

    //activity_index 64
    add_server_activity(
            DefaultActivityParameters(
                    "Oldies",
                    13,
                    9,
                    57
            )
    );

    //activity_index 65
    add_server_activity(
            DefaultActivityParameters(
                    "Pop",
                    13,
                    9,
                    58
            )
    );

    //activity_index 66
    add_server_activity(
            DefaultActivityParameters(
                    "Rap",
                    13,
                    9,
                    59
            )
    );

    //activity_index 67
    add_server_activity(
            DefaultActivityParameters(
                    "Rock",
                    13,
                    9,
                    60
            )
    );

}

void set_up_poker_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 68
    add_server_activity(
            DefaultActivityParameters(
                    "5 Card Draw",
                    18,
                    10,
                    61
            )
    );

    //activity_index 69
    add_server_activity(
            DefaultActivityParameters(
                    "7 Card Stud",
                    18,
                    10,
                    62
            )
    );

    //activity_index 70
    add_server_activity(
            DefaultActivityParameters(
                    "Blind Man's Bluff",
                    18,
                    10,
                    63
            )
    );

    //activity_index 71
    add_server_activity(
            DefaultActivityParameters(
                    "Omaha Hold'em",
                    18,
                    10,
                    64
            )
    );

    //activity_index 72
    add_server_activity(
            DefaultActivityParameters(
                    "Texas Hold'em",
                    18,
                    10,
                    65
            )
    );
}

void set_up_collectable_card_games_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 73
    add_server_activity(
            DefaultActivityParameters(
                    "Magic The Gathering",
                    13,
                    11,
                    66
            )
    );

    //activity_index 74
    add_server_activity(
            DefaultActivityParameters(
                    "Pokemon",
                    13,
                    11,
                    67
            )
    );

    //activity_index 75
    add_server_activity(
            DefaultActivityParameters(
                    "Yu-Gi-Oh!",
                    13,
                    11,
                    68
            )
    );

}

void set_up_movies_and_tv_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 76
    add_server_activity(
            DefaultActivityParameters(
                    "Action/Adventure",
                    13,
                    12,
                    69,
                    "Action / Adventure"
            )
    );

    //activity_index 77
    add_server_activity(
            DefaultActivityParameters(
                    "Comedy",
                    13,
                    12,
                    70
            )
    );

    //activity_index 78
    add_server_activity(
            DefaultActivityParameters(
                    "Drama",
                    13,
                    12,
                    71
            )
    );

    //activity_index 79
    add_server_activity(
            DefaultActivityParameters(
                    "Reality TV",
                    13,
                    12,
                    72
            )
    );

    //activity_index 80
    add_server_activity(
            DefaultActivityParameters(
                    "Thriller/Suspense",
                    13,
                    12,
                    73,
                    "Thriller / Suspense"
            )
    );

}

void set_up_personal_development_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 81
    add_server_activity(
            DefaultActivityParameters(
                    "Emotional",
                    13,
                    13,
                    74
            )
    );

    //activity_index 82
    add_server_activity(
            DefaultActivityParameters(
                    "Intellectual",
                    13,
                    13,
                    75
            )
    );

    //activity_index 83
    add_server_activity(
            DefaultActivityParameters(
                    "Investing & Finances",
                    13,
                    13,
                    76
            )
    );

    //activity_index 84
    add_server_activity(
            DefaultActivityParameters(
                    "Relational",
                    13,
                    13,
                    77
            )
    );

}

void set_up_video_games_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 85
    add_server_activity(
            DefaultActivityParameters(
                    "Battle Royale",
                    13,
                    14,
                    78
            )
    );

    //activity_index 86
    add_server_activity(
            DefaultActivityParameters(
                    "Digital Card Games",
                    13,
                    14,
                    79
            )
    );

    //activity_index 87
    add_server_activity(
            DefaultActivityParameters(
                    "FPS",
                    13,
                    14,
                    80
            )
    );

    //activity_index 88
    add_server_activity(
            DefaultActivityParameters(
                    "MMORPG",
                    13,
                    14,
                    81
            )
    );

    //activity_index 89
    add_server_activity(
            DefaultActivityParameters(
                    "MOBA",
                    13,
                    14,
                    82
            )
    );

    //activity_index 90
    add_server_activity(
            DefaultActivityParameters(
                    "Racing",
                    13,
                    14,
                    83
            )
    );

    //activity_index 91
    add_server_activity(
            DefaultActivityParameters(
                    "Sports",
                    13,
                    14,
                    84
            )
    );

}

void set_up_diy_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 92
    add_server_activity(
            DefaultActivityParameters(
                    "Cars",
                    13,
                    15,
                    85
            )
    );

    //activity_index 93
    add_server_activity(
            DefaultActivityParameters(
                    "Home Care",
                    13,
                    15,
                    86
            )
    );

    //activity_index 94
    add_server_activity(
            DefaultActivityParameters(
                    "Item Refurbishment",
                    13,
                    15,
                    87,
                    "Refurbish Items"
            )
    );

    //activity_index 95
    add_server_activity(
            DefaultActivityParameters(
                    "Antique Restoration",
                    13,
                    15,
                    88,
                    "Restore Antiques"
            )
    );
}

void set_up_board_games_category(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {
    //activity_index 96
    add_server_activity(
            DefaultActivityParameters(
                    "Catan",
                    13,
                    16,
                    89
            )
    );

    //activity_index 97
    add_server_activity(
            DefaultActivityParameters(
                    "Checkers",
                    13,
                    16,
                    90
            )
    );

    //activity_index 98
    add_server_activity(
            DefaultActivityParameters(
                    "Chess",
                    13,
                    16,
                    91
            )
    );

    //activity_index 99
    add_server_activity(
            DefaultActivityParameters(
                    "Dominoes",
                    13,
                    16,
                    92
            )
    );

    //activity_index 100
    add_server_activity(
            DefaultActivityParameters(
                    "Dungeons & Dragons",
                    13,
                    16,
                    93
            )
    );

    //activity_index 101
    add_server_activity(
            DefaultActivityParameters(
                    "Monopoly",
                    13,
                    16,
                    94
            )
    );

    //activity_index 102
    add_server_activity(
            DefaultActivityParameters(
                    "Pictionary",
                    13,
                    16,
                    95
            )
    );

    //activity_index 103
    add_server_activity(
            DefaultActivityParameters(
                    "Scrabble",
                    13,
                    16,
                    96
            )
    );

}

void set_up_activities_added_later(const std::function<void(DefaultActivityParameters&&)>& add_server_activity) {

    //activity_index 104
    add_server_activity(
            DefaultActivityParameters(
                    "PickleBall",
                    13,
                    1,
                    97
            )
    );
}
