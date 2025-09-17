//
// Created by jeremiah on 3/23/23.
//

#pragma once

#include <QWidget>
#include <QLabel>
#include <utility>
#include "add_admin_event.h"
#include "display_user_picture.h"

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class AddEventWindow; }
QT_END_NAMESPACE

class AddEventWindow : public QWidget {
Q_OBJECT

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    explicit AddEventWindow(QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~AddEventWindow() override;

private slots:

    void on_submitPushButton_clicked();

    void on_addPicturesPushButton_clicked();

    void on_addQrCodePushButton_clicked();

    void slot_removePicture(QWidget *w, const std::string& picture_oid);

    void slot_removeQrCode(QWidget *w, const std::string& picture_oid);

    void slot_requestCanceled(const std::function<void()>& completed_lambda);

    void slot_displayWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& lambda
    );

    void slot_addAdminEventSuccessfullyCompleted(
            const std::string& event_oid,
            const std::function<void()>& lambda
    );

private:
    Ui::AddEventWindow* ui;

    HomeWindow* home_window_handle = nullptr;

    void setupDefaultValuesAndRestrictions();

    void setupSignalsAndSlots();

    void setEnabledForMutableFields(bool enabled);

    inline static const QString BIO_PLAIN_TEXT_EDIT_NAME = "bio_plain_text_edit_name";
    inline static const QString BIO_PLAIN_TEXT_EDIT_PLACEHOLDER = "This will be shown as the bio in the card.";

    AddAdminEventObject add_admin_event_object;

    struct Picture {
        std::string picture_id;
        QByteArray picture_bytes;
        QByteArray thumbnail_bytes;
        DisplayUserPicture* picture_frame = nullptr;

        Picture() = delete;

        explicit Picture(
                std::string picture_id
                ): picture_id(std::move(picture_id)) {}
    };

    std::vector<Picture> saved_pictures;

    QByteArray saved_qr_code;
    DisplayUserPicture* qr_code_picture_frame = nullptr;
};
