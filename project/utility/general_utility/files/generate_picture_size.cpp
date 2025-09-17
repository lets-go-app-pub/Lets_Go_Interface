//
// Created by jeremiah on 9/18/21.
//

#include <QScreen>
#include <QGuiApplication>
#include "general_utility.h"

QSize generatePictureSize(int height, int width) {
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    //will make the picture as close as possible to 2/3 of the screen size
    // in increments of 1/20
    int factor = 1;
    int max_picture_height = screenGeometry.height() * 2 / 3;
    int divisor = 20;
    double divided_height = (double)height / divisor;
    double divided_width = (double)width / divisor;
    while (
            factor < divisor
            && divided_height * factor < max_picture_height
            ) {
        factor++;
    }

    return {(int)divided_width * factor, (int)divided_height * factor};
}