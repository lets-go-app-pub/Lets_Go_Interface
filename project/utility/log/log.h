//
// Created by jeremiah on 9/26/21.
//

#pragma once

template<typename ...Args>
void LOG(Args && ...args)
{
    (std::cout << ... << args);
    std::cout << '\n';
}