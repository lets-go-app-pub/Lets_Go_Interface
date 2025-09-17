//
// Created by jeremiah on 3/19/22.
//

#include <QString>

#include "set_default_icons.h"

void set_up_unknown_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_pick_up_sports_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_working_out_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_food_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_activities_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_social_drinking_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_sporting_entertainment_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_beauty_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_play_dates_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_concerts_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_poker_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_collectable_card_games_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_movies_and_tv_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_personal_development_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_video_games_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_diy_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_board_games_category(const std::function<void(const QString&)>& add_server_icon);

void set_up_icons_added_later(const std::function<void(const QString&)>& add_server_icon);

void set_default_icons(std::vector<SetServerIconsType>& set_server_icons) {

    auto add_server_icon = [&set_server_icons](const QString& basic_resource_path) {

        set_server_icons.emplace_back(
                DefaultIconParameters(
                        basic_resource_path
                ),
                std::make_shared<SetServerIcon>()
        );
    };

    set_up_unknown_category(add_server_icon);

    set_up_pick_up_sports_category(add_server_icon);

    set_up_working_out_category(add_server_icon);

    set_up_food_category(add_server_icon);

    set_up_activities_category(add_server_icon);

    set_up_social_drinking_category(add_server_icon);

    set_up_sporting_entertainment_category(add_server_icon);

    set_up_beauty_category(add_server_icon);

    set_up_play_dates_category(add_server_icon);

    set_up_concerts_category(add_server_icon);

    set_up_poker_category(add_server_icon);

    set_up_collectable_card_games_category(add_server_icon);

    set_up_movies_and_tv_category(add_server_icon);

    set_up_personal_development_category(add_server_icon);

    set_up_video_games_category(add_server_icon);

    set_up_diy_category(add_server_icon);

    set_up_board_games_category(add_server_icon);

    set_up_icons_added_later(add_server_icon);
}

void set_up_unknown_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 0
    add_server_icon(":/question_mark.png");
}

void set_up_pick_up_sports_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 1
    add_server_icon(":/baseball.png");

    //icon_index 2
    add_server_icon(":/basketball.png");

    //icon_index 3
    add_server_icon(":/football.png");

    //icon_index 4
    add_server_icon(":/golf.png");

    //icon_index 5
    add_server_icon(":/hockey.png");

    //icon_index 6
    add_server_icon(":/soccer.png");

    //icon_index 7
    add_server_icon(":/ultimate_frisbee.png");

    //icon_index 8
    add_server_icon(":/volleyball.png");
}

void set_up_working_out_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 9
    add_server_icon(":/crossfit.png");

    //icon_index 10
    add_server_icon(":/cycling.png");

    //icon_index 11
    add_server_icon(":/dance.png");

    //icon_index 12
    add_server_icon(":/mma.png");

    //icon_index 13
    add_server_icon(":/pilates.png");

    //icon_index 14
    add_server_icon(":/running.png");

    //icon_index 15
    add_server_icon(":/swimming.png");

    //icon_index 16
    add_server_icon(":/weightlifting.png");

    //icon_index 17
    add_server_icon(":/yoga.png");
}

void set_up_food_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 18
    add_server_icon(":/american.png");

    //icon_index 19
    add_server_icon(":/brunch.png");

    //icon_index 20
    add_server_icon(":/chinese.png");

    //icon_index 21
    add_server_icon(":/indian.png");

    //icon_index 22
    add_server_icon(":/italian.png");

    //icon_index 23
    add_server_icon(":/mexican.png");

    //icon_index 24
    add_server_icon(":/pho.png");

    //icon_index 25
    add_server_icon(":/sushi.png");

    //icon_index 26
    add_server_icon(":/thai.png");
}

void set_up_activities_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 27
    add_server_icon(":/band_jam_session.png");

    //icon_index 28
    add_server_icon(":/bar_hopping.png");

    //icon_index 29
    add_server_icon(":/boating.png");

    //icon_index 30
    add_server_icon(":/clubbing.png");

    //icon_index 31
    add_server_icon(":/dancing.png");

    //icon_index 32
    add_server_icon(":/hiking.png");

    //icon_index 33
    add_server_icon(":/shopping.png");

    //icon_index 34
    add_server_icon(":/skiing.png");

    //icon_index 35
    add_server_icon(":/snowboarding.png");

    //icon_index 36
    add_server_icon(":/water_activities.png");
}

void set_up_social_drinking_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 37
    add_server_icon(":/beer.png");

    //icon_index 38
    add_server_icon(":/spirits.png");

    //icon_index 39
    add_server_icon(":/wine.png");
}

void set_up_sporting_entertainment_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 40
    add_server_icon(":/boxing.png");

    //icon_index 41
    add_server_icon(":/tennis.png");
}

void set_up_beauty_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 42
    add_server_icon(":/hair.png");

    //icon_index 43
    add_server_icon(":/makeup.png");

    //icon_index 44
    add_server_icon(":/main_pedi.png");

    //icon_index 45
    add_server_icon(":/skin_care.png");
}

void set_up_play_dates_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 46
    add_server_icon(":/board_games.png");

    //icon_index 47
    add_server_icon(":/crafts.png");

    //icon_index 48
    add_server_icon(":/fun_in_the_kitchen.png");

    //icon_index 49
    add_server_icon(":/movies.png");

    //icon_index 50
    add_server_icon(":/outings.png");

    //icon_index 51
    add_server_icon(":/park_day.png");
}

void set_up_concerts_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 52
    add_server_icon(":/country.png");

    //icon_index 53
    add_server_icon(":/edm.png");

    //icon_index 54
    add_server_icon(":/hip_hop.png");

    //icon_index 55
    add_server_icon(":/indie.png");

    //icon_index 56
    add_server_icon(":/jazz.png");

    //icon_index 57
    add_server_icon(":/oldies.png");

    //icon_index 58
    add_server_icon(":/pop.png");

    //icon_index 59
    add_server_icon(":/rap.png");

    //icon_index 60
    add_server_icon(":/rock.png");
}

void set_up_poker_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 61
    add_server_icon(":/5_card_draw.png");

    //icon_index 62
    add_server_icon(":/7_card_stud.png");

    //icon_index 63
    add_server_icon(":/blind_man_bluff.png");

    //icon_index 64
    add_server_icon(":/omaha_hold_em.png");

    //icon_index 65
    add_server_icon(":/texas_hold_em.png");
}

void set_up_collectable_card_games_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 66
    add_server_icon(":/magic_the_gathering.png");

    //icon_index 67
    add_server_icon(":/pokemon.png");

    //icon_index 68
    add_server_icon(":/yu_gi_oh.png");
}

void set_up_movies_and_tv_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 69
    add_server_icon(":/action_adventure.png");

    //icon_index 70
    add_server_icon(":/comedy.png");

    //icon_index 71
    add_server_icon(":/drama.png");

    //icon_index 72
    add_server_icon(":/reality_tv.png");

    //icon_index 73
    add_server_icon(":/thriller_suspense.png");
}

void set_up_personal_development_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 74
    add_server_icon(":/emotional.png");

    //icon_index 75
    add_server_icon(":/intellectual.png");

    //icon_index 76
    add_server_icon(":/investing_and_finances.png");

    //icon_index 77
    add_server_icon(":/relational.png");
}

void set_up_video_games_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 78
    add_server_icon(":/battle_royale.png");

    //icon_index 79
    add_server_icon(":/digital_card_games.png");

    //icon_index 80
    add_server_icon(":/fps.png");

    //icon_index 81
    add_server_icon(":/mmorpg.png");

    //icon_index 82
    add_server_icon(":/moba.png");

    //icon_index 83
    add_server_icon(":/racing.png");

    //icon_index 84
    add_server_icon(":/sports.png");
}

void set_up_diy_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 85
    add_server_icon(":/cars.png");

    //icon_index 86
    add_server_icon(":/home_care.png");

    //icon_index 87
    add_server_icon(":/refurbish_items.png");

    //icon_index 88
    add_server_icon(":/restore_antiques.png");
}

void set_up_board_games_category(const std::function<void(const QString&)>& add_server_icon) {
    //icon_index 89
    add_server_icon(":/catan.png");

    //icon_index 90
    add_server_icon(":/checkers.png");

    //icon_index 91
    add_server_icon(":/chess.png");

    //icon_index 92
    add_server_icon(":/dominoes.png");

    //icon_index 93
    add_server_icon(":/dungeons_and_dragons.png");

    //icon_index 94
    add_server_icon(":/monopoly.png");

    //icon_index 95
    add_server_icon(":/pictionary.png");

    //icon_index 96
    add_server_icon(":/scrabble.png");
}

void set_up_icons_added_later(const std::function<void(const QString&)>& add_server_icon) {

    //icon_index 97
    add_server_icon(":/pickle.png");
}
