//
// Created by jeremiah on 9/21/21.
//

#pragma once

#include <QWidget>

#include <QScrollBar>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <thread>
#include <shared_mutex>
#include <utility>

#include <HandleReports.grpc.pb.h>
#include <RequestUserAccountInfo.pb.h>
#include <AdminChatRoomCommands.pb.h>
#include <display_user_window.h>
#include <button_with_meta_data.h>

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class ReportsWindow; }
QT_END_NAMESPACE

class ReportsWindow : public QWidget {
Q_OBJECT

protected:
    void closeEvent(QCloseEvent* event) override;

    void showEvent(QShowEvent* event) override;

public:
    explicit ReportsWindow(QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~ReportsWindow() override;

signals:

    //will be set to completed if successful
    void signal_handleRequestReportCompleted(
            const std::function<void()>& completed_lambda = [](){}
    );

    //this will be set to the error message if one occurs
    void signal_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = [](){}
    );

    //this will be called if the request was canceled
    void signal_requestCanceled(const std::function<void()>& completed_lambda = [](){});

private slots:

    void slot_handleRequestReportCompleted(
            const std::function<void()>& completed_lambda
    );

    void slot_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda
    );

    void slot_requestCanceled(const std::function<void()>& completed_lambda = [](){});

    void slot_setUserAsSpammerClicked(const std::string& user_oid, ButtonWithMetaData* button_handle);

    void slot_handleReportCompleted(const std::string& reported_user_oid);

    void on_refreshReportsPushButton_clicked();

private:
    Ui::ReportsWindow* ui;
    HomeWindow* home_window_handle = nullptr;

    //this needs to be the total number of columns info is contained inside of plus 1 for layout
    // purposes
    const int TOTAL_NUMBER_COLUMNS = 4;

    DisplayUserWindow* display_user_window = nullptr;

    std::map<std::thread::id, std::unique_ptr<std::jthread>> request_report_threads;

    struct ReportInformation {

        bool no_more_reports = false;

        ReportInformation() = default;

        explicit ReportInformation(handle_reports::ReportedUserInfo* _user_reports) {
            user_reports.Swap(_user_reports);
        }

        std::chrono::milliseconds checkout_expiration_time{-1};

        handle_reports::ReportedUserInfo user_reports{};

        //NOTE: this is MOVED into the DisplayUserWindow object making it invalid afterwards
        CompleteUserAccountInfo user_info;

        //key is the message uuid, value is the message
        std::map<std::string, ChatMessageToClient> messages;
    };

    void displayNextReportedUser();

    //reports is accessed from multiple threads so reports_mutex is used when setting
    // any values
    std::shared_mutex reports_mutex;
    std::vector<std::unique_ptr<ReportInformation>> reports;

    int number_reports_currently_requesting = 0;

    void requestReportsCompleted(std::thread::id thread_id, int number_reports_requesting);

    //returns true if successfully started request reports thread
    //returns false if failed to start request reports thread
    bool requestReports();

    void initializeDisplayUserWindow();

    void clearCurrentReport();

    void refreshReports();

    struct OrganizeReportsStruct {

        OrganizeReportsStruct(
                std::string _user_oid,
                int index_value,
                std::chrono::milliseconds _earliest_timestamp_sent) :
                user_oid(std::move(_user_oid)),
                earliest_timestamp_sent(_earliest_timestamp_sent) {
            index_values.push_back(index_value);
        }

        std::string user_oid{};

        std::vector<int> index_values{};

        std::chrono::milliseconds earliest_timestamp_sent{-1};
    };

};

