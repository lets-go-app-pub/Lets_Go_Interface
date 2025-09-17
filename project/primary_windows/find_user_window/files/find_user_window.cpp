//
// Created by jeremiah on 9/10/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_find_user_window.h" resolved

#include <QMessageBox>

#include "general_utility.h"
#include "find_user_window.h"
#include "ui_find_user_window.h"
#include "homewindow.h"

FindUserWindow::FindUserWindow(bool event_window, QWidget* parent) :
        QWidget(parent), ui(new Ui::FindUserWindow), event_window(event_window) {
    ui->setupUi(this);
    initializeDisplayUserWindow();

    if (event_window) {
        ui->phoneNumberRadioButton->setChecked(false);
        ui->userIDRadioButton->setChecked(true);

        ui->selectSearchMethodFrame->setVisible(false);
        ui->typeSearchInfoTitleLabel->setText("Enter Event ID");
    }
}

void FindUserWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

FindUserWindow::~FindUserWindow() {
    delete ui;
}

void FindUserWindow::searchByOid(const std::string& account_oid) {
    ui->phoneNumberRadioButton->setChecked(false);
    ui->userIDRadioButton->setChecked(true);

    ui->typeSearchInfoTitleLineEdit->setText(QString::fromStdString(account_oid));
    on_searchPushButton_clicked();
}

void FindUserWindow::initializeDisplayUserWindow() {
    if (display_user_window == nullptr) {
        display_user_window = new DisplayUserWindow(false);

        connect(display_user_window, &DisplayUserWindow::signal_sendWarning, this,
                &FindUserWindow::slot_displayWarning);
        connect(display_user_window, &DisplayUserWindow::signal_requestCanceled, this,
                &FindUserWindow::slot_requestCanceled);
        connect(display_user_window, &DisplayUserWindow::signal_requestSuccessfullyCompleted, this,
                &FindUserWindow::slot_requestSuccessfullyCompleted);
        connect(display_user_window, &DisplayUserWindow::signal_findEvent, this,
                &FindUserWindow::slot_findEvent);
        ui->scrollAreaWidgetContents->layout()->addWidget(display_user_window);
    }
}

void FindUserWindow::slot_displayWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& lambda
) {

    lambda();
    std::cout << "title: " << title.toStdString() << '\n';
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(
            this,
            title,
            text
    );
}

void FindUserWindow::slot_findEvent(const std::string& event_oid) {
    home_window_handle->findEvent(event_oid);
}

void FindUserWindow::slot_requestSuccessfullyCompleted(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void FindUserWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void FindUserWindow::on_searchPushButton_clicked() {

    if (ui->userIDRadioButton->isChecked()) {
        std::string oid_string = ui->typeSearchInfoTitleLineEdit->text().toStdString();

        QString oid_error_string = isValidOid(oid_string);

        if (!oid_error_string.isEmpty()) {
            QMessageBox::warning(this,
                                 "Error",
                                 oid_error_string
            );
            return;
        }

        BsoncxxOIDWrapper oid(oid_string);

        ui->searchPushButton->setEnabled(false);
        display_user_window->findUser(
                oid,
                event_window,
                [this]() {
                    ui->searchPushButton->setEnabled(true);
                });

    } else if (ui->phoneNumberRadioButton->isChecked()) {
        std::string phone_number_string = ui->typeSearchInfoTitleLineEdit->text().toStdString();

        //should strip phone number of non-digit values
        phone_number_string.erase(
                std::remove_if(
                        phone_number_string.begin(),
                        phone_number_string.end(),
                        [](char c) -> bool {
                            return !std::isdigit(c);
                        }
                ),
                phone_number_string.end()
        );

        if (phone_number_string.size() != 10
            || phone_number_string[0] == '0' //area codes cannot start with 0 or 1
            || phone_number_string[0] == '1'
                ) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Invalid phone number entered."
            );
            return;
        }

        phone_number_string.insert(0, "+1");

        ui->searchPushButton->setEnabled(false);
        display_user_window->findUser(
                phone_number_string,
                [this]() {
                    ui->searchPushButton->setEnabled(true);
                });
    } else {
        QMessageBox::warning(this,
                             "Error",
                             "Neither phone number or ID radio button was selected."
        );
    }
}

void FindUserWindow::on_userIDRadioButton_toggled(bool checked) {
    if (checked) {
        ui->typeSearchInfoTitleLabel->setText("Enter User ID");
    }
}

void FindUserWindow::on_phoneNumberRadioButton_toggled(bool checked) {
    if (checked) {
        ui->typeSearchInfoTitleLabel->setText("XXX XXX-XXXX");
    }
}

