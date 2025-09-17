//
// Created by jeremiah on 3/19/22.
//

#pragma once

#include <set_server_icon.h>

struct DefaultIconParameters {
    const QString basic_resource_path;

    DefaultIconParameters() = delete;

    explicit DefaultIconParameters(
            QString basic_resource_path
            ) :
            basic_resource_path(std::move(basic_resource_path)) {}
};


struct SetServerIconsType {
    std::unique_ptr<DefaultIconParameters> parameters;
    std::shared_ptr<SetServerIcon> icons_ptr;

    SetServerIconsType() = delete;

    SetServerIconsType(
            DefaultIconParameters&& parameters,
            std::shared_ptr<SetServerIcon> icons_ptr) :
            parameters(std::make_unique<DefaultIconParameters>(std::move(parameters))),
            icons_ptr(std::move(icons_ptr)) {}

            friend void swap(SetServerIconsType& a, SetServerIconsType& b) {
        using std::swap; // bring in swap for built-in types

        swap(a.parameters, b.parameters);
        swap(a.icons_ptr, b.icons_ptr);
    }
};

void set_default_icons(std::vector<SetServerIconsType>& set_server_icons);