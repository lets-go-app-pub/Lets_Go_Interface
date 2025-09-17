//
// Created by jeremiah on 9/18/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_set_default_picture_window.h" resolved

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <user_login_info.h>
#include <general_utility.h>
#include <QBuffer>
#include <globals.h>
#include "set_default_picture_window.h"

#include "homewindow.h"
#include "ui_set_default_picture_window.h"

SetDefaultPictureWindow::SetDefaultPictureWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::SetDefaultPictureWindow) {
    ui->setupUi(this);

    QSize label_size = generatePictureSize(JPEG_HEIGHT, JPEG_WIDTH);
    ui->pictureLabel->setScaledContents(true);
    ui->pictureLabel->setFixedSize(label_size);

    connect(&set_default_picture, &SetDefaultPicture::signal_requestSuccessfullyCompleted, this,
            &SetDefaultPictureWindow::slot_setDefaultPictureCompleted);
    connect(&set_default_picture, &SetDefaultPicture::signal_sendWarning, this,
            &SetDefaultPictureWindow::slot_sendWarning);
    connect(&set_default_picture, &SetDefaultPicture::signal_requestCanceled, this,
            &SetDefaultPictureWindow::slot_requestCanceled);

}

void SetDefaultPictureWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void SetDefaultPictureWindow::slot_setDefaultPictureCompleted(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }

    /*
    std::scoped_lock<std::mutex> lock(default_picture_mutex);
    default_picture_bytes = std::string(image_bytes.constData(), image_bytes.size());
    default_picture_size = (int)image_bytes.size();
    image_bytes.clear();
    thumbnail_image_bytes.clear();
     */
}

void SetDefaultPictureWindow::slot_sendWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& completed_lambda
) {

    if (completed_lambda) {
        completed_lambda();
    }
    std::cout << "title: " << title.toStdString();
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(this,
                         title,
                         text
    );
}

SetDefaultPictureWindow::~SetDefaultPictureWindow() {
    delete ui;
}

void SetDefaultPictureWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

void SetDefaultPictureWindow::on_selectPicturePushButton_clicked() {

    const RequestImageFromFileReturnValues request_image_return = requestImageFromFile(
            this,
            std::vector<std::string>{"png", "jpg", "jpeg"}
    );

    if(request_image_return.canceled) {
        return;
    }

    const QImage& raw_image = request_image_return.image;

    if (raw_image.isNull()) {
        const QString error_string = QString("Error reading image from file.\nReturned error: '")
                .append(request_image_return.error_string)
                .append("'");

        QMessageBox::warning(
                this,
                "Error",
                error_string
        );
        return;
    }

    QImage image = raw_image.scaled(
            JPEG_HEIGHT,
            JPEG_WIDTH,
            Qt::AspectRatioMode::KeepAspectRatio
    );

    ui->pictureLabel->setPixmap(QPixmap::fromImage(image));

    image_bytes.clear();

    QBuffer buffer_img(&image_bytes);
    buffer_img.open(QIODevice::WriteOnly);

    int quality_value = -1;
    if(request_image_return.image_format != "png") {
        quality_value = JPEG_IMAGE_QUALITY_VALUE;
    }

    image.save(&buffer_img, request_image_return.image_format.c_str(), quality_value);

    std::cout << "img size: " << image_bytes.size() / 1024 << "Kb\n";

    image = image.scaled(
            JPEG_THUMBNAIL_CROPPED_HEIGHT,
            JPEG_THUMBNAIL_CROPPED_WIDTH,
            Qt::AspectRatioMode::KeepAspectRatio
    );

    thumbnail_image_bytes.clear();

    QBuffer buffer_thumbnail(&thumbnail_image_bytes);
    buffer_thumbnail.open(QIODevice::WriteOnly);

    image.save(&buffer_thumbnail, request_image_return.image_format.c_str(), quality_value);

    std::cout << "thumbnail size: " << thumbnail_image_bytes.size() / 1024 << "Kb\n";
}

void SetDefaultPictureWindow::on_sendPushButton_clicked() {

    if (image_bytes.isEmpty() || thumbnail_image_bytes.isEmpty()) {
        QMessageBox::warning(this,
                             "Error",
                             "Please select a picture first."
        );
        return;
    }

    ui->sendPushButton->setEnabled(false);
    ui->defaultPushButton->setEnabled(false);
    ui->selectPicturePushButton->setEnabled(false);

    set_default_picture.runSetDefaultPicture(
            image_bytes,
            thumbnail_image_bytes,
            [&]() {
                ui->sendPushButton->setEnabled(true);
                ui->defaultPushButton->setEnabled(true);
                ui->selectPicturePushButton->setEnabled(true);
            });

}

void SetDefaultPictureWindow::on_defaultPushButton_clicked() {
    /*
    if (default_picture_bytes.empty()) {
        QMessageBox::warning(this,
                             "Error",
                             "No default picture stored on server"
                             );
        return;
    } else if (default_picture_bytes.size() != default_picture_size) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Error corrupt default picture.\ndefault_picture_bytes.size() == %1\ndefault_picture_size.size() == %2")
                             .arg(default_picture_bytes.size())
                             .arg(default_picture_size)
        );
        return;
    }

    image_bytes.clear();
    thumbnail_image_bytes.clear();

    QPixmap pixmap;
    pixmap.loadFromData(reinterpret_cast<const uchar*>(default_picture_bytes.c_str()), default_picture_size);
    ui->pictureLabel->setPixmap(pixmap);
    */
}

