//
// Created by jeremiah on 9/10/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_display_user_picture.h" resolved

#include <general_utility.h>

#include <utility>
#include <QMessageBox>
#include <globals.h>
#include <user_login_info.h>

#include "display_user_picture.h"
#include "ui_display_user_picture.h"
#include "aspect_ratio_pixmap_label.h"

DisplayUserPicture::DisplayUserPicture(
        std::string user_oid,
        std::string picture_oid,
        bool run_delete_picture,
        QWidget* parent
        ) :
        QWidget(parent), ui(new Ui::DisplayUserPicture), user_oid(std::move(user_oid)), picture_oid(std::move(picture_oid)), run_delete_picture(run_delete_picture)  {
    ui->setupUi(this);

    ui->deletePicturePushButton->setVisible(user_admin_privileges.update_single_users());
//    ui->deletePicturePushButton->setVisible(!default_picture && user_admin_privileges.update_single_users());
//    ui->defaultPictureLabel->setVisible(default_picture);

    if(run_delete_picture) {
        connect(&remove_user_picture, &RemoveUserPicture::signal_requestSuccessfullyCompleted, this,
                &DisplayUserPicture::slot_setDefaultPictureCompleted);
        connect(&remove_user_picture, &RemoveUserPicture::signal_sendWarning, this,
                &DisplayUserPicture::slot_sendWarning);
        connect(&remove_user_picture, &RemoveUserPicture::signal_requestCanceled, this,
                &DisplayUserPicture::slot_requestCanceled);
    }
}

void DisplayUserPicture::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if(completed_lambda) {
        completed_lambda();
    }
}

void DisplayUserPicture::slot_setDefaultPictureCompleted(const std::function<void()>& completed_lambda [[maybe_unused]]) {
    //NOTE: do NOT run the completed_lambda, it will enable the delete button however the parent
    // of this object should remove it so no reason to give even a slight ability to attempt to
    // re-do delete picture function
    //completed_lambda();

    emit signal_removeThisIndex(this, picture_oid);
}

void DisplayUserPicture::slot_sendWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& completed_lambda
        ) {

    if(completed_lambda) {
        completed_lambda();
    }
    std::cout << "title: " << title.toStdString();
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(
            this,
             title,
             text
    );
}

void DisplayUserPicture::showPicture(
        const std::string& picture_in_bytes,
        const int num_bytes
) {

    QPixmap pixmap;
    pixmap.loadFromData(
            reinterpret_cast<const uchar*>(picture_in_bytes.c_str()),
            num_bytes
    );

    QSize label_size = generatePictureSize(pixmap.height(), pixmap.width());
    auto* label = new QLabel();
    label->setScaledContents(true);
    label->setFixedSize(label_size);

    label->setPixmap(pixmap);

    ui->pictureFrame->layout()->addWidget(label);
}

void DisplayUserPicture::showPicture(
        const QByteArray& picture_in_bytes
) {
    QPixmap pixmap;
    pixmap.loadFromData(picture_in_bytes);

    QSize label_size = generatePictureSize(pixmap.height(), pixmap.width());
    auto* label = new QLabel();
    label->setScaledContents(true);
    label->setFixedSize(label_size);

    label->setPixmap(pixmap);

    ui->pictureFrame->layout()->addWidget(label);
}

void DisplayUserPicture::setButtonToEnabled(bool enabled) {
    ui->deletePicturePushButton->setEnabled(enabled);
}

void DisplayUserPicture::on_deletePicturePushButton_clicked() {
    if(run_delete_picture) {
        ui->deletePicturePushButton->setEnabled(false);
        remove_user_picture.runRemoveUserPicture(
                user_oid,
                picture_oid,
                [this] {
                    ui->deletePicturePushButton->setEnabled(true);
                });
    } else {
        emit signal_removeThisIndex(this, picture_oid);
    }
}

DisplayUserPicture::~DisplayUserPicture() {
    delete ui;
}
