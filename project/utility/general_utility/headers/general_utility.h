#include <utility>
#include <string>
#include <chrono>
#include <QSize>
#include <QImage>
#include <grpcpp/channel.h>
//
// Created by jeremiah on 9/10/21.
//

#pragma once

struct BsoncxxOIDWrapper {

    explicit BsoncxxOIDWrapper(std::string _oid_string): oid_string(std::move(_oid_string)) {}

    std::string oid_string;
};

std::string getDateTimeStringFromTimestamp(const std::chrono::milliseconds& timestamp);

int calculateAge(const std::chrono::milliseconds& currentTime, int birthYear, int birthDayOfYear);

tm initializeTmByDate(
        int year,
        int month,
        int dayOfMonth,
        int hour = 0,
        int minutes = 0
        );

time_t extractTimeTFromDate(
        int year,
        int month,
        int dayOfMonth,
        int hour,
        int minutes,
        tm& setupTime
        );

QSize generatePictureSize(int height, int width);

std::chrono::milliseconds getCurrentTimestamp();

//returns empty string if valid, error message if invalid
QString isValidOid(const std::string& oid_string);

//returns true if location is valid, false if not
bool isValidLocation(double longitude, double latitude);

//remove leading whitespace
void trimLeadingWhitespace(std::string& str);

//remove trailing whitespace
void trimTrailingWhitespace(std::string& str);

//remove leading and trailing whitespace
void trimWhitespace(std::string& str);

//if value is local address, convert it to the android emulator local networking address
std::string convertLocalToAndroidIfRequired(const std::string& address);

//if value is android emulator local networking address, convert it to the local address
std::string convertAndroidToLocalIfRequired(const std::string& address);

//creates a grpc channel with proper ssl credentials and channel arguments
std::shared_ptr<grpc::Channel> setupGrpcChannel(const std::string& address, int port);

struct RequestImageFromFileReturnValues {
    bool canceled = false;
    QImage image;
    std::string image_format;
    QString error_string;

    RequestImageFromFileReturnValues() = default;

    explicit RequestImageFromFileReturnValues(
            bool _canceled
    ) : canceled(_canceled) {}

    RequestImageFromFileReturnValues(
            QImage&& _image,
            std::string _image_format,
            QString _error_string
    ) : image(std::move(_image)),
        image_format(std::move(_image_format)),
        error_string(std::move(_error_string)) {}
};

RequestImageFromFileReturnValues requestImageFromFile(
        QWidget* parent,
        const std::vector<std::string>& allowed_formats
);