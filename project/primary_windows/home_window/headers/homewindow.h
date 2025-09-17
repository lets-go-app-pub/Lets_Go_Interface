//
// Created by jeremiah on 8/26/21.
//
#pragma once

#include <QWidget>
#include <QUuid>
#include <utility>

#include <request_server_icons.h>
#include <set_server_activity.h>
#include <set_server_category.h>
#include <set_server_icon.h>
#include <request_server_activities_categories.h>
#include "../files/set_default_functions/set_default_categories.h"
#include "../files/set_default_functions/set_default_activities.h"
#include "../files/set_default_functions/set_default_icons.h"

class StatisticsWindow;

class FeedbackWindow;

class SetFieldsWindow;

class FindUserWindow;

class SetDefaultPictureWindow;

class ReportsWindow;

class ShutdownServers;

class ErrorsWindow;

class AddEventWindow;

class CancelEventWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class HomeWindow; }
QT_END_NAMESPACE

class HomeWindow : public QWidget {
Q_OBJECT

public:
    explicit HomeWindow(QWidget* parent = nullptr);

    ~HomeWindow() override;

    void findEvent(const std::string& event_oid);

    void findUser(const std::string& user_oid);

private slots:

    void on_statisticsButton_clicked();

    void on_feedbackButton_clicked();

    void on_setCategoriesActivitiesIconsButton_clicked();

    void on_findUserButton_clicked();

    void on_reportsButton_clicked();

    void on_serverShutdownButton_clicked();

    void on_errorsUserButton_clicked();

    void on_findEventButton_clicked();

    void on_addEventButton_clicked();

    void on_cancelEventButton_clicked();

    void on_setDefaultsButton_clicked();

    //will be set to the icons_response
    void slot_requestCompleted(const std::function<void()>& completed_lambda = []() {});

    //this will be set to the error message if one occurs, it will return the passed error_lambda
    void slot_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = []() {}
    );

    //this will be called if the request was canceled
    void slot_requestCanceled(const std::function<void()>& completed_lambda = []() {});

    void slot_initialIconsFailed();

    //this will be set to the error message if one occurs, it will return the passed error_lambda
    void slot_sendRequestIconsWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda
    );

protected:
    void showEvent(QShowEvent* event) override;

private:
    Ui::HomeWindow* ui;
    StatisticsWindow* statistics_window_handle = nullptr;
    FeedbackWindow* feedback_window_handle = nullptr;
    SetFieldsWindow* set_fields_window_handle = nullptr;
    FindUserWindow* find_user_window_handle = nullptr;
    ReportsWindow* reports_window_handle = nullptr;
    ShutdownServers* shutdown_servers_window_handle = nullptr;
    ErrorsWindow* errors_window_handle = nullptr;

    FindUserWindow* find_event_window_handle = nullptr;
    AddEventWindow* add_event_window_handle = nullptr;
    CancelEventWindow* cancel_event_window_handle = nullptr;

    const std::string error_message_template = "Failed to log in. Please try again later (and show me this error message, maybe a picture?)\n\nError Message: ";

    void setup_buttons_for_admin_privileges();

    void hide_all_buttons();

    bool requesting_icons = false;
    bool requesting_activities_categories = false;

    RequestIconsObject request_icons_object;
    RequestActivitiesCategoriesObject request_activities_categories_object;

    void requestInitialUserInfo();

    std::vector<SetServerCategoriesType> set_server_categories;
    std::vector<SetServerIconsType> set_server_icons;
    std::vector<SetServerActivitiesType> set_server_activities;

    //starts inserting default activities, this function is actually recursive so
    // that order of activities being inserted is guaranteed
    void setDefaultServerActivity();

    //starts inserting default categories, this function is actually recursive so
    // that order of activities being inserted is guaranteed
    //when all categories and icons have been inserted, it will insert any leftover activities
    void setDefaultServerCategory();

    //starts inserting default activities, this function is actually recursive so
    // that order of activities being inserted is guaranteed
    //when all categories and icons have been inserted, it will insert any leftover activities
    void setDefaultServerIcon();

};
