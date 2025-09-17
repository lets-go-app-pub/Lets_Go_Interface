//
// Created by jeremiah on 9/21/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_reports_window.h" resolved

#include <general_utility.h>
#include <setup_login_info.h>
#include <grpc_channel.h>
#include <user_login_info.h>
#include <QMessageBox>
#include "reports_window.h"
#include "ui_reports_window.h"
#include "homewindow.h"

#include <unordered_map>
#include <display_chat_message.h>
#include <button_with_meta_data.h>

ReportsWindow::ReportsWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::ReportsWindow) {
    ui->setupUi(this);

    //TESTING_NOTE: make sure to check multiple reporting_users as the same user and combine them ideally
    //TESTING_NOTE: test getting a 'no reports' cap then refresh removing it

    connect(this, &ReportsWindow::signal_handleRequestReportCompleted, this,
            &ReportsWindow::slot_handleRequestReportCompleted);
    connect(this, &ReportsWindow::signal_sendWarning, this,
            &ReportsWindow::slot_sendWarning);
    connect(this, &ReportsWindow::signal_requestCanceled, this,
            &ReportsWindow::slot_requestCanceled);

    ui->reportsTreeWidget->setColumnCount(TOTAL_NUMBER_COLUMNS + 1);
    ui->reportsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->reportsTreeWidget->setVerticalScrollMode(QTreeView::ScrollPerPixel);

    initializeDisplayUserWindow();
}

void ReportsWindow::slot_handleRequestReportCompleted(
        const std::function<void()>& completed_lambda
) {

    //this needs to run first to remove the no reports remaining 'cap' if necessary
    if (completed_lambda) {
        completed_lambda();
    }

    displayNextReportedUser();
}

void ReportsWindow::slot_setUserAsSpammerClicked(const std::string& user_oid, ButtonWithMetaData* button_handle) {

    if (button_handle != nullptr) {
        button_handle->setEnabled(false);
    }

    //NOTE: this method of handling the threads will allow each reporting user to be set as spam ONCE per reported user

    std::jthread spam_thread([user_oid, this, button_handle] {
        //NOTE: this currently runs on the MAIN thread so this function stops the UI from functioning
        grpc::ClientContext unary_context;

        handle_reports::ReportUnaryCallRequest request;
        handle_reports::ReportResponseUnaryCallResponse response;
        setup_login_info(request.mutable_login_info());
        request.set_user_oid(user_oid);

        std::unique_ptr<handle_reports::HandleReportsService::Stub> handle_reports_stub =
                handle_reports::HandleReportsService::NewStub(channel);

        unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        grpc::Status status = handle_reports_stub->SetReportToSpamRPC(&unary_context,
                                                                      request,
                                                                      &response);

        if (!status.ok()) { //if grpc call failed
            const std::string errorMessage =
                    "Grpc returned status.ok() == false; code: " +
                    std::to_string(status.error_code()) +
                    " message: " + status.error_message();

            emit signal_sendWarning(
                    "Error", errorMessage.c_str(),
                    [button_handle]() {
                        //TESTING_NOTE: make sure that if this is reached it doesn't seg fault
                        if (button_handle != nullptr) {
                            button_handle->setEnabled(true);
                        }
                    });
            return;
        } else if (!response.successful()) { //if failed to log in

            emit signal_sendWarning(
                    "Error", response.error_message().c_str(),
                    [user_oid, button_handle]() {
                        if (button_handle != nullptr) {
                            button_handle->setEnabled(true);
                        }
                    });
            return;
        }

        //do not re-enable spam_button here, the reporting user should only be updated as a spammer once per reported user

    });

    spam_thread.detach();
}

void ReportsWindow::clearCurrentReport() {
    ui->reportsTreeWidget->clear();
    display_user_window->clearWidget(true);

    ui->reportsTitleLabel->setText("Reports empty.");
}

void ReportsWindow::displayNextReportedUser() {

    //Tree levels
    //1) oid of user (can be a lot or almost none)
    //  2) report info (can be multiple)
    //    3) if message is part of info show it here

    std::shared_lock<std::shared_mutex> lock(reports_mutex);

    if (reports.empty()) {
        // This could happen if the internet connection is down and so no 'no reports' end cap is found
        clearCurrentReport();
    } else if (reports[0]->no_more_reports) { //no more reports to display
        ui->reportsTreeWidget->clear();
        display_user_window->clearWidget(true);

        ui->reportsTitleLabel->setText("End of reports.");
    }
    else { //properly display report

        if (display_user_window->getCurrentDisplayedUserOID() == reports[0]->user_reports.user_account_oid()
                ) { //user is already displayed

            //This could happen if
            // 1) user clicks dismiss or time out
            // 2) the return value runs displayNextReportedUser() followed by requestReports()
            // 3) at the end of requestReports() it will run displayNextReportedUser() again

            //Also, it is worth mentioning that users are stored by oid inside the server, so until a report
            // is sent back as 'handled' the new user can never be stored again. There might be an edge case here
            // where (the stars would need to align).
            // 1) admin requests user from server then goes AFK
            // 2) another admin requests same user from server and handles it
            // 3) this admin comes back from AFK and receives the same user as the next report
            return;
        }

        ui->reportsTreeWidget->clear();
        display_user_window->clearWidget(true);

        QScrollBar* vScrollBar = ui->displayUserScrollArea->verticalScrollBar();
        vScrollBar->triggerAction(QScrollBar::SliderToMinimum);

        ui->reportsTitleLabel->setText("Reports list.");

        std::vector<OrganizeReportsStruct> sorted_reports;

        { //scope for de-allocating map

            std::unordered_map<std::string, OrganizeReportsStruct> mapped_values;

            for (int i = 0; i < reports[0]->user_reports.reports_log_size(); i++) {

                const std::string& oid = reports[0]->user_reports.reports_log().at(i).account_oid_of_report_sender();
                auto found_value = mapped_values.find(oid);
                if (found_value == mapped_values.end()) { //oid does NOT exist yet
                    mapped_values.insert(std::make_pair(
                            oid,
                            OrganizeReportsStruct(
                                    oid,
                                    i,
                                    std::chrono::milliseconds{
                                            reports[0]->user_reports.reports_log().at(i).timestamp_submitted()
                                    }
                            )
                    ));
                } else {
                    found_value->second.index_values.push_back(i);

                    //if current timestamp is smaller, use it instead
                    if (reports[0]->user_reports.reports_log().at(i).timestamp_submitted()
                        < found_value->second.earliest_timestamp_sent.count()) {
                        found_value->second.earliest_timestamp_sent = std::chrono::milliseconds{
                                reports[0]->user_reports.reports_log().at(i).timestamp_submitted()
                        };
                    }
                }
            }

            for (const auto&[key, value] : mapped_values) {
                sorted_reports.emplace_back(value);
            }

            std::sort(
                    sorted_reports.begin(),
                    sorted_reports.end(),
                    [](const OrganizeReportsStruct& lhs, const OrganizeReportsStruct& rhs) -> bool {
                        return lhs.earliest_timestamp_sent < rhs.earliest_timestamp_sent;
                    });
        }

        for (const auto& sorted_report : sorted_reports) {
            auto* user_oid_report_item = new QTreeWidgetItem(ui->reportsTreeWidget);

            user_oid_report_item->setText(0, "User Id: ");

            //user_oid_report_item->setText(1, QString::fromStdString(sorted_report.user_oid));

            auto button_parent_widget = new QWidget();
            auto button_hLayout = new QHBoxLayout();

            auto user_oid_label = new QLabel(QString::fromStdString(sorted_report.user_oid));
            user_oid_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

            button_hLayout->addWidget(user_oid_label);
            auto button = new ButtonWithMetaData(sorted_report.user_oid);
            button->setText(" Spam ");
            button_hLayout->addWidget(button);
            button_hLayout->addSpacerItem(
                    new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
            button_parent_widget->setLayout(button_hLayout);

            connect(button, &ButtonWithMetaData::signal_clickedWithMetaData, this,
                    &ReportsWindow::slot_setUserAsSpammerClicked);

            ui->reportsTreeWidget->setItemWidget(user_oid_report_item, 1, button_parent_widget);

            for (const auto& single_report_index : sorted_report.index_values) {

                auto* report_reason_item = new QTreeWidgetItem(user_oid_report_item);

                report_reason_item->setText(0, "Report Reason: ");
                switch (reports[0]->user_reports.reports_log().at(single_report_index).report_reason()) {
                    case REPORT_REASON_LANGUAGE:
                        report_reason_item->setText(1, "Inappropriate Language");
                        break;
                    case REPORT_REASON_INAPPROPRIATE_PICTURE:
                        report_reason_item->setText(1, "Inappropriate Picture");
                        break;
                    case REPORT_REASON_ADVERTISING:
                        report_reason_item->setText(1, "Advertising");
                        break;
                    case REPORT_REASON_OTHER:
                        report_reason_item->setText(1, "Other reason (text below).");
                        break;
                    case REPORT_REASON_UNKNOWN_DEFAULT:
                    case ReportReason_INT_MIN_SENTINEL_DO_NOT_USE_:
                    case ReportReason_INT_MAX_SENTINEL_DO_NOT_USE_:
                        report_reason_item->setText(1, QString("ERROR: %1")
                                .arg(
                                        QString::fromStdString(
                                                ReportReason_Name(reports[0]->user_reports.reports_log().at(
                                                        single_report_index).report_reason())
                                        )
                                )
                        );
                        break;
                }

                const std::string& report_message = reports[0]->user_reports.reports_log().at(
                        single_report_index).message();

                if (!report_message.empty() && report_message != "~") {
                    auto* report_message_item = new QTreeWidgetItem(user_oid_report_item);

                    report_message_item->setText(0, "Other Message: ");
                    report_message_item->setText(1, QString::fromStdString(report_message));
                }

                auto* report_origin_item = new QTreeWidgetItem(user_oid_report_item);

                report_origin_item->setText(0, "Report Origin: ");

                switch (reports[0]->user_reports.reports_log().at(single_report_index).origin_type()) {
                    case REPORT_ORIGIN_SWIPING:
                        report_origin_item->setText(1, "Swiping screen.");
                        break;
                    case REPORT_ORIGIN_CHAT_ROOM_INFO:
                        report_origin_item->setText(1, "Chat room info screen.");
                        break;
                    case REPORT_ORIGIN_CHAT_ROOM_MEMBER:
                        report_origin_item->setText(1, "Chat room member info screen.");
                        break;
                    case REPORT_ORIGIN_CHAT_ROOM_MESSAGE:
                        report_origin_item->setText(1, "Chat room member message.");
                        break;
                    case REPORT_ORIGIN_CHAT_ROOM_MATCH_MADE:
                        report_origin_item->setText(1, "'Match Made' chat room.");
                        break;
                    case ReportOriginType_INT_MIN_SENTINEL_DO_NOT_USE_:
                    case ReportOriginType_INT_MAX_SENTINEL_DO_NOT_USE_:
                        report_reason_item->setText(1, QString("ERROR: %1")
                                .arg(
                                        QString::fromStdString(
                                                ReportOriginType_Name(reports[0]->user_reports.reports_log().at(
                                                        single_report_index).origin_type())
                                        )
                                ));
                        break;
                }

                if (reports[0]->user_reports.reports_log().at(single_report_index).origin_type() ==
                    ReportOriginType::REPORT_ORIGIN_CHAT_ROOM_MESSAGE
                    && reports[0]->user_reports.reports_log().at(single_report_index).message_uuid().size() == 36
                        ) { //if valid message n stuff
                    auto message = reports[0]->messages.find(
                            reports[0]->user_reports.reports_log().at(single_report_index).message_uuid());
                    if (message != reports[0]->messages.end()) {
                        auto* message_parent_item = new QTreeWidgetItem(user_oid_report_item);
                        auto* message_child_item = new QTreeWidgetItem(message_parent_item);

                        message_parent_item->setText(0, "Message");

                        auto* display_chat_message_item = new DisplayChatMessage(
                                message->second,
                                true,
                                false
                                );

                        ui->reportsTreeWidget->setItemWidget(
                                message_child_item,
                                1,
                                display_chat_message_item
                        );

                    }
                }

                auto* timestamp_submitted_item = new QTreeWidgetItem(user_oid_report_item);

                timestamp_submitted_item->setText(0, "Time submitted: ");
                timestamp_submitted_item->setText(1, QString::fromStdString(
                        getDateTimeStringFromTimestamp(std::chrono::milliseconds{
                                reports[0]->user_reports.reports_log().at(single_report_index).timestamp_submitted()
                        })
                ));

                //insert blank row as a separator
                new QTreeWidgetItem(user_oid_report_item);
            }
            user_oid_report_item->setExpanded(true);
        }

        //NOTE: this invalidates the pointer
        display_user_window->setupGUI(&reports[0]->user_info);

        //set a small buffer so there is whitespace at end
        int width = 15;
        for (int i = 0; i < TOTAL_NUMBER_COLUMNS - 1; i++) {
            width += ui->reportsTreeWidget->columnWidth(i);
        }

        ui->reportsTreeWidget->setMinimumWidth(width);

    }

    ui->reportsTreeWidget->viewport()->update();
}

void ReportsWindow::initializeDisplayUserWindow() {
    if (display_user_window == nullptr) {
        display_user_window = new DisplayUserWindow(true);

        connect(display_user_window, &DisplayUserWindow::signal_handleReportCompleted, this,
                &ReportsWindow::slot_handleReportCompleted);

        ui->scrollAreaWidgetContents->layout()->addWidget(display_user_window);
    }
}

void ReportsWindow::slot_handleReportCompleted(const std::string& reported_user_oid) {

    std::unique_lock<std::shared_mutex> lock(reports_mutex);
    if (!reports.empty() && reports[0]->user_reports.user_account_oid() == reported_user_oid) {
        reports.erase(reports.begin());
        lock.unlock();

        displayNextReportedUser();

        //displayNextReportedUser() will make sure the current user is NOT displayed before
        // doing anything, so even though requestReports() calls displayNextReportedUser() at
        // the end, the GUI will not be rebuilt twice for each time
        requestReports();
    } else {
        lock.unlock();
    }
}

void ReportsWindow::slot_sendWarning(
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

void ReportsWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void ReportsWindow::requestReportsCompleted(std::thread::id thread_id, const int number_reports_requesting) {
    number_reports_currently_requesting -= number_reports_requesting;
    request_report_threads.erase(thread_id);
    if (request_report_threads.empty()) { //no longer requesting any reports
        ui->refreshReportsPushButton->setEnabled(true);
    }
}

bool ReportsWindow::requestReports() {

    std::shared_lock<std::shared_mutex> shared_lock(reports_mutex);

    int num_reports = (int) reports.size() + number_reports_currently_requesting;

    if (!reports.empty() && reports[reports.size() - 1]->no_more_reports) {
        //need to account for the noMoreReports end 'cap'
        num_reports--;
    }

    if (num_reports >= globals->max_number_of_reported_users_admin_can_request_notified()) {
        //possible if the max number of reports is being requested
        return false;
    }

    const int num_to_request =
            globals->max_number_of_reported_users_admin_can_request_notified() - (int) reports.size();
    number_reports_currently_requesting += num_to_request;

    ui->refreshReportsPushButton->setEnabled(false);
    std::unique_ptr<std::jthread> request_thread = std::make_unique<std::jthread>([num_to_request, this](
            const std::stop_token& stop_token
    ) {

        std::thread::id thread_id = std::this_thread::get_id();

        grpc::ClientContext context;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        handle_reports::RequestReportsRequest request;

        setup_login_info(request.mutable_login_info());
        request.set_number_user_to_request(num_to_request);

        std::unique_ptr<handle_reports::HandleReportsService::Stub> request_fields_stub =
                handle_reports::HandleReportsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        //NOTE: this is a client stream to make it possible to programmatically send to server
        // however for GUI purposes it is treated as a unary call
        auto reader = request_fields_stub->RequestReportsRPC(&context, request);

        handle_reports::RequestReportsResponse response;
        std::unique_ptr<ReportInformation> info;
        bool no_more_reports_received = false;

        while (reader->Read(&response)) {

            if (stop_token.stop_requested()) {
                emit signal_requestCanceled(
                        [this, thread_id, num_to_request] {
                            requestReportsCompleted(thread_id, num_to_request);
                        });
                return;
            } else if (!response.successful()) { //if failed to log in
                std::string errorMessage = "Request reports failed.\n" + response.error_message();

                emit signal_sendWarning(
                        "Error",
                        errorMessage.c_str(),
                        [this, thread_id, num_to_request] {
                            requestReportsCompleted(thread_id, num_to_request);
                        });
                return;
            } else {
                switch (response.mutable_returned_info()->returned_info_case()) {
                    case handle_reports::ReturnedInfo::kReportedUserInfo: {

                        info = std::make_unique<ReportInformation>(
                                response.mutable_returned_info()->mutable_reported_user_info()
                        );

                        const std::chrono::milliseconds current_timestamp = getCurrentTimestamp();

                        //subtracting 1 second to account for latency
                        info->checkout_expiration_time =
                                (current_timestamp +
                                 std::chrono::milliseconds{globals->time_before_report_checkout_expires() - 1000});

                        break;
                    }
                    case handle_reports::ReturnedInfo::kUserAccountInfo: {

                        if (info == nullptr) {
                            std::string errorMessage = "Report info was found to be nullptr inside kUserAccountInfo.";

                            emit signal_sendWarning(
                                    "Error",
                                    errorMessage.c_str(),
                                    [this, thread_id, num_to_request] {
                                        requestReportsCompleted(thread_id, num_to_request);
                                    });
                            return;
                        }

                        info->user_info.Swap(response.mutable_returned_info()->mutable_user_account_info());
                        break;
                    }
                    case handle_reports::ReturnedInfo::kChatMessage: {

                        if (info == nullptr) {
                            std::string errorMessage = "Report info was found to be nullptr inside kChatMessage.";

                            emit signal_sendWarning(
                                    "Error",
                                    errorMessage.c_str(),
                                    [this, thread_id, num_to_request] {
                                        requestReportsCompleted(thread_id, num_to_request);
                                    });
                            return;
                        }

                        ChatMessageToClient msg;
                        msg.Swap(response.mutable_returned_info()->mutable_chat_message());
                        info->messages.insert(
                                std::make_pair(
                                        msg.message_uuid(),
                                        std::move(msg)
                                )
                        );
                        break;
                    }
                    case handle_reports::ReturnedInfo::kCompleted: {

                        if (info == nullptr) {

                            std::string errorMessage = "Report info was found to be nullptr inside kCompleted.";

                            emit signal_sendWarning(
                                    "Error",
                                    errorMessage.c_str(),
                                    [this, thread_id, num_to_request] {
                                        requestReportsCompleted(thread_id, num_to_request);
                                    });
                            return;
                        }

                        std::scoped_lock<std::shared_mutex> lock(reports_mutex);

                        //last element can be a 'kNoReports' element, if it is this needs to be
                        // pushed into the 2nd to last element slot
                        if (!reports.empty() && reports.back()->no_more_reports) {
                            reports.insert(reports.end() - 1, std::move(info));
                        } else {
                            reports.emplace_back(std::move(info));
                        }
                        info = nullptr;
                        break;
                    }
                    case handle_reports::ReturnedInfo::kNoReports: {

                        no_more_reports_received = true;

                        if (info == nullptr) {
                            //this is possible if noReports is the only thing returned
                            info = std::make_unique<ReportInformation>();
                        }

                        bool no_reports_exists = false;

                        std::shared_lock<std::shared_mutex> r_w_lock(reports_mutex);

                        //if the final element is not a 'no_more_reports' element
                        if (!reports.empty() && reports.back()->no_more_reports) {
                            no_reports_exists = true;
                        }

                        r_w_lock.unlock();

                        if (!no_reports_exists) {
                            std::unique_ptr<ReportInformation> no_reports_info = std::make_unique<ReportInformation>();
                            no_reports_info->no_more_reports = true;
                            std::scoped_lock<std::shared_mutex> lock(reports_mutex);
                            reports.emplace_back(std::move(no_reports_info));
                        }

                        break;
                    }
                    case handle_reports::ReturnedInfo::RETURNED_INFO_NOT_SET: {
                        std::string errorMessage = "Request reports failed, 'ReturnedInfo::RETURNED_INFO_NOT_SET' returned.";

                        emit signal_sendWarning(
                                "Error",
                                errorMessage.c_str(),
                                [this, thread_id, num_to_request] {
                                    requestReportsCompleted(thread_id, num_to_request);
                                });
                        return;
                    }
                }
            }
        }

        grpc::Status status = reader->Finish();

        if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Dismiss report failed..\nGrpc returned status.ok() == false; code: " +
                                             std::to_string(status.error_code()) +
                                             " message: " + status.error_message();

            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    [this, thread_id, num_to_request] {
                        requestReportsCompleted(thread_id, num_to_request);
                    }
            );
            return;
        }

        emit signal_handleRequestReportCompleted(
            [this, thread_id, num_to_request, no_more_reports_received] {

                //this needs to be popped on the main thread so that the user does not have the ability
                // to navigate to the end of the vector while it has a 'no reports remaining' cap
                if (!no_more_reports_received) {
                    std::scoped_lock<std::shared_mutex> lock(reports_mutex);
                    if (!reports.empty() && reports[reports.size() - 1]->no_more_reports) {
                        reports.pop_back();
                    }
                }

                requestReportsCompleted(thread_id, num_to_request);
            });
    });

    //NOTE: requestReportsCompleted() will run on the main thread, and it cannot be called until this insert
    // statement runs so there is no chance of the remove() being called before the insert() is on the map
    // request_report_threads
    request_report_threads.insert(std::make_pair(request_thread->get_id(), std::move(request_thread)));

    return true;
}

void ReportsWindow::refreshReports() {
    //setting refresh to disabled in case it needs to block at the mutex for some reason (not sure if this
    // is necessary, it's all running on the UI thread)
    ui->refreshReportsPushButton->setEnabled(false);

    const std::chrono::milliseconds current_timestamp = getCurrentTimestamp();

    const std::string current_displayed_user_oid = display_user_window->getCurrentDisplayedUserOID();

    std::unique_lock<std::shared_mutex> lock(reports_mutex);
    for (auto it = reports.begin(); it != reports.end();) {


        if (!(*it)->no_more_reports && (*it)->checkout_expiration_time <= current_timestamp) {

            if (current_displayed_user_oid == (*it)->user_reports.user_account_oid()) {
                //clear currently displayed report if necessary
                clearCurrentReport();
            }

            it = reports.erase(it);
        } else {
            it++;
        }
    }
    lock.unlock();

    // the end, the GUI will not be rebuilt twice for each time
    if (!requestReports()) {
        std::cout << "!requestReports()\n";
        ui->refreshReportsPushButton->setEnabled(true);
    }
}

void ReportsWindow::on_refreshReportsPushButton_clicked() {
    refreshReports();
}

void ReportsWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

ReportsWindow::~ReportsWindow() {
    delete ui;
}

void ReportsWindow::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    refreshReports();
}



