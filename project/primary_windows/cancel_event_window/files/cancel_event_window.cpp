//
// Created by jeremiah on 3/26/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_cancel_event_window.h" resolved

#include <QMessageBox>
#include <QJsonObject>
#include <iomanip>
#include "cancel_event_window.h"
#include "ui_cancel_event_window.h"
#include "homewindow.h"
#include "general_utility.h"

CancelEventWindow::CancelEventWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::CancelEventWindow) {
    ui->setupUi(this);

    connectInitialSignalsAndSlots();

    setupInitialUIState();
}

void CancelEventWindow::connectInitialSignalsAndSlots() {
    connect(&get_events_object, &GetEventsObject::signal_sendWarning, this,
            &CancelEventWindow::slot_displayWarning);
    connect(&get_events_object, &GetEventsObject::signal_requestCanceled, this,
            &CancelEventWindow::slot_requestCanceled);
    connect(&get_events_object, &GetEventsObject::signal_getEventsSuccessfullyCompleted, this,
            &CancelEventWindow::slot_getEventsSuccessfullyCompleted);

    connect(&cancel_event_object, &CancelEventObject::signal_sendWarning, this,
            &CancelEventWindow::slot_displayWarning);
    connect(&cancel_event_object, &CancelEventObject::signal_requestCanceled, this,
            &CancelEventWindow::slot_requestCanceled);
    connect(&cancel_event_object, &CancelEventObject::signal_cancelEventSuccessfullyCompleted, this,
            &CancelEventWindow::slot_cancelEventSuccessfullyCompleted);

    connect(ui->displayEventsTreeWidget, &QTreeWidget::itemDoubleClicked, this,
            &CancelEventWindow::slot_treeWidgetItemDoubleClicked);
}

void CancelEventWindow::setupInitialUIState() {
    ui->eventTypeSelectionLineEdit->setVisible(false);
    ui->createdTimeDateTimeEdit->setVisible(false);
    ui->expirationTimeDateTimeEdit->setVisible(false);

    ui->displayEventsTreeWidget->header()->setStretchLastSection(false);
    ui->displayEventsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QDateTime current_date_time = QDateTime::currentDateTime();

    ui->createdTimeDateTimeEdit->setDateTime(current_date_time);
    ui->expirationTimeDateTimeEdit->setDateTime(current_date_time);
}

CancelEventWindow::~CancelEventWindow() {
    delete ui;
}

void CancelEventWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

void CancelEventWindow::on_cancelEventPushButton_clicked() {
    QTreeWidgetItem* selected_item = ui->displayEventsTreeWidget->currentItem();

    if (!selected_item) {
        slot_displayWarning(
                "Error",
                "Please select an event to be canceled."
        );
        return;
    }

    if (LetsGoEventStatus_Name(LetsGoEventStatus::ONGOING)
        != selected_item->text(EVENT_STATUS_COLUMN_NUM).toStdString()) {
        slot_displayWarning(
                "Error",
                QString("Event must have a Status of '")
                        .append(QString::fromStdString(LetsGoEventStatus_Name(LetsGoEventStatus::ONGOING)))
                        .append("' in order to be canceled.")
        );
        return;
    }

    auto data = selected_item->data(EVENT_TITLE_COLUMN_NUM, Qt::UserRole);
    if (data.isNull()) {
        slot_displayWarning(
                "Error",
                "Internal issue data.isNull() found when attempting to retrieve event ID for cancellation. "
                "Please restart the program if problem persists."
        );
        return;
    }

    const std::string event_title = selected_item->text(EVENT_TITLE_COLUMN_NUM).toStdString();

    QJsonObject doc = data.toJsonObject();
    const std::string event_oid = doc.value(DOUBLE_CLICK_JSON_STRING_KEY).toString().toStdString();

    const QString title = "Warning";
    const QString text = QString("Do you want to cancel event '")
            .append(QString::fromStdString(event_title))
            .append("'? This action cannot be undone.");

    const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            title,
            text,
            QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::No) {
        return;
    }

    ui->cancelEventPushButton->setEnabled(false);
    cancel_event_object.runCancelEvent(
            event_oid,
            event_title,
            [this]() {
                ui->cancelEventPushButton->setEnabled(true);
            }
    );
}

void CancelEventWindow::on_searchPushButton_clicked() {

    admin_event_commands::GetEventsRequest get_events_request;

    if (ui->allEventsRadioButton->isChecked()) {
        get_events_request.mutable_account_types_to_search()->add_events_to_search_by(
                UserAccountType::ADMIN_GENERATED_EVENT_TYPE);
        get_events_request.mutable_account_types_to_search()->add_events_to_search_by(
                UserAccountType::USER_GENERATED_EVENT_TYPE);
    } else if (ui->letsGoEventsOnlyRadioButton->isChecked()) {
        get_events_request.mutable_account_types_to_search()->add_events_to_search_by(
                UserAccountType::ADMIN_GENERATED_EVENT_TYPE);
    } else if (ui->userEventsOnlyRadioButton->isChecked()) {
        get_events_request.mutable_account_types_to_search()->add_events_to_search_by(
                UserAccountType::USER_GENERATED_EVENT_TYPE);
    } else if (ui->specificUserRadioButton->isChecked()) {
        std::string user_name_or_id = ui->eventTypeSelectionLineEdit->text().toStdString();
        trimWhitespace(user_name_or_id);
        if (user_name_or_id.empty()) {
            slot_displayWarning(
                    "Error",
                    "When 'Specific User' is selected, a valid user ID or admin name must be entered."
            );
            return;
        }
        get_events_request.set_single_user_created_by(std::move(user_name_or_id));
    } else {
        slot_displayWarning(
                "Error",
                "Could not detect what type of events to search for. Please restart the program if problem persists."
        );
        return;
    }

    if (ui->anyCreatedTimeRadioButton->isChecked()) {
        get_events_request.set_start_time_search(admin_event_commands::GetEventsRequest_TimeSearch_NO_SEARCH);
    } else if (ui->greaterThanCreatedTimeRadioButton->isChecked()) {
        get_events_request.set_start_time_search(admin_event_commands::GetEventsRequest_TimeSearch_GREATER_THAN);
        get_events_request.set_events_start_time_ms(ui->createdTimeDateTimeEdit->dateTime().toMSecsSinceEpoch());
    } else if (ui->lessThanCreatedTimeRadioButton->isChecked()) {
        get_events_request.set_start_time_search(admin_event_commands::GetEventsRequest_TimeSearch_LESS_THAN);
        get_events_request.set_events_start_time_ms(ui->createdTimeDateTimeEdit->dateTime().toMSecsSinceEpoch());
    } else {
        slot_displayWarning(
                "Error",
                "Could not detect parameters of created time to search for. Please restart the program if problem persists."
        );
        return;
    }

    if (ui->anyExpirationTimeRadioButton->isChecked()) {
        get_events_request.set_expiration_time_search(admin_event_commands::GetEventsRequest_TimeSearch_NO_SEARCH);
    } else if (ui->greaterThanExpirationTimeRadioButton->isChecked()) {
        get_events_request.set_expiration_time_search(admin_event_commands::GetEventsRequest_TimeSearch_GREATER_THAN);
        get_events_request.set_events_expiration_time_ms(
                ui->expirationTimeDateTimeEdit->dateTime().toMSecsSinceEpoch());
    } else if (ui->lessThanExpirationTimeRadioButton->isChecked()) {
        get_events_request.set_expiration_time_search(admin_event_commands::GetEventsRequest_TimeSearch_LESS_THAN);
        get_events_request.set_events_expiration_time_ms(
                ui->expirationTimeDateTimeEdit->dateTime().toMSecsSinceEpoch());
    } else {
        slot_displayWarning(
                "Error",
                "Could not detect parameters of expiration time to search for. Please restart the program if problem persists."
        );
        return;
    }

    ui->searchPushButton->setEnabled(false);
    get_events_object.runGetEvents(
            std::move(get_events_request),
            [this]() {
                ui->searchPushButton->setEnabled(true);
            }
    );
}

void CancelEventWindow::setEventTypeSelectionLineEditVisible() {
    ui->eventTypeSelectionLineEdit->setVisible(ui->specificUserRadioButton->isChecked());
}

void CancelEventWindow::setCreatedTimeDateTimeEditVisible() {
    ui->createdTimeDateTimeEdit->setVisible(!ui->anyCreatedTimeRadioButton->isChecked());
}

void CancelEventWindow::setExpirationTimeDateTimeEditVisible() {
    ui->expirationTimeDateTimeEdit->setVisible(!ui->anyExpirationTimeRadioButton->isChecked());
}

void CancelEventWindow::on_allEventsRadioButton_toggled(bool checked [[maybe_unused]]) {
    setEventTypeSelectionLineEditVisible();
}

void CancelEventWindow::on_letsGoEventsOnlyRadioButton_toggled(bool checked [[maybe_unused]]) {
    setEventTypeSelectionLineEditVisible();
}

void CancelEventWindow::on_userEventsOnlyRadioButton_toggled(bool checked [[maybe_unused]]) {
    setEventTypeSelectionLineEditVisible();
}

void CancelEventWindow::on_specificUserRadioButton_toggled(bool checked [[maybe_unused]]) {
    setEventTypeSelectionLineEditVisible();
}

void CancelEventWindow::on_anyCreatedTimeRadioButton_toggled(bool checked [[maybe_unused]]) {
    setCreatedTimeDateTimeEditVisible();
}

void CancelEventWindow::on_greaterThanCreatedTimeRadioButton_toggled(bool checked [[maybe_unused]]) {
    setCreatedTimeDateTimeEditVisible();
}

void CancelEventWindow::on_lessThanCreatedTimeRadioButton_toggled(bool checked [[maybe_unused]]) {
    setCreatedTimeDateTimeEditVisible();
}

void CancelEventWindow::on_anyExpirationTimeRadioButton_toggled(bool checked [[maybe_unused]]) {
    setExpirationTimeDateTimeEditVisible();
}

void CancelEventWindow::on_greaterThanExpirationTimeRadioButton_toggled(bool checked [[maybe_unused]]) {
    setExpirationTimeDateTimeEditVisible();
}

void CancelEventWindow::on_lessThanExpirationTimeRadioButton_toggled(bool checked [[maybe_unused]]) {
    setExpirationTimeDateTimeEditVisible();
}

void CancelEventWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void CancelEventWindow::slot_displayWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& lambda
) {
    if (lambda) {
        lambda();
    }

    std::cout << "title: " << title.toStdString() << '\n';
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(
            this,
            title,
            text
    );
}

void CancelEventWindow::slot_getEventsSuccessfullyCompleted(
        const admin_event_commands::GetEventsResponse& response,
        const std::function<void()>& completed_lambda
) {
    if (completed_lambda) {
        completed_lambda();
    }

    ui->displayEventsTreeWidget->clear();

    for (const auto& event: response.events()) {
        auto* event_item = new QTreeWidgetItem();

        event_item->setText(EVENT_TITLE_COLUMN_NUM, QString::fromStdString(event.event_title()));
        event_item->setTextAlignment(EVENT_TITLE_COLUMN_NUM, Qt::AlignHCenter);

        QJsonObject event_title_doc;
        event_title_doc.insert(DOUBLE_CLICK_JSON_ENUM_KEY, QString::number((int) TypeOfDoubleClick::OPEN_EVENT));
        event_title_doc.insert(DOUBLE_CLICK_JSON_STRING_KEY, QString::fromStdString(event.event_oid()));

        event_item->setData(EVENT_TITLE_COLUMN_NUM, Qt::UserRole, event_title_doc);

        event_item->setText(CREATED_BY_COLUMN_NUM, QString::fromStdString(event.created_by()));
        event_item->setTextAlignment(CREATED_BY_COLUMN_NUM, Qt::AlignHCenter);

        if (isValidOid(event.created_by()) == "") {
            QJsonObject created_by_doc;
            created_by_doc.insert(DOUBLE_CLICK_JSON_ENUM_KEY, QString::number((int) TypeOfDoubleClick::OPEN_USER));
            created_by_doc.insert(DOUBLE_CLICK_JSON_STRING_KEY, QString::fromStdString(event.created_by()));

            event_item->setData(CREATED_BY_COLUMN_NUM, Qt::UserRole, created_by_doc);
        }

        event_item->setText(CHAT_ROOM_ID_COLUMN_NUM, QString::fromStdString(event.chat_room_id()));
        event_item->setTextAlignment(CHAT_ROOM_ID_COLUMN_NUM, Qt::AlignHCenter);

        event_item->setText(
                EVENT_TYPE_COLUMN_NUM,
                QString::fromStdString(
                        event.account_type() == UserAccountType::ADMIN_GENERATED_EVENT_TYPE
                        ? "Admin Event"
                        : "User Event"
                )
        );
        event_item->setTextAlignment(EVENT_TYPE_COLUMN_NUM, Qt::AlignHCenter);

        event_item->setText(
                TIME_CREATED_COLUMN_NUM,
                QString::fromStdString(
                        getDateTimeStringFromTimestamp(
                                std::chrono::milliseconds{
                                        event.time_created_ms()
                                }
                        )
                )
        );
        event_item->setTextAlignment(TIME_CREATED_COLUMN_NUM, Qt::AlignHCenter);

        if (event.expiration_time_ms() > 0) {
            event_item->setText(
                    EXPIRATION_TIME_COLUMN_NUM,
                    QString::fromStdString(
                            getDateTimeStringFromTimestamp(
                                    std::chrono::milliseconds{
                                            event.expiration_time_ms()
                                    }
                            )
                    )
            );
        } else {
            event_item->setText(
                    EXPIRATION_TIME_COLUMN_NUM,
                    NOT_ONGOING_EXPIRATION_TIME_PLACEHOLDER
            );
        }

        event_item->setTextAlignment(EXPIRATION_TIME_COLUMN_NUM, Qt::AlignHCenter);

        event_item->setText(
                EVENT_STATUS_COLUMN_NUM,
                QString::fromStdString(
                        LetsGoEventStatus_Name(event.event_status())
                )
        );
        event_item->setTextAlignment(EVENT_STATUS_COLUMN_NUM, Qt::AlignHCenter);

        event_item->setText(
                NUMBER_SWIPED_YES_COLUMN_NUM,
                QString::number(event.number_swipes_yes())
        );
        event_item->setTextAlignment(NUMBER_SWIPED_YES_COLUMN_NUM, Qt::AlignHCenter);

        const int denominator =
                event.number_swipes_yes() + event.number_swipes_no() + event.number_swipes_block_and_report();
        const float percentage_yes_swipes = denominator != 0
                                            ? 100.0f * (float) event.number_swipes_yes() / (float) denominator
                                            : 0.0f;

        event_item->setText(
                PERCENTAGE_SWIPED_YES_COLUMN_NUM,
                QString::number(percentage_yes_swipes, 'f', 2)
        );
        event_item->setTextAlignment(PERCENTAGE_SWIPED_YES_COLUMN_NUM, Qt::AlignHCenter);

        ui->displayEventsTreeWidget->addTopLevelItem(event_item);
    }

}

void CancelEventWindow::slot_cancelEventSuccessfullyCompleted(
        const std::string& event_oid,
        const std::string& event_title,
        const std::function<void()>& completed_lambda
) {
    if (completed_lambda) {
        completed_lambda();
    }

    const int row_count = ui->displayEventsTreeWidget->topLevelItemCount();

    for (int i = 0; i < row_count; ++i) {
        QTreeWidgetItem* item = ui->displayEventsTreeWidget->topLevelItem(i);

        if (!item) {
            continue;
        }

        auto data = item->data(EVENT_TITLE_COLUMN_NUM, Qt::UserRole);
        if (!data.isNull()) {
            QJsonObject doc = data.toJsonObject();
            const std::string extracted_event_oid = doc.value(DOUBLE_CLICK_JSON_STRING_KEY).toString().toStdString();

            if (event_oid == extracted_event_oid) {
                item->setText(
                        EVENT_STATUS_COLUMN_NUM,
                        QString::fromStdString(
                                LetsGoEventStatus_Name(LetsGoEventStatus::CANCELED)
                        )
                );

                item->setText(
                        EXPIRATION_TIME_COLUMN_NUM,
                        NOT_ONGOING_EXPIRATION_TIME_PLACEHOLDER
                );
                break;
            }
        }
    }

    const QString title = "Canceled";
    const QString text = QString("Successfully canceled '")
            .append(QString::fromStdString(event_title))
            .append("' event.");

    //Keep this information box after displayEventsTreeWidget is updated. Otherwise, the user will have an information
    // box popped up while the update has not yet occurred.
    QMessageBox::information(
            this,
            title,
            text
    );

}

void CancelEventWindow::slot_treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column) {
    auto data = item->data(column, Qt::UserRole);

    if (!data.isNull()) {
        QJsonObject doc = data.toJsonObject();
        const TypeOfDoubleClick type_of_double_click = TypeOfDoubleClick(
                doc.value(DOUBLE_CLICK_JSON_ENUM_KEY).toString().toInt());

        switch (type_of_double_click) {
            case TypeOfDoubleClick::OPEN_EVENT: {
                const std::string event_oid = doc.value(DOUBLE_CLICK_JSON_STRING_KEY).toString().toStdString();
                home_window_handle->findEvent(event_oid);
                break;
            }
            case TypeOfDoubleClick::OPEN_USER: {
                const std::string user_oid = doc.value(DOUBLE_CLICK_JSON_STRING_KEY).toString().toStdString();
                home_window_handle->findUser(user_oid);
            }
        }
    }
}
