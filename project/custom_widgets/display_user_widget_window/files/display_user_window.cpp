//
// Created by jeremiah on 9/8/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_find_user_window.h" resolved

#include "display_user_window.h"

#include <sstream>
#include <iomanip>
#include <regex>

#include <QLineEdit>
#include <QFontMetrics>
#include <QSpinBox>
#include <QLabel>
#include <QSpacerItem>
#include <QComboBox>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <access_status_invalid_message_dialog.h>

#include <SetFields.grpc.pb.h>
#include <SetAdminFields.grpc.pb.h>

#include "setup_login_info.h"
#include "grpc_channel.h"
#include "plain_text_edit_with_max_chars.h"
#include "ui_display_user_window.h"
#include "display_user_picture.h"
#include "widgets_generated_names/widgets_generated_names.h"

DisplayUserWindow::DisplayUserWindow(bool show_dismiss_report_button, QWidget* parent) :
        QWidget(parent), ui(new Ui::DisplayUserWindow) {

    ui->setupUi(this);

    ui->dismissReportPushButton->setVisible(user_admin_privileges.handle_reports() && show_dismiss_report_button);
    ui->timeOutPushButton->setVisible(user_admin_privileges.handle_reports());

    ui->accountHasBeenDeletedLabel->setVisible(false);

    ui->userInfoTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->userInfoTreeWidget->setUniformRowHeights(false);
    ui->userInfoTreeWidget->setIconSize(QSize(50, 50));
    ui->userInfoTreeWidget->setStyleSheet("QTreeWidget { font-size: " + QString::number(12) + "pt; }");
    ui->userInfoTreeWidget->setVerticalScrollMode(QTreeView::ScrollPerPixel);

    current_user_can_be_updated = user_admin_privileges.update_single_users();

    connect(this, &DisplayUserWindow::signal_internalCompleted, this, &DisplayUserWindow::slot_internalCompleted);

    connect(this, &DisplayUserWindow::signal_userAccessStatusCompleted, this,
            &DisplayUserWindow::slot_userAccessStatusCompleted);
    connect(this, &DisplayUserWindow::signal_userNameUpdateCompleted, this,
            &DisplayUserWindow::slot_userNameUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userGenderUpdateCompleted, this,
            &DisplayUserWindow::slot_userGenderUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userBirthdayUpdateCompleted, this,
            &DisplayUserWindow::slot_userBirthdayUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userEmailUpdateCompleted, this,
            &DisplayUserWindow::slot_userEmailUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userBioUpdateCompleted, this,
            &DisplayUserWindow::slot_userBioUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userCityUpdateCompleted, this,
            &DisplayUserWindow::slot_userCityUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userGenderRangeUpdateCompleted, this,
            &DisplayUserWindow::slot_userGenderRangeUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userAgeRangeUpdateCompleted, this,
            &DisplayUserWindow::slot_userAgeRangeUpdateCompleted);
    connect(this, &DisplayUserWindow::signal_userMaxDistanceUpdateCompleted, this,
            &DisplayUserWindow::slot_userMaxDistanceUpdateCompleted);

    connect(&dismiss_report, &DismissReport::signal_reportRequestSuccessfullyCompleted, this,
            &DisplayUserWindow::slot_handleDismissReportCompleted);
    connect(&dismiss_report, &DismissReport::signal_sendWarning, this,
            &DisplayUserWindow::slot_sendWarning);
    connect(&dismiss_report, &DismissReport::signal_requestCanceled, this,
            &DisplayUserWindow::slot_handleReportRequestCanceled);

    connect(&time_out_user, &TimeOutUser::signal_reportRequestSuccessfullyCompleted, this,
            &DisplayUserWindow::slot_handleTimeOutCompleted);
    connect(&time_out_user, &TimeOutUser::signal_sendWarning, this,
            &DisplayUserWindow::slot_sendWarning);
    connect(&time_out_user, &TimeOutUser::signal_requestCanceled, this,
            &DisplayUserWindow::slot_handleReportRequestCanceled);

    connect(ui->userInfoTreeWidget, &QTreeWidget::itemDoubleClicked, this,
            &DisplayUserWindow::slot_treeWidgetItemDoubleClicked);
}

void DisplayUserWindow::updateInfoForSuccessfulTimeOut(
        const std::string& timed_out_user_oid,
        const std::string& reason,
        const handle_reports::TimeOutUserResponse& time_out_response
) {
    if (timed_out_user_oid == user_account_response->account_oid()) {

        DisciplinaryActionTypeEnum action_taken = DISCIPLINE_ACCOUNT_TIME_OUT_SUSPENDED;

        switch (time_out_response.updated_user_account_status()) {
            case STATUS_SUSPENDED:
                action_taken = DisciplinaryActionTypeEnum::DISCIPLINE_ACCOUNT_TIME_OUT_SUSPENDED;
                break;
            case STATUS_BANNED:
                action_taken = DisciplinaryActionTypeEnum::DISCIPLINE_ACCOUNT_TIME_OUT_BANNED;
                break;
            default:
                QMessageBox::warning(this,
                                     "Error",
                                     QString("Returned invalid access status of '%1' returned.")
                                             .arg(QString::fromStdString(UserAccountStatus_Name(
                                                     time_out_response.updated_user_account_status())))
                );
                return;
        }

        auto combo_box = ui->userInfoTreeWidget->findChild<QComboBox*>(generateComboBoxName(ACCESS_STATUS_NAME));
        auto number_times_timed_out_line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(
                generateLineEditName(NUMBER_TIMES_TIMED_OUT_NAME));

        user_account_response->set_number_of_times_timed_out(user_account_response->number_of_times_timed_out() + 1);

        if (number_times_timed_out_line_edit != nullptr) {
            number_times_timed_out_line_edit->setText(
                    QString::number(user_account_response->number_of_times_timed_out()));
        }

        setupNewAccessStatus(
                std::chrono::milliseconds{time_out_response.timestamp_last_updated()},
                std::chrono::milliseconds{time_out_response.timestamp_suspension_expires()},
                time_out_response.updated_user_account_status(),
                QString::fromStdString(reason),
                action_taken,
                combo_box
        );
    }
}

void DisplayUserWindow::slot_handleTimeOutCompleted(
        const std::string& timed_out_user_oid,
        const std::string& reason,
        const handle_reports::TimeOutUserResponse& time_out_response
) {
    //NOTE: do NOT run completed lambda, it will enable the 2 buttons however there is no reason
    // to allow the user to do something like Time Out a user twice (even though the server protects
    // against it as well)
    updateInfoForSuccessfulTimeOut(timed_out_user_oid, reason, time_out_response);

    emit signal_handleReportCompleted(timed_out_user_oid);
}

void DisplayUserWindow::slot_handleDismissReportCompleted(const std::string& reported_user_oid) {
    //NOTE: do NOT run completed lambda, it will enable the 2 buttons however there is no reason
    // to allow the user to do something like Time Out a user twice (even though the server protects
    // against it as well)

    emit signal_handleReportCompleted(reported_user_oid);
}

void DisplayUserWindow::slot_sendWarning(
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

void DisplayUserWindow::slot_handleReportRequestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void DisplayUserWindow::findUser(
        const BsoncxxOIDWrapper& oid,
        bool request_event,
        const std::function<void()>& completed_lambda
) {
    RequestUserAccountInfoRequest request;
    setup_login_info(request.mutable_login_info());
    request.set_user_account_oid(oid.oid_string);
    request.set_request_event(request_event);
    findUser(request, completed_lambda);
}

void DisplayUserWindow::findUser(
        const std::string& phone_number,
        const std::function<void()>& completed_lambda
) {
    RequestUserAccountInfoRequest request;
    setup_login_info(request.mutable_login_info());
    request.set_user_phone_number(phone_number);
    request.set_request_event(false);
    findUser(request, completed_lambda);
}

void DisplayUserWindow::clearWidget(bool clear_user_info) {
    ui->userInfoTreeWidget->clear();
    clearLayout(ui->picturesHorizontalLayout->layout());

    if (clear_user_info) {
        user_account_response->Clear();
    }
}

void DisplayUserWindow::clearLayout(QLayout* layout) {
    if (layout == nullptr)
        return;
    QLayoutItem* item;
    while ((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void DisplayUserWindow::findUser(
        const RequestUserAccountInfoRequest& request,
        const std::function<void()>& completed_lambda
) {
    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    clearWidget(false);

    function_thread = std::make_unique<std::jthread>([request, completed_lambda, this](
            const std::stop_token& stop_token
    ) {
        grpc::ClientContext context;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });
        RequestUserAccountInfoResponse response;

        std::unique_ptr<RequestUserAccountInfoService::Stub> request_user_account_stub =
                RequestUserAccountInfoService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        //NOTE: the GUI is cleared above
        user_account_response->Clear();

        grpc::Status status = request_user_account_stub->RequestUserAccountInfoRPC(&context, request, &response);

        if (stop_token.stop_requested()) {
            emit signal_internalCompleted(false);
            emit signal_requestCanceled(completed_lambda);
        } else if (!status.ok()) { //if grpc call failed
            const std::string errorMessage =
                    "Request server activities and categories failed.\nGrpc returned status.ok() == false; code: " +
                    std::to_string(status.error_code()) +
                    " message: " + status.error_message();

            emit signal_internalCompleted(false);
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
            );
            return;
        } else if (!response.success()) { //if failed
            const std::string errorMessage = "Requesting info failed.\n" + response.error_msg();

            emit signal_internalCompleted(false);
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
            );
            return;
        }

        user_account_response->Swap(response.mutable_user_account_info());

        emit signal_internalCompleted(true);
        emit signal_requestSuccessfullyCompleted(completed_lambda);
    });
}

QWidget* DisplayUserWindow::generatePushButtonWidget(
        const std::string& label_constant,
        void (DisplayUserWindow::* func)(bool)
) const {

    auto button = new QPushButton("Update");
    button->setObjectName(generatePushButtonName(label_constant));

    if (func != nullptr) {
        connect(button, &QPushButton::clicked, this, func);
    }

    auto button_parent_widget = new QWidget();
    auto button_vLayout = new QVBoxLayout();
    button_vLayout->addSpacerItem(
            new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    button_vLayout->addWidget(button);
    button_vLayout->addSpacerItem(
            new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    button_parent_widget->setLayout(button_vLayout);

    return button_parent_widget;
}

QTreeWidgetItem* DisplayUserWindow::setupSingleRowInTreeWidgetLineEdit(
        const std::string& label_constant,
        const QString& text,
        unsigned int max_chars,
        void (DisplayUserWindow::* func)(bool)
) {

    auto* tree_item = new QTreeWidgetItem();
    tree_item->setText(0, generateLabel(label_constant));

    auto line_edit = new QLineEdit();
    line_edit->setObjectName(generateLineEditName(label_constant));
    line_edit->setText(text);
    line_edit->setMaxLength((int) max_chars);
    line_edit->setReadOnly(true);

    ui->userInfoTreeWidget->addTopLevelItem(tree_item);
    ui->userInfoTreeWidget->setItemWidget(tree_item, 1, line_edit);

    QFontMetrics fm(line_edit->font());
    int pixels_wide = fm.horizontalAdvance(text);
    line_edit->setMinimumWidth(pixels_wide + 2 * fm.averageCharWidth());

    if (
            current_user_can_be_updated
            && func != nullptr
            ) {

        line_edit->setReadOnly(false);
        ui->userInfoTreeWidget->setItemWidget(tree_item, 2, generatePushButtonWidget(label_constant, func));
    }

    return tree_item;
}

void DisplayUserWindow::setupSingleRowInTreeWidgetSpinBox(
        const std::string& label_constant,
        const int min_value,
        const int max_value,
        const int value,
        void (DisplayUserWindow::* func)(bool)
) {

    auto* tree_item = new QTreeWidgetItem();
    tree_item->setText(0, generateLabel(label_constant));

    auto spin_box = new QSpinBox();
    spin_box->setObjectName(generateSpinBoxName(label_constant, 0));
    spin_box->setMinimum(min_value);
    spin_box->setMaximum(max_value);
    spin_box->setValue(value);
    spin_box->setReadOnly(true);

    ui->userInfoTreeWidget->addTopLevelItem(tree_item);
    ui->userInfoTreeWidget->setItemWidget(tree_item, 1, spin_box);

    if (current_user_can_be_updated
        && func != nullptr) {
        spin_box->setReadOnly(false);
        ui->userInfoTreeWidget->setItemWidget(tree_item, 2, generatePushButtonWidget(label_constant, func));
    }
}

std::string DisplayUserWindow::getCurrentDisplayedUserOID() {
    return user_account_response->account_oid();
}

void DisplayUserWindow::setupGUI(CompleteUserAccountInfo* _user_account_response) {
    user_account_response->Swap(_user_account_response);
    setupGUI();
}

void DisplayUserWindow::setupGUI() {

    clearWidget(false);

    int row_height = 0;
    int number_rows = 1;

    const bool account_is_event =
            user_account_response->account_type() >= UserAccountType::ADMIN_GENERATED_EVENT_TYPE &&
            user_account_response->has_event_values();

    //sort pictures by index
    std::sort(user_account_response->mutable_picture_list()->begin(),
              user_account_response->mutable_picture_list()->end(), [](
                    const PictureMessageResult& lhs, const PictureMessageResult& rhs
            ) -> bool {
                return lhs.picture().index_number() < rhs.picture().index_number();
            }
    );

    for (const auto& picture: user_account_response->picture_list()) {

        //If all references for picture have been removed, no reason to show picture at all.
        //NOTE: The picture is not downloaded when picture_references_have_been_deleted() is set to true.
        if (!picture.picture_references_have_been_deleted()
            && picture.has_picture()
            && !picture.picture().file_in_bytes().empty()) {

            if (picture.picture().file_in_bytes().size() != (unsigned int) picture.picture().file_size()) {
                QMessageBox::warning(
                        this,
                        "Error",
                        QString("User picture at index number '%1' was corrupt.")
                                .arg(picture.picture().index_number())
                );
                continue;
            }

            auto* picture_frame = new DisplayUserPicture(
                    user_account_response->account_oid(),
                    picture.picture().picture_oid(),
                    true
            );

            picture_frame->showPicture(
                    picture.picture().file_in_bytes(),
                    picture.picture().file_size()
            );

            connect(picture_frame, &DisplayUserPicture::signal_removeThisIndex, this,
                    &DisplayUserWindow::slot_removePicture);

            ui->picturesHorizontalLayout->addWidget(picture_frame, 0, Qt::AlignHCenter);
        }
    }

    //Admin has privileges to update account AND account is not deleted, user can be updated.
    current_user_can_be_updated =
            user_admin_privileges.update_single_users() && !user_account_response->account_deleted();

    ui->accountHasBeenDeletedLabel->setVisible(user_account_response->account_deleted());

    if (!account_is_event) {
        auto* tree_item = new QTreeWidgetItem();
        tree_item->setText(0, generateLabel(ACCESS_STATUS_NAME));

        auto combo_box = new QComboBox();
        combo_box->setObjectName(generateComboBoxName(ACCESS_STATUS_NAME));

        ui->userInfoTreeWidget->addTopLevelItem(tree_item);
        ui->userInfoTreeWidget->setItemWidget(tree_item, 1, combo_box);
        number_rows++;

        setActiveStatusToComboBox(combo_box);

        if (current_user_can_be_updated) {
            ui->userInfoTreeWidget->setItemWidget(
                    tree_item,
                    2,
                    generatePushButtonWidget(
                            ACCESS_STATUS_NAME,
                            &DisplayUserWindow::slot_setUserAccessStatus
                    )
            );
        }

        row_height = ui->userInfoTreeWidget->visualItemRect(tree_item).height();

        ui->dismissReportPushButton->setEnabled(true);

        if (user_account_response->account_status() == UserAccountStatus::STATUS_SUSPENDED
            || user_account_response->account_status() == UserAccountStatus::STATUS_BANNED
            || user_account_response->account_deleted()
                ) {
            ui->timeOutPushButton->setEnabled(false);
        } else {
            ui->timeOutPushButton->setEnabled(true);
        }

        setupSingleRowInTreeWidgetLineEdit(
                NUMBER_TIMES_TIMED_OUT_NAME,
                QString::number(user_account_response->number_of_times_timed_out()),
                globals->maximum_number_allowed_bytes()
        );

        auto* inactive_message_item = setupSingleRowInTreeWidgetLineEdit(
                INACTIVE_MESSAGE_NAME,
                QString::fromStdString(user_account_response->inactive_message()),
                globals->maximum_number_allowed_bytes_inactive_message()
        );
        number_rows++;

        auto ending_timestamp_string = getDateTimeStringFromTimestamp(
                std::chrono::milliseconds{user_account_response->inactive_end_time()}
        );

        auto* inactive_end_time_item = setupSingleRowInTreeWidgetLineEdit(
                INACTIVE_END_TIME_NAME,
                QString::fromStdString(ending_timestamp_string),
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;

        if (!user_account_response->inactive_message().empty() && user_account_response->inactive_message() != "~") {
            if (user_account_response->inactive_end_time() < 1) {
                //hide inactive end time
                ui->userInfoTreeWidget->setRowHidden(
                        ui->userInfoTreeWidget->indexOfTopLevelItem(inactive_end_time_item),
                        ui->userInfoTreeWidget->rootIndex(),
                        true
                );
            }
        } else {
            //hide inactive message
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(inactive_message_item),
                    ui->userInfoTreeWidget->rootIndex(),
                    true
            );

            //hide inactive end time
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(inactive_end_time_item),
                    ui->userInfoTreeWidget->rootIndex(),
                    true
            );
        }

        auto* disciplinary_actions_item = new QTreeWidgetItem();
        ui->userInfoTreeWidget->addTopLevelItem(disciplinary_actions_item);
        disciplinary_actions_item->setText(0, generateLabel(DISCIPLINARY_ACTIONS_RECORD));
        number_rows++;

        if (user_account_response->disciplinary_actions().empty()) {
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(disciplinary_actions_item),
                    ui->userInfoTreeWidget->rootIndex(),
                    true
            );
        } else {

            for (int i = 0; i < user_account_response->disciplinary_actions().size(); i++) {
                displayDisciplinaryActionInItem(
                        disciplinary_actions_item,
                        user_account_response->disciplinary_actions()[i],
                        i);
            }

        }

        setupSingleRowInTreeWidgetLineEdit(
                LAST_TIME_ACCOUNT_VERIFIED_NAME,
                QString::fromStdString(
                        getDateTimeStringFromTimestamp(
                                std::chrono::milliseconds{
                                        user_account_response->last_time_account_verified()
                                }
                        )
                ),
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                OPTED_IN_TO_PROMO_EMAIL_NAME,
                QString::fromStdString(user_account_response->opted_in_to_promotional_email() ? "Yes" : "No"),
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;
    }

    QTreeWidgetItem* time_account_created_tree_item = setupSingleRowInTreeWidgetLineEdit(
            TIME_ACCOUNT_CREATED,
            QString::fromStdString(
                    getDateTimeStringFromTimestamp(
                            std::chrono::milliseconds{
                                    user_account_response->time_account_created()
                            }
                    )
            ),
            globals->maximum_number_allowed_bytes()
    );
    number_rows++;

    if (account_is_event) {
        row_height = (int) ((float) ui->userInfoTreeWidget->visualItemRect(time_account_created_tree_item).height() *
                            1.9);
    }

    setupSingleRowInTreeWidgetLineEdit(
            ACCOUNT_ID_NAME,
            QString::fromStdString(user_account_response->account_oid()),
            24
    );

    number_rows++;

    setupSingleRowInTreeWidgetLineEdit(
            ACCOUNT_TYPE_NAME,
            QString::fromStdString(UserAccountType_Name(user_account_response->account_type())),
            globals->maximum_number_allowed_bytes()
    );

    number_rows++;

    if (!account_is_event) {
        setupSingleRowInTreeWidgetLineEdit(
                SUBSCRIPTION_STATUS_NAME,
                QString::fromStdString(UserSubscriptionStatus_Name(user_account_response->subscription_status())),
                globals->maximum_number_allowed_bytes()
        );

        number_rows++;

        if (user_account_response->subscription_status() > UserSubscriptionStatus::NO_SUBSCRIPTION) {
            setupSingleRowInTreeWidgetLineEdit(
                    SUBSCRIPTION_EXPIRATION_TIME_NAME,
                    QString::fromStdString(
                            getDateTimeStringFromTimestamp(
                                    std::chrono::milliseconds{
                                            user_account_response->subscription_expiration_time_ms()
                                    }
                            )
                    ),
                    globals->maximum_number_allowed_bytes()
            );

            number_rows++;
        }

        QString phone_number = "+1 (";

        if (user_account_response->phone_number().size() > 2) {
            for (unsigned int i = 2; i < user_account_response->phone_number().size(); i++) {

                if (i == 5) {
                    phone_number += ") ";
                } else if (i == 8) {
                    phone_number += "-";
                }
                phone_number += user_account_response->phone_number()[i];

            }
        } else {
            phone_number = "";
        }

        setupSingleRowInTreeWidgetLineEdit(
                PHONE_NUMBER_NAME,
                phone_number,
                globals->maximum_number_allowed_bytes()
        );

        number_rows++;
        QString algorithm_search_options_str;

        switch (user_account_response->algorithm_search_options()) {
            case AlgorithmSearchOptions::USER_MATCHING_BY_ACTIVITY:
                algorithm_search_options_str = "Only Activities";
                break;
            case AlgorithmSearchOptions::USER_MATCHING_BY_CATEGORY_AND_ACTIVITY:
                algorithm_search_options_str = "Categories & Activities";
                break;
            default:
                algorithm_search_options_str = "Err unknown value.";
                break;
        }

        setupSingleRowInTreeWidgetLineEdit(
                ALGORITHM_SEARCH_BY_NAME,
                algorithm_search_options_str,
                globals->maximum_number_allowed_bytes());

        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                USER_NAME_NAME,
                QString::fromStdString(user_account_response->user_name()),
                globals->maximum_number_allowed_bytes_first_name(),
                &DisplayUserWindow::slot_setUserFirstName
        );
        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                GENDER_NAME,
                QString::fromStdString(user_account_response->gender()),
                globals->maximum_number_allowed_bytes(),
                &DisplayUserWindow::slot_setUserGender);
        number_rows++;

        auto* birthday_tree_item = new QTreeWidgetItem();
        ui->userInfoTreeWidget->addTopLevelItem(birthday_tree_item);
        number_rows++;

        birthday_tree_item->setText(0, generateLabel(BIRTHDAY_NAME));

        auto b_day_month_spin_box = new QSpinBox();
        b_day_month_spin_box->setMinimum(1);
        b_day_month_spin_box->setMaximum(12);
        b_day_month_spin_box->setValue(user_account_response->birthday_info().birth_month());
        b_day_month_spin_box->setReadOnly(true);
        b_day_month_spin_box->setObjectName(generateSpinBoxName(BIRTHDAY_NAME, 0));

        auto b_day_month_day_divider_label = new QLabel();
        b_day_month_day_divider_label->setText("/");
        b_day_month_day_divider_label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

        auto b_day_day_spin_box = new QSpinBox();
        b_day_day_spin_box->setMinimum(1);
        b_day_day_spin_box->setMaximum(31);
        b_day_day_spin_box->setValue(user_account_response->birthday_info().birth_day_of_month());
        b_day_day_spin_box->setReadOnly(true);
        b_day_day_spin_box->setObjectName(generateSpinBoxName(BIRTHDAY_NAME, 1));

        auto b_day_day_year_divider_label = new QLabel();
        b_day_day_year_divider_label->setText("/");
        b_day_day_year_divider_label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

        time_t theTime = time(nullptr);
        struct tm* aTime = localtime(&theTime);

        int year = aTime->tm_year + 1900;

        auto b_day_year_spin_box = new QSpinBox();
        b_day_year_spin_box->setMinimum(year - (int) globals->highest_allowed_age());
        b_day_year_spin_box->setMaximum(year - (int) globals->lowest_allowed_age());
        b_day_year_spin_box->setValue(user_account_response->birthday_info().birth_year());
        b_day_year_spin_box->setReadOnly(true);
        b_day_year_spin_box->setObjectName(generateSpinBoxName(BIRTHDAY_NAME, 2));

        auto* b_day_parent_widget = new QWidget();
        auto* b_day_hLayout = new QHBoxLayout();
        b_day_hLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
        b_day_hLayout->addWidget(b_day_month_spin_box);
        b_day_hLayout->addWidget(b_day_month_day_divider_label);
        b_day_hLayout->addWidget(b_day_day_spin_box);
        b_day_hLayout->addWidget(b_day_day_year_divider_label);
        b_day_hLayout->addWidget(b_day_year_spin_box);
        b_day_hLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
        b_day_hLayout->addStretch();
        b_day_parent_widget->setLayout(b_day_hLayout);

        ui->userInfoTreeWidget->setItemWidget(birthday_tree_item, 1, b_day_parent_widget);

        if (current_user_can_be_updated) {
            b_day_month_spin_box->setReadOnly(false);
            b_day_day_spin_box->setReadOnly(false);
            b_day_year_spin_box->setReadOnly(false);
            ui->userInfoTreeWidget->setItemWidget(
                    birthday_tree_item,
                    2,
                    generatePushButtonWidget(
                            BIRTHDAY_NAME,
                            &DisplayUserWindow::slot_setUserBirthday
                    )
            );
        }

        setupSingleRowInTreeWidgetLineEdit(
                AGE_NAME,
                QString::number(user_account_response->birthday_info().age()),
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                EMAIL_ADDRESS_NAME,
                QString::fromStdString(user_account_response->email_info().email()),
                globals->maximum_number_allowed_bytes(),
                &DisplayUserWindow::slot_setUserEmail);
        number_rows++;

        QString email_verified_string;

        if (user_account_response->email_info().requires_email_verification()) {
            email_verified_string = "False";
        } else {
            email_verified_string = "True";
        }

        setupSingleRowInTreeWidgetLineEdit(
                EMAIL_ADDRESS_VERIFIED_NAME,
                email_verified_string,
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;
    } else { //is event
        setupSingleRowInTreeWidgetLineEdit(
                EVENT_TITLE_NAME,
                QString::fromStdString(user_account_response->event_values().event_title()),
                globals->maximum_number_allowed_bytes()
        );

        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                EVENT_CREATED_BY_NAME,
                QString::fromStdString(user_account_response->event_values().created_by()),
                globals->maximum_number_allowed_bytes()
        );

        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                EVENT_CHAT_ROOM_ID_NAME,
                QString::fromStdString(user_account_response->event_values().chat_room_id()),
                globals->maximum_number_allowed_bytes()
        );

        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                EVENT_STATUS_NAME,
                QString::fromStdString(LetsGoEventStatus_Name(user_account_response->event_values().event_state())),
                globals->maximum_number_allowed_bytes()
        );

        number_rows++;

        //If this is any other state, the expiration time will be set to a value like -2. This is meaningless for
        // a human.
        if (user_account_response->event_values().event_state() != LetsGoEventStatus::ONGOING
            || user_account_response->event_values().event_state() != LetsGoEventStatus::COMPLETED) {
            setupSingleRowInTreeWidgetLineEdit(
                    EVENT_EXPIRATION_TIME_NAME,
                    QString::fromStdString(
                            getDateTimeStringFromTimestamp(
                                    std::chrono::milliseconds{
                                            user_account_response->event_expiration_time_ms()
                                    }
                            )
                    ),
                    globals->maximum_number_allowed_bytes()
            );
        }

        number_rows++;
    }

    auto* categories_activities_item = new QTreeWidgetItem();
    ui->userInfoTreeWidget->addTopLevelItem(categories_activities_item);
    number_rows++;

    categories_activities_item->setText(
            0,
            account_is_event ? generateLabel(EVENT_ACTIVITIES_NAME) : generateLabel(USER_ACTIVITIES_NAME)
    );

    for (const auto& activity: user_account_response->categories_array()) {

        auto* activity_item = new QTreeWidgetItem();
        categories_activities_item->addChild(activity_item);

        int idx = activity.activity_index();
        if (idx < activities.size()) {

            QPixmap pixmap;
            pixmap.loadFromData(
                    reinterpret_cast<const uchar*>(icons[activities[idx].icon_index()].icon_in_bytes().c_str()),
                    icons[activities[idx].icon_index()].icon_size_in_bytes());
            activity_item->setIcon(0, pixmap);
            activity_item->setText(0, QString::fromStdString(activities[idx].display_name()));
            activity_item->type();

            //puts in a 'remove activity' button
            /*if (current_user_can_be_updated) {
                auto remove_activity_button = new QPushButton("Remove");
                remove_activity_button->setIcon(QIcon(":/close.png"));
                remove_activity_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

                auto activity_parent_widget = new QWidget();
                auto activity_vLayout = new QVBoxLayout();
                activity_vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                activity_vLayout->addWidget(remove_activity_button);
                activity_vLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                activity_vLayout->setAlignment(Qt::Alignment::enum_type::AlignLeft);
                activity_parent_widget->setLayout(activity_vLayout);

                ui->userInfoTreeWidget->setItemWidget(activity_item, 1, activity_parent_widget);
            }*/

            bool add_anytime;
            if (activity.time_frame_array().empty()) {
                add_anytime = true;
            } else {
                long current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();

                int num_added = 0;
                for (const auto& timeframe: activity.time_frame_array()) {
                    if (timeframe.stop_time_frame() <= current_timestamp) {
                        continue;
                    }

                    auto* time_frame_item = new QTreeWidgetItem();
                    activity_item->addChild(time_frame_item);

                    num_added++;

                    std::string timeframe_string;

                    if (timeframe.start_time_frame() > current_timestamp) { //if start time is valid
                        timeframe_string += getDateTimeStringFromTimestamp(
                                std::chrono::milliseconds{timeframe.start_time_frame()}
                        );
                    } else { //if start time is invalid
                        timeframe_string += getDateTimeStringFromTimestamp(
                                std::chrono::milliseconds{current_timestamp}
                        );
                    }
                    timeframe_string += "  to  ";
                    timeframe_string += getDateTimeStringFromTimestamp(
                            std::chrono::milliseconds{timeframe.stop_time_frame()}
                    );
                    time_frame_item->setText(0, timeframe_string.c_str());

                }

                add_anytime = !num_added;
            }

            if (add_anytime) {
                auto* anytime_item = new QTreeWidgetItem();
                activity_item->addChild(anytime_item);

                anytime_item->setText(0, "ANYTIME");
            }

        } else {
            activity_item->setText(0, "ERROR_ACTIVITY_INDEX_OOR: " + QString::number(idx));
        }
    }

    auto* bio_tree_item = new QTreeWidgetItem();
    ui->userInfoTreeWidget->addTopLevelItem(bio_tree_item);
    number_rows++;

    bio_tree_item->setText(0, account_is_event ? generateLabel(DESCRIPTION_NAME) : generateLabel(BIO_NAME));

    QString bio_text;
    if (!user_account_response->post_login_info().user_bio().empty() &&
        user_account_response->post_login_info().user_bio() != "~") {
        bio_text = QString::fromStdString(user_account_response->post_login_info().user_bio());
    }

    auto text_edit = new PlainTextEditWithMaxChars();
    text_edit->setObjectName(generatePlainTextEditName(BIO_NAME));
    text_edit->setPlainText(bio_text);
    text_edit->setMaxChar((int) globals->maximum_number_allowed_bytes_user_bio());
    text_edit->setReadOnly(true);

    ui->userInfoTreeWidget->setItemWidget(bio_tree_item, 1, text_edit);

    if (current_user_can_be_updated) {
        text_edit->setReadOnly(false);
        ui->userInfoTreeWidget->setItemWidget(
                bio_tree_item,
                2,
                generatePushButtonWidget(
                        BIO_NAME,
                        &DisplayUserWindow::slot_setUserBio
                )
        );
    }

    QString city_text;
    if (!user_account_response->post_login_info().user_city().empty() &&
        user_account_response->post_login_info().user_city() != "~") {
        city_text = QString::fromStdString(
                user_account_response->post_login_info().user_city());
    }

    setupSingleRowInTreeWidgetLineEdit(
            CITY_NAME,
            city_text,
            globals->maximum_number_allowed_bytes(),
            &DisplayUserWindow::slot_setUserCity
    );
    number_rows++;

    auto* gender_range_tree_item = new QTreeWidgetItem();
    ui->userInfoTreeWidget->addTopLevelItem(gender_range_tree_item);
    number_rows++;

    gender_range_tree_item->setText(0, generateLabel(
            GENDERS_MATCHING_WITH_NAME + "\n\n" + globals->everyone_gender_string() + "," +
            globals->male_gender_string() + "," + globals->female_gender_string())
    );

    auto gender_range_parent_widget = new QWidget();
    auto gender_range_vLayout = new QVBoxLayout();

    for (int i = 0; i < (int) globals->number_genders_can_match_with(); i++) {
        QString text;

        if (i < user_account_response->post_login_info().gender_range().size()) {
            text = QString::fromStdString(user_account_response->post_login_info().gender_range().at(i));
        }

        auto line_edit = new QLineEdit();
        line_edit->setObjectName(generateIndexedLineEditName(GENDERS_MATCHING_WITH_NAME, i));
        line_edit->setMaxLength((int) globals->maximum_number_allowed_bytes());

        QFontMetrics fm(line_edit->font());
        int pixels_wide = fm.horizontalAdvance(text);
        line_edit->setMinimumWidth(pixels_wide + 2 * fm.averageCharWidth());

        line_edit->setText(text);

        if (!current_user_can_be_updated) {
            line_edit->setReadOnly(true);
        }

        gender_range_vLayout->addWidget(line_edit);
    }

    gender_range_parent_widget->setLayout(gender_range_vLayout);

    ui->userInfoTreeWidget->setItemWidget(gender_range_tree_item, 1, gender_range_parent_widget);

    if (current_user_can_be_updated) {
        ui->userInfoTreeWidget->setItemWidget(
                gender_range_tree_item, 2,
                generatePushButtonWidget(
                        GENDERS_MATCHING_WITH_NAME,
                        &DisplayUserWindow::slot_setUserGenderRange
                )
        );
    }

    auto* age_range_tree_item = new QTreeWidgetItem();
    ui->userInfoTreeWidget->addTopLevelItem(age_range_tree_item);
    number_rows++;

    age_range_tree_item->setText(0, generateLabel(AGE_RANGE_MATCHING_WITH_NAME));

    auto min_age_spin_box = new QSpinBox();
    min_age_spin_box->setObjectName(generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 0));
    min_age_spin_box->setMinimum((int) globals->lowest_allowed_age());
    min_age_spin_box->setMaximum((int) globals->highest_allowed_age());
    min_age_spin_box->setValue(user_account_response->post_login_info().min_age());
    min_age_spin_box->setReadOnly(true);

    auto age_range_divider_label = new QLabel();
    age_range_divider_label->setText(" - ");
    age_range_divider_label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    auto max_age_spin_box = new QSpinBox();
    max_age_spin_box->setObjectName(generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 1));
    max_age_spin_box->setMinimum((int) globals->lowest_allowed_age());
    max_age_spin_box->setMaximum((int) globals->highest_allowed_age());
    max_age_spin_box->setValue(user_account_response->post_login_info().max_age());
    max_age_spin_box->setReadOnly(true);

    auto* age_range_parent_widget = new QWidget();
    auto* age_range_hLayout = new QHBoxLayout();
    age_range_hLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed));
    age_range_hLayout->addWidget(min_age_spin_box);
    age_range_hLayout->addWidget(age_range_divider_label);
    age_range_hLayout->addWidget(max_age_spin_box);
    age_range_hLayout->addStretch();
    age_range_parent_widget->setLayout(age_range_hLayout);

    ui->userInfoTreeWidget->setItemWidget(age_range_tree_item, 1, age_range_parent_widget);

    if (current_user_can_be_updated) {
        min_age_spin_box->setReadOnly(false);
        max_age_spin_box->setReadOnly(false);
        ui->userInfoTreeWidget->setItemWidget(
                age_range_tree_item,
                2,
                generatePushButtonWidget(
                        AGE_RANGE_MATCHING_WITH_NAME,
                        &DisplayUserWindow::slot_setUserAgeRange
                )
        );
    }

    setupSingleRowInTreeWidgetSpinBox(
            MAX_DISTANCE_MATCHING_WITH_NAME,
            (int) globals->minimum_allowed_distance(),
            (int) globals->maximum_allowed_distance(),
            user_account_response->post_login_info().max_distance(),
            &DisplayUserWindow::slot_setUserMaxDistance
    );
    number_rows++;

    std::stringstream latitude_text;

    latitude_text
            << std::fixed
            << std::setprecision(15)
            << user_account_response->location().client_latitude();

    setupSingleRowInTreeWidgetLineEdit(
            LOCATION_LATITUDE_NAME,
            QString::fromStdString(latitude_text.str()),
            globals->maximum_number_allowed_bytes());
    number_rows++;

    std::stringstream longitude_text;

    longitude_text
            << std::fixed
            << std::setprecision(15)
            << user_account_response->location().client_longitude();

    setupSingleRowInTreeWidgetLineEdit(
            LOCATION_LONGITUDE_NAME,
            QString::fromStdString(longitude_text.str()),
            globals->maximum_number_allowed_bytes());
    number_rows++;

    if (!account_is_event) {

        auto* user_created_events_item = new QTreeWidgetItem();
        ui->userInfoTreeWidget->addTopLevelItem(user_created_events_item);
        user_created_events_item->setText(0, generateLabel(USER_CREATED_EVENTS_NAME));
        number_rows++;

        if (user_account_response->user_created_events().empty()) {
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(user_created_events_item),
                    ui->userInfoTreeWidget->rootIndex(),
                    true
            );
        } else {
            for (int i = 0; i < user_account_response->user_created_events_size(); i++) {
                displayUserCreatedEventInItem(
                        user_created_events_item,
                        user_account_response->user_created_events(i)
                );
            }
        }

        setupSingleRowInTreeWidgetLineEdit(
                LAST_TIME_ALGORITHM_ATTEMPTED_NAME,
                QString::fromStdString(
                        getDateTimeStringFromTimestamp(
                                std::chrono::milliseconds{
                                        user_account_response->timestamp_find_matches_last_ran()}
                        )
                ),
                globals->maximum_number_allowed_bytes());
        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                CURRENT_NUM_OTHER_USERS_BLOCKED_NAME,
                QString::number(user_account_response->number_of_other_users_blocked()),
                globals->maximum_number_allowed_bytes());
        number_rows++;

        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SWIPED_YES_NAME,
                QString::number(user_account_response->number_of_times_swiped_yes()),
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SWIPED_NO_NAME,
                QString::number(user_account_response->number_of_times_swiped_no()),
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SWIPED_BLOCK_AND_REPORT_NAME,
                QString::number(user_account_response->number_of_times_swiped_block_report()),
                globals->maximum_number_allowed_bytes()
        );
        number_rows++;

    }

    setupSingleRowInTreeWidgetLineEdit(
            NUM_TIMES_SWIPED_YES_ON_BY_OTHERS_NAME,
            QString::number(
                    user_account_response->number_of_times_others_swiped_yes_on()),
            globals->maximum_number_allowed_bytes());
    number_rows++;
    setupSingleRowInTreeWidgetLineEdit(
            NUM_TIMES_SWIPED_NO_ON_BY_OTHERS_NAME,
            QString::number(user_account_response->number_of_times_others_swiped_no_on()),
            globals->maximum_number_allowed_bytes());
    number_rows++;
    setupSingleRowInTreeWidgetLineEdit(
            NUM_TIMES_SWIPED_BLOCK_AND_REPORT_ON_BY_OTHERS_NAME,
            QString::number(
                    user_account_response->number_of_times_others_swiped_block_report()),
            globals->maximum_number_allowed_bytes());
    number_rows++;

    if (!account_is_event) {
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SENT_ACTIVITY_SUGGESTION_NAME,
                QString::number(
                        user_account_response->number_of_times_sent_activity_suggestion()),
                globals->maximum_number_allowed_bytes());
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SENT_BUG_REPORT_NAME,
                QString::number(user_account_response->number_of_times_sent_bug_report()),
                globals->maximum_number_allowed_bytes());
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SENT_OTHER_SUGGESTION_NAME,
                QString::number(
                        user_account_response->number_of_times_sent_other_suggestion()),
                globals->maximum_number_allowed_bytes());
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SPAM_FEEDBACK_SENT_NAME,
                QString::number(
                        user_account_response->number_of_times_spam_feedback_sent()),
                globals->maximum_number_allowed_bytes());
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_SPAM_REPORT_SENT_NAME,
                QString::number(
                        user_account_response->number_of_times_spam_reports_sent()),
                globals->maximum_number_allowed_bytes());
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_BLOCK_AND_REPORT_USED_IN_CHAT_NAME, QString::number(
                        user_account_response->number_of_times_this_user_blocked_reported_from_chat_room()),
                globals->maximum_number_allowed_bytes());
        number_rows++;
        setupSingleRowInTreeWidgetLineEdit(
                NUM_TIMES_BLOCK_AND_REPORT_USED_ON_BY_OTHERS_IN_CHAT_NAME,
                QString::number(
                        user_account_response->number_of_times_blocked_reported_by_others_in_chat_room()),
                globals->maximum_number_allowed_bytes());
        number_rows++;
    }

    ui->userInfoTreeWidget->setFixedHeight(number_rows * row_height);
    ui->userInfoTreeWidget->viewport()->update();
}

void DisplayUserWindow::slot_internalCompleted(bool successfully_loaded) {
    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
        function_thread = nullptr;

        //if failed to load, break here
        if (!successfully_loaded) {
            return;
        }

        setupGUI();
    }
}

void DisplayUserWindow::setActiveStatusToComboBox(QComboBox* combo_box) {

    combo_box->clear();

    QString account_status_active = ACCOUNT_STATUS_ACTIVE;
    QString account_status_requires_info = ACCOUNT_STATUS_REQUIRES_INFO;
    QString account_status_suspended = ACCOUNT_STATUS_SUSPENDED;
    QString account_status_banned = ACCOUNT_STATUS_BANNED;

    switch (user_account_response->account_status()) {
        case STATUS_ACTIVE:
            account_status_active.insert(0, "-> ");
            break;
        case STATUS_REQUIRES_MORE_INFO:
            account_status_requires_info.insert(0, "-> ");
            break;
        case STATUS_SUSPENDED:
            account_status_suspended.insert(0, "-> ");
            break;
        case STATUS_BANNED:
            account_status_banned.insert(0, "-> ");
            break;
        default:
            combo_box->addItem("ERROR_UNKNOWN_VALUE: " + QString::number(user_account_response->account_status()));
            break;
    }

    if (current_user_can_be_updated) {

        combo_box->addItem(account_status_active);
        combo_box->addItem(account_status_requires_info);
        combo_box->addItem(account_status_suspended);
        combo_box->addItem(account_status_banned);

        if (user_account_response->account_status() < combo_box->count()) {
            combo_box->setCurrentIndex(user_account_response->account_status());
        } else {
            combo_box->clear();
            combo_box->addItem("ERROR_UNKNOWN_VALUE: " + QString::number(user_account_response->account_status()));
        }

    } else {
        switch (user_account_response->account_status()) {

            case STATUS_ACTIVE:
                combo_box->addItem(account_status_active);
                break;
            case STATUS_REQUIRES_MORE_INFO:
                combo_box->addItem(account_status_requires_info);
                break;
            case STATUS_SUSPENDED:
                combo_box->addItem(account_status_suspended);
                break;
            case STATUS_BANNED:
                combo_box->addItem(account_status_banned);
                break;
            default:
                combo_box->addItem(
                        "ERROR_UNKNOWN_VALUE: " + QString::number(user_account_response->account_status()));
                break;
        }
    }
}

void DisplayUserWindow::displayDisciplinaryActionInItem(
        QTreeWidgetItem* disciplinary_actions_item,
        const UserDisciplinaryAction& action,
        int index
) const {
    auto* disciplinary_action = new QTreeWidgetItem();
    disciplinary_actions_item->addChild(disciplinary_action);
    disciplinary_action->setText(0, "Index " + QString::number(index));

    QString action_string;
    switch (action.type()) {
        case DISCIPLINE_ACCOUNT_TIME_OUT_SUSPENDED:
            action_string += "Suspended";
            break;
        case DISCIPLINE_ACCOUNT_TIME_OUT_BANNED:
            action_string += "Banned";
            break;
        case DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_SUSPENDED:
            action_string += "Manually set to suspended";
            break;
        case DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_BANNED:
            action_string += "Manually set to banned";
            break;
        case DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_ACTIVE:
            action_string += "Manually set to active";
            break;
        case DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_REQUIRES_MORE_INFO:
            action_string += "Manually set to requires more info";
            break;
        default:
            action_string += "Error";
            break;
    }

    auto* action_item = new QTreeWidgetItem();
    disciplinary_action->addChild(action_item);
    action_item->setText(0, "Action: ");
    action_item->setText(1, action_string);

    QString reason_string;

    if (action.reason().empty()) {
        reason_string += "NONE";
    } else {
        reason_string += QString::fromStdString(action.reason());
    }

    auto* reason_item = new QTreeWidgetItem();
    disciplinary_action->addChild(reason_item);
    reason_item->setText(0, "Reason: ");
    reason_item->setText(1, reason_string);

    std::string submitted_timestamp_string = getDateTimeStringFromTimestamp(
            std::chrono::milliseconds{action.timestamp_submitted()}
    );

    auto* time_submitted_item = new QTreeWidgetItem();
    disciplinary_action->addChild(time_submitted_item);
    time_submitted_item->setText(0, "Time Submitted: ");
    time_submitted_item->setText(1, QString::fromStdString(submitted_timestamp_string));

    if (action.timestamp_ends() > -1) {
        std::string ending_timestamp_string = getDateTimeStringFromTimestamp(
                std::chrono::milliseconds{action.timestamp_ends()}
        );

        auto* time_ends_item = new QTreeWidgetItem();
        disciplinary_action->addChild(time_ends_item);
        time_ends_item->setText(0, "Ending Time: ");
        time_ends_item->setText(1, QString::fromStdString(ending_timestamp_string));
    }

    if (current_user_can_be_updated) {
        auto* admin_name_item = new QTreeWidgetItem();
        disciplinary_action->addChild(admin_name_item);
        admin_name_item->setText(0, "Admin Name: ");
        admin_name_item->setText(1, QString::fromStdString(action.admin_name()));
    }

    disciplinary_action->setExpanded(true);
}

void DisplayUserWindow::displayUserCreatedEventInItem(
        QTreeWidgetItem* user_created_event_item,
        const UserCreatedEvent& event_message
) const {

    auto* user_created_event_action = new QTreeWidgetItem();
    QString event_oid = QString::fromStdString(event_message.event_oid());
    user_created_event_item->addChild(user_created_event_action);
    user_created_event_action->setText(0, "Id: ");
    user_created_event_action->setText(1, event_oid);

    QJsonObject doc;
    doc.insert(DOUBLE_CLICK_JSON_ENUM_KEY, QString::number((int) TypeOfDoubleClick::USER_CREATED_EVENT));
    doc.insert(DOUBLE_CLICK_JSON_OID_KEY, event_oid);

    user_created_event_action->setData(0, Qt::UserRole, doc);
    user_created_event_action->setData(1, Qt::UserRole, doc);

    auto* expiration_time_item = new QTreeWidgetItem();
    QString expiration_time = QString::fromStdString(getDateTimeStringFromTimestamp(
            std::chrono::milliseconds{
                    event_message.expiration_time_long()
            }
    ));
    user_created_event_action->addChild(expiration_time_item);
    expiration_time_item->setText(0, "Expiration Time: ");
    expiration_time_item->setText(1, expiration_time);

    expiration_time_item->setData(0, Qt::UserRole, doc);
    expiration_time_item->setData(1, Qt::UserRole, doc);

    QString event_state_string = QString::fromStdString(LetsGoEventStatus_Name(event_message.event_state()));

    auto* event_state_item = new QTreeWidgetItem();
    user_created_event_action->addChild(event_state_item);
    event_state_item->setText(0, "State: ");
    event_state_item->setText(1, event_state_string);

    event_state_item->setData(0, Qt::UserRole, doc);
    event_state_item->setData(1, Qt::UserRole, doc);

    user_created_event_action->setExpanded(false);
}

DisplayUserWindow::~DisplayUserWindow() {

    for (auto& t: set_user_access_status) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_name_threads) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_gender) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_birthday) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_email) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_bio) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_city) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_gender_range) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_age_range) {
        t.second.request_stop();
        t.second.join();
    }

    for (auto& t: set_user_max_distance) {
        t.second.request_stop();
        t.second.join();
    }

    delete ui;
}

void DisplayUserWindow::slot_setUserAccessStatus([[maybe_unused]] bool clicked) {

    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(ACCESS_STATUS_NAME));
    auto combo_box = ui->userInfoTreeWidget->findChild<QComboBox*>(generateComboBoxName(ACCESS_STATUS_NAME));

    if (combo_box == nullptr || update_button == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Combo box or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    auto user_account_status = UserAccountStatus(combo_box->currentIndex());

    if (user_account_status == user_account_response->account_status()) {
        QMessageBox::warning(this,
                             "Error",
                             "Please select a different account status in order to update it."
        );
        return;
    } else if (user_account_response->account_status() == UserAccountStatus::STATUS_REQUIRES_MORE_INFO) {
        QMessageBox::warning(this,
                             "Error",
                             "Do not change accounts that still require info collection."
        );
        return;
    }

    switch (user_account_status) {
        case STATUS_ACTIVE:
        case STATUS_REQUIRES_MORE_INFO:
        case STATUS_SUSPENDED:
        case STATUS_BANNED:
            break;
        default:
            QMessageBox::warning(this,
                                 "Error",
                                 "Invalid user account status extracted from UI."
            );
            return;
    }

    AccessStatusInvalidMessageDialog dialog(user_account_status);
    auto return_status = AccessStatusInvalidMessageDialog::ReturnStatusForDialog(dialog.exec());
    int duration = -1;
    QString reason;

    switch (return_status) {
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_OK:
            reason = dialog.getReasonString();
            duration = dialog.getDurationInHours();
            break;
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_CANCEL:
            return;
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_ERROR:
            QMessageBox::warning(this,
                                 "Error",
                                 dialog.getErrorString()
            );
            return;
    }

    if (reason.size() < globals->minimum_number_allowed_bytes_inactive_message()) {
        QMessageBox::warning(this,
                             "Error",
                             "Inactive reason too short, " +
                             QString::number(globals->minimum_number_allowed_bytes_inactive_message()) +
                             " characters required."
        );
        return;
    } else if (user_account_status == STATUS_SUSPENDED
               && duration == -1) {
        QMessageBox::warning(this,
                             "Error",
                             "Failed to extract suspension duration."
        );
        return;
    }

    update_button->setEnabled(false);
    combo_box->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_access_status.contains(current_oid)) {
        set_user_access_status[current_oid].request_stop();
        set_user_access_status[current_oid].join();
        set_user_access_status.erase(current_oid);
    }

    set_user_access_status.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, user_account_status, reason, duration, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        set_admin_fields::SetAccessStatusRequest request;
                        set_admin_fields::SetAccessStatusResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<set_admin_fields::SetAdminFieldsService::Stub> set_fields_stub =
                                set_admin_fields::SetAdminFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_new_account_status(user_account_status);
                        request.set_inactive_message(reason.toStdString());

                        if (duration < 1) {
                            request.set_duration_in_millis(-1);
                        } else {
                            request.set_duration_in_millis(duration * 60 * 60 * 1000);
                        }

                        const grpc::Status status = set_fields_stub->SetServerAccessStatusRPC(
                                &context,
                                request,
                                &response
                        );

                        const std::string basic_error = "Set user access status failed.\n";
                        if (stop_token.stop_requested()) {
                            //NOTE: empty error string will not be printed
                            emit signal_userAccessStatusCompleted(current_oid,
                                                                  false,
                                                                  "");
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(
                                                                     status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userAccessStatusCompleted(current_oid,
                                                                  false,
                                                                  errorMessage);
                            return;
                        } else if (!response.successful()) { //if failed to log in
                            emit signal_userAccessStatusCompleted(current_oid,
                                                                  response.successful(),
                                                                  basic_error +
                                                                  response.error_message());
                            return;
                        }

                        emit signal_userAccessStatusCompleted(current_oid,
                                                              response.successful(),
                                                              response.error_message(),
                                                              std::chrono::milliseconds{response.current_timestamp()},
                                                              std::chrono::milliseconds{response.final_timestamp()},
                                                              user_account_status,
                                                              reason);
                    })
            )

    );

}

void DisplayUserWindow::slot_setUserFirstName([[maybe_unused]] bool clicked) {

    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(USER_NAME_NAME));
    auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(USER_NAME_NAME));

    if (line_edit == nullptr || update_button == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Line edit or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    std::string new_user_name = line_edit->text().toStdString();

    trimWhitespace(new_user_name);

    if (new_user_name == user_account_response->user_name()) {
        QMessageBox::warning(this,
                             "Error",
                             "Please select a different user name in order to update it."
        );
        return;
    } else if (new_user_name.size() < 2 || globals->maximum_number_allowed_bytes_first_name() < new_user_name.size()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Name must be between %1 and %2 characters long")
                                     .arg(2)
                                     .arg(globals->maximum_number_allowed_bytes_first_name())
        );
        return;
    }

    for (char c: new_user_name) {
        if (!isalpha(c)) {
            QMessageBox::warning(this,
                                 "Error",
                                 QString("Name currently must contain all alphabetic characters.\n'%1' is invalid.")
                                         .arg(c)
            );
            return;
        }
    }

    update_button->setEnabled(false);
    line_edit->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_name_threads.contains(current_oid)) {
        set_user_name_threads[current_oid].request_stop();
        set_user_name_threads[current_oid].join();
        set_user_name_threads.erase(current_oid);
    }

    set_user_name_threads.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, new_user_name, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetStringRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_set_string(new_user_name);

                        grpc::Status status = set_fields_stub->SetFirstNameRPC(&context,
                                                                               request,
                                                                               &response);

                        const std::string basic_error = "Set user name failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userNameUpdateCompleted(current_oid,
                                                                ReturnStatus::VALUE_NOT_SET,
                                                                "", "");
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(
                                                                     status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userNameUpdateCompleted(current_oid,
                                                                ReturnStatus::LG_ERROR,
                                                                errorMessage, "");
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userNameUpdateCompleted(current_oid,
                                                                response.return_status(),
                                                                basic_error +
                                                                response.error_string(),
                                                                "");
                            return;
                        }

                        emit signal_userNameUpdateCompleted(current_oid,
                                                            response.return_status(),
                                                            response.error_string(),
                                                            new_user_name);
                    })
            )

    );

}

void DisplayUserWindow::slot_setUserGender([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(GENDER_NAME));
    auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(GENDER_NAME));

    if (line_edit == nullptr || update_button == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Line edit or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    const std::string new_gender = line_edit->text().toStdString();

    if (new_gender == user_account_response->gender()) {
        QMessageBox::warning(this,
                             "Error",
                             "Please select a different gender in order to update it."
        );
        return;
    } else if (new_gender.empty() || globals->maximum_number_allowed_bytes() < new_gender.size()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Gender must be between %1 and %2 characters long")
                                     .arg(1)
                                     .arg(globals->maximum_number_allowed_bytes())
        );
        return;
    }

    update_button->setEnabled(false);
    line_edit->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_gender.contains(current_oid)) {
        set_user_gender[current_oid].request_stop();
        set_user_gender[current_oid].join();
        set_user_gender.erase(current_oid);
    }

    set_user_gender.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, new_gender, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetStringRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_set_string(new_gender);

                        grpc::Status status = set_fields_stub->SetGenderRPC(&context,
                                                                            request,
                                                                            &response);

                        const std::string basic_error = "Set user gender failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userGenderUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    "",
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userGenderUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage,
                                    ""
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userGenderUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error +
                                    response.error_string(),
                                    ""
                            );
                            return;
                        }

                        emit signal_userGenderUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            new_gender
                    );
                    })
            )
    );
}

void DisplayUserWindow::slot_setUserBirthday([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(BIRTHDAY_NAME));
    auto month_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(generateSpinBoxName(BIRTHDAY_NAME, 0));
    auto day_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(generateSpinBoxName(BIRTHDAY_NAME, 1));
    auto year_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(generateSpinBoxName(BIRTHDAY_NAME, 2));

    auto age_range_update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
            generatePushButtonName(AGE_RANGE_MATCHING_WITH_NAME));
    auto age_range_min_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
            generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 0));
    auto age_range_max_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
            generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 1));

    if (update_button == nullptr || month_spin_box == nullptr || day_spin_box == nullptr || year_spin_box == nullptr
        || age_range_update_button == nullptr || age_range_min_spin_box == nullptr ||
        age_range_max_spin_box == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Spin box or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    const int birth_day = day_spin_box->value();
    const int birth_month = month_spin_box->value();
    const int birth_year = year_spin_box->value();

    std::chrono::milliseconds current_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());

    const time_t time_object = current_time.count() / 1000;
    tm* a_time = gmtime(&time_object);

    int year = a_time->tm_year + 1900;

    if (birth_year == user_account_response->birthday_info().birth_year()
        && birth_month == user_account_response->birthday_info().birth_month()
        && birth_day == user_account_response->birthday_info().birth_day_of_month()
            ) {
        QMessageBox::warning(this,
                             "Error",
                             "Change to a new birthday before updating."
        );
        return;
    } else if (birth_year < (year - (int) globals->highest_allowed_age())
               || (year - (int) globals->lowest_allowed_age()) < birth_year
            ) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Invalid birth year must be a value between %1 and %2")
                                     .arg(year - (int) globals->highest_allowed_age())
                                     .arg(year - (int) globals->lowest_allowed_age())
        );
        return;
    } else if (birth_month < 1 || 12 < birth_month) { //this should be a value 1-12
        QMessageBox::warning(this,
                             "Error",
                             QString("Invalid birth month must be a value between %1 and %2")
                                     .arg(1)
                                     .arg(12)
        );
        return;
    } else if (birth_day < 1 || 31 < birth_day) { //this should be a value 1-31
        QMessageBox::warning(this,
                             "Error",
                             QString("Invalid birth day of month must be a value between %1 and %2")
                                     .arg(1)
                                     .arg(31)
        );
        return;
    }

    tm birthday_tm = initializeTmByDate(birth_year, birth_month, birth_day);
    const int user_age = calculateAge(current_time, birth_year, birthday_tm.tm_yday);

    if (user_age < (int) globals->lowest_allowed_age() || (int) globals->highest_allowed_age() < user_age) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Invalid age must be a value between %1 and %2. Currently %3.")
                                     .arg(globals->lowest_allowed_age())
                                     .arg(globals->highest_allowed_age())
                                     .arg(user_age)
        );
        return;
    }

    update_button->setEnabled(false);
    month_spin_box->setEnabled(false);
    day_spin_box->setEnabled(false);
    year_spin_box->setEnabled(false);
    age_range_update_button->setEnabled(false);
    age_range_min_spin_box->setEnabled(false);
    age_range_max_spin_box->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_birthday.contains(current_oid)) {
        set_user_birthday[current_oid].request_stop();
        set_user_birthday[current_oid].join();
        set_user_birthday.erase(current_oid);
    }

    if (set_user_age_range.contains(current_oid)) {
        set_user_age_range[current_oid].request_stop();
        set_user_age_range[current_oid].join();
        set_user_age_range.erase(current_oid);
    }

    set_user_birthday.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([=, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetBirthdayRequest request;
                        setfields::SetBirthdayResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_birth_year(birth_year);
                        request.set_birth_month(birth_month);
                        request.set_birth_day_of_month(birth_day);

                        grpc::Status status = set_fields_stub->SetBirthdayRPC(&context, request, &response);

                        const std::string basic_error = "Set user birthday failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userBirthdayUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userBirthdayUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userBirthdayUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error +
                                    response.error_string()
                            );
                            return;
                        }

                        emit signal_userBirthdayUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            birth_year,
                            birth_month,
                            birth_day,
                            response.age(),
                            response.min_age_range(),
                            response.max_age_range()
                    );
                    })
            )
    );
}

void DisplayUserWindow::slot_setUserEmail([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(EMAIL_ADDRESS_NAME));
    auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(EMAIL_ADDRESS_NAME));

    if (line_edit == nullptr || update_button == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Line edit or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    const std::string new_email = line_edit->text().toStdString();

    std::regex e(globals->email_authorization_regex());

    if (new_email == user_account_response->email_info().email()) {
        QMessageBox::warning(this,
                             "Error",
                             "Please select a different email in order to update it."
        );
        return;
    } else if (new_email.empty() || globals->maximum_number_allowed_bytes() < new_email.size()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Email must be between %1 and %2 characters long")
                                     .arg(1)
                                     .arg(globals->maximum_number_allowed_bytes())
        );
        return;
    } else if (!std::regex_match(new_email, e)) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Invalid email address entered.")
        );
        return;
    }

    update_button->setEnabled(false);
    line_edit->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_email.contains(current_oid)) {
        set_user_email[current_oid].request_stop();
        set_user_email[current_oid].join();
        set_user_email.erase(current_oid);
    }

    set_user_email.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, new_email, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetEmailRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_set_email(new_email);

                        grpc::Status status = set_fields_stub->SetEmailRPC(&context,
                                                                           request,
                                                                           &response);

                        const std::string basic_error = "Set user email failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userEmailUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userEmailUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userEmailUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error + response.error_string()
                            );
                            return;
                        }

                        emit signal_userEmailUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            new_email
                    );
                    })
            )
    );
}

void DisplayUserWindow::slot_setUserBio([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(BIO_NAME));
    auto plain_text_edit = ui->userInfoTreeWidget->findChild<QPlainTextEdit*>(generatePlainTextEditName(BIO_NAME));

    if (plain_text_edit == nullptr || update_button == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Plain text edit or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    std::string new_bio = plain_text_edit->toPlainText().toStdString();

    trimTrailingWhitespace(new_bio);

    if (
            (new_bio == user_account_response->post_login_info().user_bio())
            || (new_bio.empty() && user_account_response->post_login_info().user_bio() == "~")
            || (new_bio == "~" && user_account_response->post_login_info().user_bio().empty())
            ) {
        QMessageBox::warning(this,
                             "Error",
                             "Please select a different bio in order to update it."
        );
        return;
    } else if (globals->maximum_number_allowed_bytes_user_bio() < new_bio.size()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Bio must be less than %1 characters long")
                                     .arg(globals->maximum_number_allowed_bytes_user_bio())
        );
        return;
    }

    if (new_bio.empty()) {
        new_bio = "~";
    }

    update_button->setEnabled(false);
    plain_text_edit->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_bio.contains(current_oid)) {
        set_user_bio[current_oid].request_stop();
        set_user_bio[current_oid].join();
        set_user_bio.erase(current_oid);
    }

    set_user_bio.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, new_bio, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetBioRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_set_string(new_bio);

                        grpc::Status status = set_fields_stub->SetUserBioRPC(&context,
                                                                             request,
                                                                             &response);

                        const std::string basic_error = "Set user bio failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userBioUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userBioUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userBioUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error + response.error_string()
                            );
                            return;
                        }

                        emit signal_userBioUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            new_bio
                    );
                    })
            )
    );
}

void DisplayUserWindow::slot_setUserCity([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(CITY_NAME));
    auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(CITY_NAME));

    if (line_edit == nullptr || update_button == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Line edit or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    std::string new_city_name = line_edit->text().toStdString();

    trimWhitespace(new_city_name);

    if (
            (new_city_name.empty() && user_account_response->post_login_info().user_city() == "~")
            || (new_city_name == "~" && user_account_response->post_login_info().user_city().empty())
            || (new_city_name == user_account_response->post_login_info().user_city())
            ) {
        QMessageBox::warning(this,
                             "Error",
                             "Please select a different city in order to update it."
        );
        return;
    } else if (globals->maximum_number_allowed_bytes() < new_city_name.size()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Email must be less than %1 characters long")
                                     .arg(globals->maximum_number_allowed_bytes())
        );
        return;
    }

    if (new_city_name.empty()) {
        new_city_name = "~";
    }

    update_button->setEnabled(false);
    line_edit->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_city.contains(current_oid)) {
        set_user_city[current_oid].request_stop();
        set_user_city[current_oid].join();
        set_user_city.erase(current_oid);
    }

    set_user_city.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, new_city_name, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetStringRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_set_string(new_city_name);

                        grpc::Status status = set_fields_stub->SetUserCityRPC(&context,
                                                                              request,
                                                                              &response);

                        const std::string basic_error = "Set user city failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userCityUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userCityUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userCityUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error + response.error_string()
                            );
                            return;
                        }

                        emit signal_userCityUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            new_city_name
                    );
                    })
            )
    );
}

void DisplayUserWindow::slot_setUserGenderRange([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
            generatePushButtonName(GENDERS_MATCHING_WITH_NAME));

    bool one_line_edit_found = false;
    bool everyone_set = false;
    std::set<std::string> genders_to_match_set;
    std::vector<QLineEdit*> line_edits;
    for (int i = 0; i < (int) globals->number_genders_can_match_with(); i++) {
        auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(
                generateIndexedLineEditName(GENDERS_MATCHING_WITH_NAME, i));

        if (line_edit != nullptr) {

            line_edits.emplace_back(line_edit);

            one_line_edit_found = true;
            std::string gender_to_match = line_edit->text().toStdString();

            if (gender_to_match.empty() || everyone_set) {
                continue;
            } else if (globals->maximum_number_allowed_bytes() < gender_to_match.size()) {
                QMessageBox::warning(this,
                                     "Error",
                                     QString("Genders to match with must be less than %1 bytes long")
                                             .arg(globals->maximum_number_allowed_bytes())
                );
                return;
            } else if (genders_to_match_set.find(gender_to_match) != genders_to_match_set.end()) {
                QMessageBox::warning(this,
                                     "Error",
                                     QString("Cannot have multiple matching genders. %1 repeated.")
                                             .arg(QString::fromStdString(gender_to_match))
                );
                return;
            } else if (gender_to_match == globals->everyone_gender_string()) {
                everyone_set = true;
            }

            genders_to_match_set.insert(std::move(gender_to_match));
        }
    }

    if (!one_line_edit_found || update_button == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Line edit or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    if (genders_to_match_set.empty()) {
        QMessageBox::warning(this,
                             "Error",
                             "Must enter at least one gender.\n(Type in 'Everyone' as a default to match with everyone.)"
        );
        return;
    }

    std::vector<std::string> genders_to_match;

    auto sort_genders = [](
            const std::string& lhs, const std::string& rhs
    ) -> bool {
        if (lhs == globals->male_gender_string()) {
            return true;
        } else if (rhs == globals->male_gender_string()) {
            return false;
        } else if (lhs == globals->female_gender_string()) {
            return true;
        } else if (rhs == globals->female_gender_string()) {
            return false;
        }

        return lhs < rhs;
    };

    //if 'Everyone' was set no need for any other genders
    if (everyone_set) {
        genders_to_match.clear();
        genders_to_match.emplace_back(globals->everyone_gender_string());
    } else {
        for (const auto& s: genders_to_match_set) {
            genders_to_match.emplace_back(s);
        }

        std::sort(genders_to_match.begin(), genders_to_match.end(), sort_genders);
    }

    std::sort(user_account_response->mutable_post_login_info()->mutable_gender_range()->begin(),
              user_account_response->mutable_post_login_info()->mutable_gender_range()->end(),
              sort_genders);

    if (user_account_response->post_login_info().gender_range().size()
        == (int) genders_to_match.size()) {
        bool different_gender_range = false;
        for (unsigned int i = 0; i < genders_to_match.size(); i++) {
            if (user_account_response->post_login_info().gender_range()[(int) i]
                != genders_to_match[i]) {
                different_gender_range = true;
                break;
            }
        }

        if (!different_gender_range) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Please change at least one gender in order to update gender ranges to match with."
            );
            return;
        }
    }

    update_button->setEnabled(false);

    for (auto l: line_edits) {
        l->setEnabled(false);
    }

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_gender_range.contains(current_oid)) {
        set_user_gender_range[current_oid].request_stop();
        set_user_gender_range[current_oid].join();
        set_user_gender_range.erase(current_oid);
    }

    set_user_gender_range.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, genders_to_match, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetGenderRangeRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        for (const auto& s: genders_to_match) {
                            request.add_gender_range(s);
                        }

                        grpc::Status status = set_fields_stub->SetGenderRangeRPC(&context,
                                                                                 request,
                                                                                 &response);

                        const std::string basic_error = "Set user gender ranges to match with failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userGenderRangeUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userGenderRangeUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userGenderRangeUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error + response.error_string()
                            );
                            return;
                        }

                        emit signal_userGenderRangeUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            genders_to_match
                    );
                    })
            )
    );
}

void DisplayUserWindow::slot_setUserAgeRange([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
            generatePushButtonName(AGE_RANGE_MATCHING_WITH_NAME));
    auto min_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
            generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 0));
    auto max_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
            generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 1));

    if (update_button == nullptr || min_spin_box == nullptr || max_spin_box == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Spin box or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    int min_age = min_spin_box->value();
    int max_age = max_spin_box->value();

    if (min_age == user_account_response->post_login_info().min_age()
        && max_age == user_account_response->post_login_info().max_age()
            ) {
        QMessageBox::warning(this,
                             "Error",
                             "Please change at least one age in order to update age ranges to match with."
        );
        return;
    } else if (min_age < (int) globals->lowest_allowed_age()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Minimum age must be greater than or equal to %1.")
                                     .arg(globals->lowest_allowed_age())
        );
        return;
    } else if (max_age > (int) globals->highest_allowed_age()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Maximum age must be less than or equal to %1.")
                                     .arg(globals->highest_allowed_age())
        );
        return;
    } else if (min_age > max_age) {
        QMessageBox::warning(this,
                             "Error",
                             "Minimum age must be less than or equal to maximum age."
        );
        return;
    }

    int user_age = user_account_response->birthday_info().age();

    //NOTE: this is the logic this generated document follows written out in C++
    if ((int) globals->lowest_allowed_age() <= user_age &&
        user_age <= 15) { //if age is between 13->15 the age range must be between 13->17 (13 is checked for above)
        if (17 < min_age || 17 < max_age) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Users between ages 13 and 15 cannot match with others above 17."
            );
            return;
        }
    } else if (16 == user_age || user_age ==
                                 17) { //if age is 16 or 17 then age range must be between 13->(user_age+2) (13 is checked for above)
        if (user_age + 2 < min_age
            || user_age + 2 < max_age) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Users of age 16 or 17 can only match with +/- 2 above and below their current age."
            );
            return;
        }
    } else if (18 == user_age || user_age == 19) {  //if age is 18 or 19 then age range must be between (userSge-2)-120
        if (user_age - 2 > min_age
            || user_age - 2 > max_age) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Users of age 18 or 19 can only match down to 2 years below their current age."
            );
            return;
        }
    } else if (20 <= user_age
               && user_age <= (int) globals->highest_allowed_age()
            ) { //if age is 20 or above the age range must be between 18->120 (120 is checked for above)
        if (18 > min_age) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Users aged 20 and above can only match with others 18 years or older."
            );
            return;
        }
    }

    update_button->setEnabled(false);
    min_spin_box->setEnabled(false);
    max_spin_box->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_age_range.contains(current_oid)) {
        set_user_age_range[current_oid].request_stop();
        set_user_age_range[current_oid].join();
        set_user_age_range.erase(current_oid);
    }

    set_user_age_range.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, min_age, max_age, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetAgeRangeRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_min_age(min_age);
                        request.set_max_age(max_age);

                        grpc::Status status = set_fields_stub->SetAgeRangeRPC(&context,
                                                                              request,
                                                                              &response);

                        const std::string basic_error = "Set user age range to match with failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userAgeRangeUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userAgeRangeUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userAgeRangeUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error + response.error_string()
                            );
                            return;
                        }

                        emit signal_userAgeRangeUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            min_age,
                            max_age
                    );
                    })
            )
    );
}

void DisplayUserWindow::slot_setUserMaxDistance([[maybe_unused]] bool clicked) {
    auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
            generatePushButtonName(MAX_DISTANCE_MATCHING_WITH_NAME));
    auto max_distance_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
            generateSpinBoxName(MAX_DISTANCE_MATCHING_WITH_NAME, 0));

    if (update_button == nullptr || max_distance_box == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Spin box or update button not found inside tree object.\nRestart recommended."
        );
        return;
    }

    int max_distance = max_distance_box->value();

    if (max_distance == user_account_response->post_login_info().min_age()) {
        QMessageBox::warning(this,
                             "Error",
                             "Please change max distance in order to update it."
        );
        return;
    } else if (max_distance < (int) globals->minimum_allowed_distance()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Max distance must be greater than or equal to %1.")
                                     .arg(globals->minimum_allowed_distance())
        );
        return;
    } else if (max_distance > (int) globals->maximum_allowed_distance()) {
        QMessageBox::warning(this,
                             "Error",
                             QString("Max distance must be less than or equal to %1.")
                                     .arg(globals->maximum_allowed_distance())
        );
        return;
    }

    update_button->setEnabled(false);
    max_distance_box->setEnabled(false);

    const std::string current_oid = user_account_response->account_oid();

    if (set_user_max_distance.contains(current_oid)) {
        set_user_max_distance[current_oid].request_stop();
        set_user_max_distance[current_oid].join();
        set_user_max_distance.erase(current_oid);
    }

    set_user_max_distance.insert(
            std::make_pair(
                    current_oid,
                    std::jthread([current_oid, max_distance, this](
                            const std::stop_token& stop_token
                    ) {
                        grpc::ClientContext context;
                        setfields::SetMaxDistanceRequest request;
                        setfields::SetFieldResponse response;

                        std::stop_callback stop_callback = std::stop_callback(stop_token,
                                                                              [&context]() {
                                                                                  context.TryCancel();
                                                                              });

                        std::unique_ptr<setfields::SetFieldsService::Stub> set_fields_stub =
                                setfields::SetFieldsService::NewStub(channel);
                        context.set_deadline(
                                std::chrono::system_clock::now() + GRPC_DEADLINE);

                        setup_login_info(request.mutable_login_info(), current_oid);
                        request.set_max_distance(max_distance);

                        grpc::Status status = set_fields_stub->SetMaxDistanceRPC(&context,
                                                                                 request,
                                                                                 &response);

                        const std::string basic_error = "Set max distance to match with failed.\n";
                        if (stop_token.stop_requested()) {
                            emit signal_userMaxDistanceUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::VALUE_NOT_SET,
                                    ""
                            );
                        } else if (!status.ok()) { //if grpc call failed
                            const std::string errorMessage = basic_error +
                                                             "Grpc returned status.ok() == false; code: " +
                                                             std::to_string(status.error_code()) +
                                                             " message: " +
                                                             status.error_message();

                            emit signal_userMaxDistanceUpdateCompleted(
                                    current_oid,
                                    ReturnStatus::LG_ERROR,
                                    errorMessage
                            );
                            return;
                        } else if (response.return_status() !=
                                   ReturnStatus::SUCCESS) { //if failed to log in
                            emit signal_userMaxDistanceUpdateCompleted(
                                    current_oid,
                                    response.return_status(),
                                    basic_error + response.error_string()
                            );
                            return;
                        }

                        emit signal_userMaxDistanceUpdateCompleted(
                            current_oid,
                            response.return_status(),
                            response.error_string(),
                            max_distance
                    );
                    })
            )
    );
}

void DisplayUserWindow::setupNewAccessStatus(const std::chrono::milliseconds& current_timestamp,
                                             const std::chrono::milliseconds& final_timestamp,
                                             const UserAccountStatus updated_access_status, const QString& reason,
                                             DisciplinaryActionTypeEnum action_taken, QComboBox* combo_box) {

    auto inactive_message_line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(
            generateLineEditName(INACTIVE_MESSAGE_NAME));
    auto inactive_end_time_line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(
            generateLineEditName(INACTIVE_END_TIME_NAME));

    user_account_response->set_account_status(updated_access_status);
    user_account_response->set_inactive_message(reason.toStdString());
    user_account_response->set_inactive_end_time(final_timestamp.count());

    auto action = user_account_response->add_disciplinary_actions();

    action->set_timestamp_submitted(current_timestamp.count());
    action->set_timestamp_ends(final_timestamp.count());

    action->set_type(action_taken);
    action->set_reason(reason.toStdString());
    action->set_admin_name(USER_NAME);

    if (combo_box != nullptr) {
        setActiveStatusToComboBox(combo_box);
    }

    auto inactive_message_items = ui->userInfoTreeWidget->findItems(generateLabel(INACTIVE_MESSAGE_NAME),
                                                                    Qt::MatchExactly,
                                                                    0);

    if (inactive_message_line_edit != nullptr && !inactive_message_items.empty()) {

        if (updated_access_status == STATUS_SUSPENDED
            || updated_access_status == STATUS_BANNED
                ) {

            inactive_message_line_edit->setText(reason);
            //show root item
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(inactive_message_items.front()),
                    ui->userInfoTreeWidget->rootIndex(),
                    false
            );
        } else {
            //hide root item
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(inactive_message_items.front()),
                    ui->userInfoTreeWidget->rootIndex(),
                    true
            );
        }
    }

    auto inactive_ending_time_items = ui->userInfoTreeWidget->findItems(
            generateLabel(INACTIVE_END_TIME_NAME),
            Qt::MatchExactly,
            0
    );

    if (!inactive_ending_time_items.empty()) {

        std::cout << "final_timestamp: " << final_timestamp.count() << '\n';

        if (final_timestamp.count() > 0 && inactive_end_time_line_edit != nullptr) {

            auto ending_timestamp_string = getDateTimeStringFromTimestamp(
                    std::chrono::milliseconds{user_account_response->inactive_end_time()}
            );

            inactive_end_time_line_edit->setText(QString::fromStdString(ending_timestamp_string));

            //show item
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(inactive_ending_time_items.front()),
                    ui->userInfoTreeWidget->rootIndex(),
                    false
            );

        } else {
            //hide root item
            ui->userInfoTreeWidget->setRowHidden(
                    ui->userInfoTreeWidget->indexOfTopLevelItem(inactive_ending_time_items.front()),
                    ui->userInfoTreeWidget->rootIndex(),
                    true
            );
        }
    }

    auto items = ui->userInfoTreeWidget->findItems(generateLabel(DISCIPLINARY_ACTIONS_RECORD), Qt::MatchExactly,
                                                   0);

    if (!items.empty()) {

        auto* disciplinary_actions_item = items.front();

        //show root item
        ui->userInfoTreeWidget->setRowHidden(
                ui->userInfoTreeWidget->indexOfTopLevelItem(disciplinary_actions_item),
                ui->userInfoTreeWidget->rootIndex(),
                false
        );

        displayDisciplinaryActionInItem(
                disciplinary_actions_item,
                *action,
                user_account_response->disciplinary_actions_size() - 1);
    }
}

void DisplayUserWindow::slot_userAccessStatusCompleted(
        const std::string& current_user_account_id,
        const bool successful,
        const std::string& error_string,
        const std::chrono::milliseconds& current_timestamp,
        const std::chrono::milliseconds& final_timestamp,
        const UserAccountStatus updated_access_status,
        const QString& reason
) {

    if (set_user_access_status.contains(current_user_account_id)) {
        set_user_access_status[current_user_account_id].request_stop();
        set_user_access_status[current_user_account_id].join();
        set_user_access_status.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {

        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
                generatePushButtonName(ACCESS_STATUS_NAME));
        auto combo_box = ui->userInfoTreeWidget->findChild<QComboBox*>(generateComboBoxName(ACCESS_STATUS_NAME));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (combo_box != nullptr) {
            combo_box->setEnabled(true);
        }

        if (successful) {

            DisciplinaryActionTypeEnum action_taken;

            switch (updated_access_status) {
                case STATUS_REQUIRES_MORE_INFO:
                    action_taken = DisciplinaryActionTypeEnum::DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_REQUIRES_MORE_INFO;
                    break;
                case STATUS_SUSPENDED:
                    action_taken = DisciplinaryActionTypeEnum::DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_SUSPENDED;
                    break;
                case STATUS_BANNED:
                    action_taken = DisciplinaryActionTypeEnum::DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_BANNED;
                    break;
                default:
                    action_taken = DisciplinaryActionTypeEnum::DISCIPLINE_ACCOUNT_MANUALLY_SET_TO_ACTIVE;
                    break;
            }

            setupNewAccessStatus(
                    current_timestamp,
                    final_timestamp,
                    updated_access_status,
                    reason,
                    action_taken,
                    combo_box
            );

        } else if (!error_string.empty()) { //this could mean the thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }

}


void DisplayUserWindow::slot_userNameUpdateCompleted(const std::string& current_user_account_id, ReturnStatus status,
                                                     const std::string& error_string, const std::string& updated_name) {

    if (set_user_name_threads.contains(current_user_account_id)) {
        set_user_name_threads[current_user_account_id].request_stop();
        set_user_name_threads[current_user_account_id].join();
        set_user_name_threads.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(USER_NAME_NAME));
        auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(USER_NAME_NAME));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (line_edit != nullptr) {
            line_edit->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            user_account_response->set_user_name(updated_name);

            if (line_edit != nullptr) {
                line_edit->setText(QString::fromStdString(updated_name));
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userGenderUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        const std::string& updated_gender
) {

    if (set_user_gender.contains(current_user_account_id)) {
        set_user_gender[current_user_account_id].request_stop();
        set_user_gender[current_user_account_id].join();
        set_user_gender.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(GENDER_NAME));
        auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(GENDER_NAME));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (line_edit != nullptr) {
            line_edit->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            user_account_response->set_gender(updated_gender);

            if (line_edit != nullptr) {
                line_edit->setText(QString::fromStdString(updated_gender));
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userBirthdayUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        const int updated_birth_year,
        const int updated_birth_month,
        const int updated_birth_day_of_month,
        int updated_age,
        int updated_min_age_range,
        int updated_max_age_range
) {
    if (set_user_birthday.contains(current_user_account_id)) {
        set_user_birthday[current_user_account_id].request_stop();
        set_user_birthday[current_user_account_id].join();
        set_user_birthday.erase(current_user_account_id);
    }

    if (set_user_age_range.contains(current_user_account_id)) {
        set_user_age_range[current_user_account_id].request_stop();
        set_user_age_range[current_user_account_id].join();
        set_user_age_range.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(BIRTHDAY_NAME));
        auto month_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(generateSpinBoxName(BIRTHDAY_NAME, 0));
        auto day_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(generateSpinBoxName(BIRTHDAY_NAME, 1));
        auto year_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(generateSpinBoxName(BIRTHDAY_NAME, 2));

        auto age_range_update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
                generatePushButtonName(AGE_RANGE_MATCHING_WITH_NAME));
        auto age_range_min_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
                generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 0));
        auto age_range_max_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
                generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 1));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (month_spin_box != nullptr) {
            month_spin_box->setEnabled(true);
        }

        if (day_spin_box != nullptr) {
            day_spin_box->setEnabled(true);
        }

        if (year_spin_box != nullptr) {
            year_spin_box->setEnabled(true);
        }

        if (age_range_update_button != nullptr) {
            age_range_update_button->setEnabled(true);
        }

        if (age_range_min_spin_box != nullptr) {
            age_range_min_spin_box->setEnabled(true);
        }

        if (age_range_max_spin_box != nullptr) {
            age_range_max_spin_box->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            auto age_line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(AGE_NAME));

            if (age_line_edit != nullptr) {
                age_line_edit->setText(QString::number(updated_age));
            }

            user_account_response->mutable_birthday_info()->set_birth_year(updated_birth_year);
            user_account_response->mutable_birthday_info()->set_birth_month(updated_birth_month);
            user_account_response->mutable_birthday_info()->set_birth_day_of_month(updated_birth_day_of_month);
            user_account_response->mutable_birthday_info()->set_age(updated_age);

            if (month_spin_box != nullptr) {
                month_spin_box->setValue(updated_birth_month);
            }

            if (day_spin_box != nullptr) {
                day_spin_box->setValue(updated_birth_day_of_month);
            }

            if (year_spin_box != nullptr) {
                year_spin_box->setValue(updated_birth_year);
            }

            if (age_range_min_spin_box != nullptr) {
                age_range_min_spin_box->setValue(updated_min_age_range);
            }

            if (age_range_max_spin_box != nullptr) {
                age_range_max_spin_box->setValue(updated_max_age_range);
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userEmailUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        const std::string& updated_email
) {

    if (set_user_email.contains(current_user_account_id)) {
        set_user_email[current_user_account_id].request_stop();
        set_user_email[current_user_account_id].join();
        set_user_email.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
                generatePushButtonName(EMAIL_ADDRESS_NAME));
        auto email_line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(EMAIL_ADDRESS_NAME));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (email_line_edit != nullptr) {
            email_line_edit->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {
            auto email_verification_line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(
                    generateLineEditName(EMAIL_ADDRESS_VERIFIED_NAME));

            user_account_response->mutable_email_info()->set_email(updated_email);
            user_account_response->mutable_email_info()->set_requires_email_verification(true);

            if (email_verification_line_edit != nullptr) {

                QString email_verified_string;

                if (user_account_response->email_info().requires_email_verification()) {
                    email_verified_string = "False";
                } else {
                    email_verified_string = "True";
                }

                email_verification_line_edit->setText(email_verified_string);
            }

            if (email_line_edit != nullptr) {
                email_line_edit->setText(QString::fromStdString(updated_email));
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userBioUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        const std::string& updated_bio
) {

    if (set_user_bio.contains(current_user_account_id)) {
        set_user_bio[current_user_account_id].request_stop();
        set_user_bio[current_user_account_id].join();
        set_user_bio.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(BIO_NAME));
        auto plain_text_edit = ui->userInfoTreeWidget->findChild<QPlainTextEdit*>(generatePlainTextEditName(BIO_NAME));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (plain_text_edit != nullptr) {
            plain_text_edit->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            user_account_response->mutable_post_login_info()->set_user_bio(updated_bio);

            if (plain_text_edit != nullptr) {

                QString bio_text;
                if (!updated_bio.empty() && updated_bio != "~") {
                    bio_text = QString::fromStdString(updated_bio);
                }

                plain_text_edit->setPlainText(bio_text);
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userCityUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        const std::string& updated_city
) {

    if (set_user_city.contains(current_user_account_id)) {
        set_user_city[current_user_account_id].request_stop();
        set_user_city[current_user_account_id].join();
        set_user_city.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(generatePushButtonName(CITY_NAME));
        auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(generateLineEditName(CITY_NAME));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (line_edit != nullptr) {
            line_edit->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            user_account_response->mutable_post_login_info()->set_user_city(updated_city);

            if (line_edit != nullptr) {

                QString city_text;
                if (!updated_city.empty() && updated_city != "~") {
                    city_text = QString::fromStdString(updated_city);
                }

                line_edit->setText(city_text);
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userGenderRangeUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        const std::vector<std::string>& gender_ranges
) {
    if (set_user_gender_range.contains(current_user_account_id)) {
        set_user_gender_range[current_user_account_id].request_stop();
        set_user_gender_range[current_user_account_id].join();
        set_user_gender_range.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
                generatePushButtonName(GENDERS_MATCHING_WITH_NAME));
        std::vector<QLineEdit*> line_edits;
        for (unsigned int i = 0; i < globals->number_genders_can_match_with(); i++) {
            auto line_edit = ui->userInfoTreeWidget->findChild<QLineEdit*>(
                    generateIndexedLineEditName(GENDERS_MATCHING_WITH_NAME, i));

            if (line_edit != nullptr) {
                line_edits.emplace_back(line_edit);
                line_edit->setEnabled(true);
            }
        }

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            user_account_response->mutable_post_login_info()->mutable_gender_range()->Clear();

            for (const auto& g: gender_ranges) {
                user_account_response->mutable_post_login_info()->add_gender_range(g);
            }

            for (unsigned int i = 0; i < line_edits.size(); i++) {
                if (i < gender_ranges.size()) {
                    line_edits[i]->setText(QString::fromStdString(gender_ranges[i]));
                } else {
                    line_edits[i]->setText("");
                }
            }


        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userAgeRangeUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        const int updated_min_age,
        const int updated_max_age
) {
    if (set_user_age_range.contains(current_user_account_id)) {
        set_user_age_range[current_user_account_id].request_stop();
        set_user_age_range[current_user_account_id].join();
        set_user_age_range.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
                generatePushButtonName(AGE_RANGE_MATCHING_WITH_NAME));
        auto min_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
                generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 0));
        auto max_spin_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
                generateSpinBoxName(AGE_RANGE_MATCHING_WITH_NAME, 1));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (min_spin_box != nullptr) {
            min_spin_box->setEnabled(true);
        }

        if (max_spin_box != nullptr) {
            max_spin_box->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            user_account_response->mutable_post_login_info()->set_min_age(updated_min_age);
            user_account_response->mutable_post_login_info()->set_max_age(updated_max_age);

            if (min_spin_box != nullptr) {
                min_spin_box->setValue(updated_min_age);
            }

            if (max_spin_box != nullptr) {
                max_spin_box->setValue(updated_max_age);
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(this,
                                 "Error",
                                 QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_userMaxDistanceUpdateCompleted(
        const std::string& current_user_account_id,
        ReturnStatus status,
        const std::string& error_string,
        int updated_max_distance
) {
    if (set_user_max_distance.contains(current_user_account_id)) {
        set_user_max_distance[current_user_account_id].request_stop();
        set_user_max_distance[current_user_account_id].join();
        set_user_max_distance.erase(current_user_account_id);
    }

    if (current_user_account_id == user_account_response->account_oid()) {
        auto update_button = ui->userInfoTreeWidget->findChild<QPushButton*>(
                generatePushButtonName(MAX_DISTANCE_MATCHING_WITH_NAME));
        auto max_distance_box = ui->userInfoTreeWidget->findChild<QSpinBox*>(
                generateSpinBoxName(MAX_DISTANCE_MATCHING_WITH_NAME, 0));

        if (update_button != nullptr) {
            update_button->setEnabled(true);
        }

        if (max_distance_box != nullptr) {
            max_distance_box->setEnabled(true);
        }

        if (status == ReturnStatus::SUCCESS) {

            user_account_response->mutable_post_login_info()->set_max_distance(updated_max_distance);

            if (max_distance_box != nullptr) {
                max_distance_box->setValue(updated_max_distance);
            }

        } else if (status != ReturnStatus::VALUE_NOT_SET &&
                   !error_string.empty()) { //VALUE_NOT_SET means thread was canceled
            QMessageBox::warning(
                    this,
                    "Error",
                    QString::fromStdString(error_string)
            );
            return;
        }
    }
}

void DisplayUserWindow::slot_removePicture(QWidget* w, const std::string& picture_oid[[maybe_unused]]) {
    ui->picturesHorizontalLayout->removeWidget(w);
    delete w;
}

void DisplayUserWindow::on_timeOutPushButton_clicked() {

    if (user_account_response->account_oid().empty()
        || user_account_response->account_oid() == "~") {
        QMessageBox::warning(
                this,
                "Error",
                "User not set up."
        );
        return;
    }

    if (user_account_response->account_status() == UserAccountStatus::STATUS_REQUIRES_MORE_INFO
        || user_account_response->account_status() == UserAccountStatus::STATUS_SUSPENDED
        || user_account_response->account_status() == UserAccountStatus::STATUS_BANNED
            ) {

        QString access_status_string;

        switch (user_account_response->account_status()) {
            case STATUS_ACTIVE:
                access_status_string = ACCOUNT_STATUS_ACTIVE;
                break;
            case STATUS_REQUIRES_MORE_INFO:
                access_status_string = ACCOUNT_STATUS_REQUIRES_INFO;
                break;
            case STATUS_SUSPENDED:
                access_status_string = ACCOUNT_STATUS_SUSPENDED;
                break;
            case STATUS_BANNED:
                access_status_string = ACCOUNT_STATUS_BANNED;
                break;
            default:
                access_status_string =
                        "ERROR_UNKNOWN_VALUE: " + QString::number(user_account_response->account_status());
                break;
        }

        QMessageBox::warning(
                this,
                "Error",
                QString("Can not time out a user with access status of '%1'.")
                        .arg(access_status_string)
        );
        return;
    }

    //NOTE: banned here is simply to get the correct dialog to show, it does not mean the
    // user will be banned
    AccessStatusInvalidMessageDialog dialog(UserAccountStatus::STATUS_BANNED);
    auto return_status = AccessStatusInvalidMessageDialog::ReturnStatusForDialog(dialog.exec());
    std::string reason;

    switch (return_status) {
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_OK:
            reason = dialog.getReasonString().toStdString();
            break;
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_CANCEL:
            return;
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_ERROR:
            QMessageBox::warning(
                    this,
                    "Error",
                    dialog.getErrorString()
            );
            return;
    }

    if (reason.size() < globals->minimum_number_allowed_bytes_inactive_message()) {
        QMessageBox::warning(
                this,
                "Error",
                "Inactive reason too short, " +
                QString::number(globals->minimum_number_allowed_bytes_inactive_message()) +
                " characters required."
        );
        return;
    }

    ui->timeOutPushButton->setEnabled(false);
    ui->dismissReportPushButton->setEnabled(false);

    time_out_user.timeOutUser(
            user_account_response->account_oid(),
            reason,
            [&]() {
                ui->timeOutPushButton->setEnabled(true);
                ui->dismissReportPushButton->setEnabled(true);
            });

}

void DisplayUserWindow::on_dismissReportPushButton_clicked() {

    if (user_account_response->account_oid().empty()
        || user_account_response->account_oid() == "~") {
        QMessageBox::warning(
                this,
                "Error",
                "User not set up."
        );
        return;
    }

    ui->timeOutPushButton->setEnabled(false);
    ui->dismissReportPushButton->setEnabled(false);

    dismiss_report.dismissReport(
            user_account_response->account_oid(),
            [&]() {
                ui->timeOutPushButton->setEnabled(true);
                ui->dismissReportPushButton->setEnabled(true);
            });

}

void DisplayUserWindow::slot_treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column) {
    auto data = item->data(column, Qt::UserRole);

    if (!data.isNull()) {
        QJsonObject doc = data.toJsonObject();
        const TypeOfDoubleClick type_of_double_click = TypeOfDoubleClick(
                doc.value(DOUBLE_CLICK_JSON_ENUM_KEY).toString().toInt());

        switch (type_of_double_click) {
            case TypeOfDoubleClick::USER_CREATED_EVENT: {
                const std::string event_oid = doc.value(DOUBLE_CLICK_JSON_OID_KEY).toString().toStdString();
                emit signal_findEvent(event_oid);
                break;
            }
        }
    }
}