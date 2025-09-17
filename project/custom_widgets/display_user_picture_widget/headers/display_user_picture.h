//
// Created by jeremiah on 9/10/21.
//

#pragma once

#include <QWidget>
#include <remove_user_picture.h>
#include "RequestUserAccountInfo.grpc.pb.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DisplayUserPicture; }
QT_END_NAMESPACE

class DisplayUserPicture : public QWidget {
Q_OBJECT

public:
    explicit DisplayUserPicture(
            std::string user_oid,
            std::string picture_oid,
            bool run_delete_picture,
            QWidget* parent = nullptr);

    ~DisplayUserPicture() override;

    void showPicture(
            const std::string& picture_in_bytes,
            int num_bytes);

    void showPicture(
            const QByteArray& picture_in_bytes
    );

    void setButtonToEnabled(bool enabled);

signals:

    void signal_removeThisIndex(QWidget* w, const std::string& picture_oid);

private slots:

    void on_deletePicturePushButton_clicked();

    //will be set to the icons_response
    void slot_setDefaultPictureCompleted(const std::function<void()>& completed_lambda = [](){});

    //this will be set to the error message if one occurs, it will return the passed error_lambda
    void slot_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = [](){}
    );

    //this will be called if the request was canceled
    void slot_requestCanceled(const std::function<void()>& completed_lambda = [](){});

private:
    Ui::DisplayUserPicture* ui;

    RemoveUserPicture remove_user_picture;
    const std::string user_oid;
    const std::string picture_oid;
    //Set this to nullptr if RemoveUserPicture is being used.
    const bool run_delete_picture;

};


