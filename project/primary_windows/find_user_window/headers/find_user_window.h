//
// Created by jeremiah on 9/10/21.
//

#pragma once

#include <QWidget>
#include "display_user_window.h"

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class FindUserWindow; }
QT_END_NAMESPACE

class FindUserWindow : public QWidget {
Q_OBJECT

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    FindUserWindow(bool event_window = false, QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~FindUserWindow() override;

    void searchByOid(const std::string& account_oid);

private slots:
    void on_phoneNumberRadioButton_toggled(bool checked);

    void on_userIDRadioButton_toggled(bool checked);

    void on_searchPushButton_clicked();

    void slot_displayWarning(const QString& title, const QString& text, const std::function<void()>& lambda);

    //will be set to completed if successful
    void slot_requestSuccessfullyCompleted(const std::function<void()>& completed_lambda = [](){});

    //this will be called if the request was canceled
    void slot_requestCanceled(const std::function<void()>& completed_lambda = [](){});

    void slot_findEvent(const std::string& event_oid);
private:
    Ui::FindUserWindow* ui;

    const bool event_window = false;

    HomeWindow* home_window_handle = nullptr;

    DisplayUserWindow* display_user_window = nullptr;

    void initializeDisplayUserWindow();
};

