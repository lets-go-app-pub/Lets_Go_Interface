//
// Created by jeremiah on 9/18/21.
//

#pragma once
#include <QWidget>
#include <set_default_picture.h>

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class SetDefaultPictureWindow; }
QT_END_NAMESPACE

class SetDefaultPictureWindow : public QWidget {
Q_OBJECT

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    explicit SetDefaultPictureWindow(QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~SetDefaultPictureWindow() override;

private slots:
    void on_defaultPushButton_clicked();

private slots:
    void on_sendPushButton_clicked();

    void on_selectPicturePushButton_clicked();

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
    HomeWindow* home_window_handle = nullptr;
    Ui::SetDefaultPictureWindow* ui;

    QByteArray image_bytes;
    QByteArray thumbnail_image_bytes;

    SetDefaultPicture set_default_picture;
};

