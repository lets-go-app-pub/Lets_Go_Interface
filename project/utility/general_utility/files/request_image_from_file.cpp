//
// Created by jeremiah on 3/25/23.
//

#include "general_utility.h"
#include <QWidget>
#include <QImageReader>
#include <QFileDialog>
#include <QMessageBox>

RequestImageFromFileReturnValues requestImageFromFile(
        QWidget* parent,
        const std::vector<std::string>& allowed_formats
) {

    if (allowed_formats.empty()) {
        return {};
    }

    QString filter_str = "Image File (";
    std::vector<std::string> lower_case_formats;
    std::for_each(allowed_formats.begin(), allowed_formats.end(), [&](const std::string& str) {
        std::string new_str = str;
        std::for_each(new_str.begin(), new_str.end(), [](char& c) {
            if (isalnum(c)) {
                c = (char) tolower(c);
            }
        });

        filter_str.append("*.").append(QString::fromStdString(new_str)).append(" ");
        lower_case_formats.emplace_back(new_str);
    });

    filter_str[filter_str.size() - 1] = ')';

    const QString file_name = QFileDialog::getOpenFileName(parent, "Select an image.", QDir::homePath(), filter_str);

    //if the action was canceled
    if (file_name.isNull()) {
        return RequestImageFromFileReturnValues{true};
    }

    QImageReader image_reader(file_name);
    image_reader.setAutoTransform(true);
    image_reader.setAutoDetectImageFormat(true);

    std::string image_format = image_reader.format().toStdString();

    std::for_each(image_format.begin(), image_format.end(), [](char& c) {
        if (isalnum(c)) {
            c = (char) tolower(c);
        }
    });

    if (std::find(lower_case_formats.begin(), lower_case_formats.end(), image_format) == lower_case_formats.end()) {
        QString allowed_formats_printable;

        std::for_each(lower_case_formats.begin(), lower_case_formats.end(), [&](const std::string& str) {
            allowed_formats_printable.append(" ").append(QString::fromStdString(str));
        });

        const QString error_string =
                QString("Invalid format for image file."
                        " Allowed format(s): ")
                        .append(allowed_formats_printable)
                        .append(". Found format is ")
                        .append(QString::fromStdString(image_format))
                        .append(".");

        QMessageBox::warning(
                parent,
                "Error",
                error_string
        );
        return {};
    }

    RequestImageFromFileReturnValues return_values{
            image_reader.read(),
            image_format,
            image_reader.errorString()
    };

    return return_values;
}