//
// Created by jeremiah on 8/26/21.
//

#include <QMessageBox>
#include <grpcpp/impl/codegen/client_context.h>
#include <RequestStatistics.grpc.pb.h>
#include <QBuffer>

#include "grpc_channel.h"
#include "user_login_info.h"
#include "setup_login_info.h"

#include "homewindow.h"
#include "ui_homewindow.h"

#include "RequestAdminInfo.grpc.pb.h"
#include "statistics_window.h"
#include "feedback_window.h"
#include "set_fields_window.h"
#include "find_user_window.h"
#include "reports_window.h"
#include "shutdown_servers.h"
#include "errors_window.h"
#include "add_event_window.h"
#include "cancel_event_window.h"

Q_DECLARE_METATYPE(std::function<void()>)

Q_DECLARE_METATYPE(request_statistics::AgeGenderStatisticsResponse)

HomeWindow::HomeWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::HomeWindow) {

    ui->setupUi(this);
    setFixedSize(size());

    qRegisterMetaType<std::function<void()>>();
    qRegisterMetaType<request_statistics::AgeGenderStatisticsResponse>();

    connect(&request_icons_object, &RequestIconsObject::signal_requestSuccessfullyCompleted, this,
            &HomeWindow::slot_requestCompleted);
    connect(&request_icons_object, &RequestIconsObject::signal_sendWarning, this,
            &HomeWindow::slot_sendRequestIconsWarning);
    connect(&request_icons_object, &RequestIconsObject::signal_requestCanceled, this,
            &HomeWindow::slot_requestCanceled);

    connect(&request_activities_categories_object, &RequestIconsObject::signal_requestSuccessfullyCompleted, this,
            &HomeWindow::slot_requestCompleted);
    connect(&request_activities_categories_object, &RequestIconsObject::signal_sendWarning, this,
            &HomeWindow::slot_sendWarning);
    connect(&request_activities_categories_object, &RequestIconsObject::signal_requestCanceled, this,
            &HomeWindow::slot_requestCanceled);

    hide_all_buttons();
    ui->welcomeLabel->setVisible(false);
}

void HomeWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    //initial login
    requestInitialUserInfo();

    //request icons
    ui->setDefaultsButton->setEnabled(false);
    requesting_icons = true;
    request_icons_object.runRequestIcons([&]() {
        requesting_icons = false;

        if (!requesting_activities_categories && !requesting_icons) {
            ui->setDefaultsButton->setEnabled(true);
        }
    });

    setup_buttons_for_admin_privileges();
}

void HomeWindow::requestInitialUserInfo() {

    grpc::ClientContext unary_context;

    request_admin_info::InitialProgramOpenRequestInfoRequest request;
    request_admin_info::InitialProgramOpenRequestInfoResponse response;

    setup_login_info(request.mutable_login_info());

    std::unique_ptr<request_admin_info::RequestAdminInfoService::Stub> request_admin_info_stub = request_admin_info::RequestAdminInfoService::NewStub(
            channel);
    unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

    grpc::Status status = request_admin_info_stub->InitialProgramOpenRequestInfoRPC(&unary_context, request, &response);

    if (!status.ok()) { //if grpc call failed
        const std::string errorMessage = error_message_template + "Grpc returned status.ok() == false; code: " +
                                         std::to_string(status.error_code()) +
                                         " message: " + status.error_message();

        ui->logginInLabel->setText(errorMessage.c_str());
        return;
    } else if (!response.success()) { //if failed to log in
        const std::string errorMessage = error_message_template + response.error_msg();

        ui->logginInLabel->setText(errorMessage.c_str());
        return;
    }

    globals = std::make_unique<const request_admin_info::GlobalValuesToPassMessage>(response.globals());

    user_admin_level = response.admin_level();
    user_admin_privileges = response.admin_privileges();

    std::unique_lock<std::mutex> categories_lock(categories_mutex, std::defer_lock);
    std::unique_lock<std::mutex> activities_lock(activities_mutex, std::defer_lock);

    std::lock(categories_lock, activities_lock);
    categories = response.server_categories();
    activities = response.server_activities();

}

void HomeWindow::setup_buttons_for_admin_privileges() {

    hide_all_buttons();

    ui->welcomeLabel->setVisible(true);
    ui->welcomeLabel->setText(QString("Welcome %1!").arg(QString::fromStdString(USER_NAME)));

    if (
            !user_admin_privileges.view_matching_activity_statistics()
            && !user_admin_privileges.view_age_gender_statistics()
            && !user_admin_privileges.view_activities_and_categories()
            && !user_admin_privileges.update_activities_and_categories()
            && !user_admin_privileges.update_icons()
            && !user_admin_privileges.view_reports()
            && !user_admin_privileges.handle_reports()
            && !user_admin_privileges.find_single_users()
            && !user_admin_privileges.update_single_users()
            && !user_admin_privileges.view_activity_feedback()
            && !user_admin_privileges.view_other_feedback()
            && !user_admin_privileges.view_bugs_feedback()
            && !user_admin_privileges.setup_default_values()
            && !user_admin_privileges.request_chat_messages()
            && !user_admin_privileges.request_error_messages()
            && !user_admin_privileges.handle_error_messages()
            && !user_admin_privileges.run_server_shutdowns()
            && !user_admin_privileges.add_events()
            && !user_admin_privileges.cancel_events()
            ) {
        ui->logginInLabel->setText("No privileges for this user detected.");
        return;
    }

    if (user_admin_privileges.view_matching_activity_statistics()
        || user_admin_privileges.view_age_gender_statistics()
            ) {
        ui->statisticsButton->setVisible(true);
    }

    if (user_admin_privileges.view_activities_and_categories()
        || user_admin_privileges.update_activities_and_categories()
        || user_admin_privileges.update_icons()) {
        ui->setCategoriesActivitiesIconsButton->setVisible(true);
    }

    if (user_admin_privileges.view_reports()
        && user_admin_privileges.handle_reports()) {
        ui->reportsButton->setVisible(true);
    }

    if (user_admin_privileges.find_single_users()
        && user_admin_privileges.update_single_users()) {
        ui->findUserButton->setVisible(true);
        ui->findEventButton->setVisible(true);
    }

    if (user_admin_privileges.view_activity_feedback()
        || user_admin_privileges.view_other_feedback()
        || user_admin_privileges.view_bugs_feedback()
            ) {
        ui->feedbackButton->setVisible(true);
    }

    if (user_admin_privileges.setup_default_values()) {
        //Do not want the set defaults button when on Release version.
#ifdef QT_DEBUG
        ui->setDefaultsButton->setVisible(true);
#else
        ui->setDefaultsButton->setVisible(false);
#endif
    }

    if (user_admin_privileges.request_error_messages()) {
        ui->errorsUserButton->setVisible(true);
    }

    if (user_admin_privileges.run_server_shutdowns()) {
        ui->serverShutdownButton->setVisible(true);
    }

    if (user_admin_privileges.add_events()) {
        ui->addEventButton->setVisible(true);
    }

    if (user_admin_privileges.cancel_events()) {
        ui->cancelEventButton->setVisible(true);
    }

    ui->logginInLabel->setVisible(false);
}

//will be set to the icons_response
void HomeWindow::slot_requestCompleted(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void HomeWindow::slot_initialIconsFailed() {
    //if icon request failed, need to empty categories and activities to avoid seg faults from
    // invalid index values
    std::unique_lock<std::mutex> categories_lock(categories_mutex, std::defer_lock);
    std::unique_lock<std::mutex> activities_lock(activities_mutex, std::defer_lock);

    std::lock(categories_lock, activities_lock);
    categories.Clear();
    activities.Clear();
}

//this will be set to the error message if one occurs, it will return the passed error_lambda
void HomeWindow::slot_sendRequestIconsWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& completed_lambda
) {
    slot_initialIconsFailed();
    slot_sendWarning(title, text, completed_lambda);
}

//this will be set to the error message if one occurs, it will return the passed error_lambda
void HomeWindow::slot_sendWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& completed_lambda
) {

    if (completed_lambda) {
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

//this will be called if the request was canceled
void HomeWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
    slot_initialIconsFailed();
}

HomeWindow::~HomeWindow() {
    delete ui;
}

void HomeWindow::on_statisticsButton_clicked() {

    if (statistics_window_handle == nullptr) {
        statistics_window_handle = new StatisticsWindow();
        statistics_window_handle->setHomeWindowHandle(this);
    }

    statistics_window_handle->show();
}

void HomeWindow::on_feedbackButton_clicked() {
    if (feedback_window_handle == nullptr) {
        feedback_window_handle = new FeedbackWindow();
        feedback_window_handle->setHomeWindowHandle(this);
    }

    feedback_window_handle->show();
}

void HomeWindow::on_setCategoriesActivitiesIconsButton_clicked() {
    if (set_fields_window_handle == nullptr) {
        set_fields_window_handle = new SetFieldsWindow();
        set_fields_window_handle->setHomeWindowHandle(this);
    }

    set_fields_window_handle->show();
}

void HomeWindow::on_findUserButton_clicked() {
    if (find_user_window_handle == nullptr) {
        find_user_window_handle = new FindUserWindow();
        find_user_window_handle->setHomeWindowHandle(this);
    }

    find_user_window_handle->show();
}

void HomeWindow::on_reportsButton_clicked() {
    if (reports_window_handle == nullptr) {
        reports_window_handle = new ReportsWindow();
        reports_window_handle->setHomeWindowHandle(this);
    }

    reports_window_handle->show();
}

void HomeWindow::on_serverShutdownButton_clicked() {
    if (shutdown_servers_window_handle == nullptr) {
        shutdown_servers_window_handle = new ShutdownServers();
        shutdown_servers_window_handle->setHomeWindowHandle(this);
    }

    shutdown_servers_window_handle->show();
}

void HomeWindow::on_errorsUserButton_clicked() {
    if (errors_window_handle == nullptr) {
        errors_window_handle = new ErrorsWindow();
        errors_window_handle->setHomeWindowHandle(this);
    }

    errors_window_handle->show();
}

void HomeWindow::on_findEventButton_clicked() {
    if (find_event_window_handle == nullptr) {
        find_event_window_handle = new FindUserWindow(true);
        find_event_window_handle->setHomeWindowHandle(this);
    }

    find_event_window_handle->show();
}

void HomeWindow::on_addEventButton_clicked() {
    if (add_event_window_handle == nullptr) {
        add_event_window_handle = new AddEventWindow();
        add_event_window_handle->setHomeWindowHandle(this);
    }

    add_event_window_handle->show();
}

void HomeWindow::on_cancelEventButton_clicked() {
    if (cancel_event_window_handle == nullptr) {
        cancel_event_window_handle = new CancelEventWindow();
        cancel_event_window_handle->setHomeWindowHandle(this);
    }

    cancel_event_window_handle->show();
}

void HomeWindow::hide_all_buttons() {
    ui->statisticsButton->setVisible(false);
    ui->reportsButton->setVisible(false);
    ui->findUserButton->setVisible(false);
    ui->feedbackButton->setVisible(false);
    ui->setCategoriesActivitiesIconsButton->setVisible(false);
    ui->setDefaultsButton->setVisible(false);
    ui->errorsUserButton->setVisible(false);
    ui->serverShutdownButton->setVisible(false);
    ui->findEventButton->setVisible(false);
    ui->addEventButton->setVisible(false);
    ui->cancelEventButton->setVisible(false);
}

void HomeWindow::setDefaultServerActivity() {

    if (set_server_activities.empty()) {
        return;
    }

    auto set_server_activity = set_server_activities.back().activities_ptr;

    connect(set_server_activity.get(), &SetServerActivity::signal_requestSuccessfullyCompleted, this,
            &HomeWindow::slot_requestCompleted);
    connect(set_server_activity.get(), &SetServerActivity::signal_sendWarning, this,
            &HomeWindow::slot_sendRequestIconsWarning);
    connect(set_server_activity.get(), &SetServerActivity::signal_requestCanceled, this,
            &HomeWindow::slot_requestCanceled);

    set_server_activity->runSetServerActivity(
            false,
            set_server_activities.back().parameters->display_name,
            set_server_activities.back().parameters->icon_display_name,
            set_server_activities.back().parameters->min_age,
            set_server_activities.back().parameters->category_index,
            set_server_activities.back().parameters->icon_index,
            [this]() {
                set_server_activities.pop_back();

                if (set_server_activities.empty()) {

                    //refresh after all defaults have been set

                    requesting_icons = true;
                    request_icons_object.runRequestIcons([&]() {
                        requesting_icons = false;

                        if (!requesting_activities_categories && !requesting_icons) {
                            ui->setDefaultsButton->setEnabled(true);
                        }
                    });

                    requesting_activities_categories = true;
                    request_activities_categories_object.runRequestActivitiesCategories([&]() {
                        requesting_activities_categories = false;

                        if (!requesting_activities_categories && !requesting_icons) {
                            ui->setDefaultsButton->setEnabled(true);
                        }
                    });

                } else {
                    setDefaultServerActivity();
                }
            });
}

void HomeWindow::setDefaultServerCategory() {

    if (set_server_categories.empty()) {
        return;
    }

    auto set_server_category = set_server_categories.back().category_ptr;

    connect(set_server_category.get(), &SetServerCategory::signal_requestSuccessfullyCompleted, this,
            &HomeWindow::slot_requestCompleted);
    connect(set_server_category.get(), &SetServerCategory::signal_sendWarning, this,
            &HomeWindow::slot_sendRequestIconsWarning);
    connect(set_server_category.get(), &SetServerCategory::signal_requestCanceled, this,
            &HomeWindow::slot_requestCanceled);

    set_server_category->runSetServerCategory(
            false,
            set_server_categories.back().parameters->category_name,
            set_server_categories.back().parameters->icon_display_name,
            set_server_categories.back().parameters->order_number,
            set_server_categories.back().parameters->min_age,
            set_server_categories.back().parameters->color,
            [this]() {
                set_server_categories.pop_back();

                if (set_server_icons.empty() && set_server_categories.empty()) {
                    setDefaultServerActivity();
                } else {
                    setDefaultServerCategory();
                }
            });
}

void HomeWindow::setDefaultServerIcon() {

    if (set_server_icons.empty()) {
        return;
    }

    QImage basic_image(set_server_icons.back().parameters->basic_resource_path);

    QByteArray basic_bytes;

    QBuffer basic_buffer(&basic_bytes);
    basic_buffer.open(QIODevice::WriteOnly);

    //NOTE: do NOT set a quality rating for .save(), leave it at -1. It will increase the
    // size of the file needlessly.
    basic_image.save(&basic_buffer, "PNG");

    auto set_server_icon = set_server_icons.back().icons_ptr;

    connect(set_server_icon.get(), &SetServerIcon::signal_requestSuccessfullyCompleted, this,
            &HomeWindow::slot_requestCompleted);
    connect(set_server_icon.get(), &SetServerIcon::signal_sendWarning, this,
            &HomeWindow::slot_sendRequestIconsWarning);
    connect(set_server_icon.get(), &SetServerIcon::signal_requestCanceled, this,
            &HomeWindow::slot_requestCanceled);

    set_server_icon->runSetServerIcon(
            true,
            -1,
            true,
            basic_bytes,
            [this]() {
                set_server_icons.pop_back();

                if (set_server_icons.empty() && set_server_categories.empty()) {
                    setDefaultServerActivity();
                } else {
                    setDefaultServerIcon();
                }
            });
}

void HomeWindow::on_setDefaultsButton_clicked() {

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Warning",
                                  "This will set the server defaults. It expects all defaults to be cleared from the database because it will 'push' some values on. Continue anyways?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    //build default value vectors
    set_default_categories(set_server_categories);
    set_default_icons(set_server_icons);
    set_default_activities(set_server_activities);

    ui->setDefaultsButton->setEnabled(false);

    //NOTE: These vectors must be reversed. This is because
    // pop_back is used to extract them so the vector must be treated like a stack.
    std::reverse(set_server_categories.begin(), set_server_categories.end());
    std::reverse(set_server_icons.begin(), set_server_icons.end());
    std::reverse(set_server_activities.begin(), set_server_activities.end());

    setDefaultServerCategory();
    setDefaultServerIcon();
}

void HomeWindow::findEvent(const std::string& event_oid) {
    on_findEventButton_clicked();

    find_event_window_handle->searchByOid(event_oid);
}

void HomeWindow::findUser(const std::string& user_oid) {
    on_findUserButton_clicked();

    find_user_window_handle->searchByOid(user_oid);
}

