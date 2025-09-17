//
// Created by jeremiah on 9/13/21.
//

#include <plain_text_edit_with_max_chars.h>
#include <user_login_info.h>
#include "access_status_invalid_message_dialog.h"
#include "ui_access_status_invalid_message_dialog.h"

AccessStatusInvalidMessageDialog::AccessStatusInvalidMessageDialog(UserAccountStatus status, QWidget* parent) :
        QDialog(parent), ui(new Ui::AccessStatusInvalidMessageDialog) {
    ui->setupUi(this);
    user_account_status = status;

    switch (status) {

        case STATUS_ACTIVE:
        case STATUS_REQUIRES_MORE_INFO: {
            ui->titleMoveReasonLabel->setVisible(false);
            ui->titleNoteLabel->setVisible(false);
            ui->titleNoteLabel->setVisible(false);
            ui->languageRadioButton->setVisible(false);
            ui->pictureRadioButton->setVisible(false);
            ui->otherRadioButton->setVisible(false);
            ui->reasonTitleLabel->setVisible(true);

            auto text_edit = new PlainTextEditWithMaxChars();
            text_edit->setMaxChar((int) globals->maximum_number_allowed_bytes_inactive_message());
            text_edit->setObjectName(TEXT_EDIT_NAME);

            ui->otherTextEditVerticalWidget->layout()->addWidget(text_edit);

            ui->durationHorizontalWidget->setVisible(false);

            break;
        }
        case STATUS_BANNED: {

            ui->titleMoveReasonLabel->setVisible(false);
            ui->titleNoteLabel->setVisible(true);
            ui->languageRadioButton->setVisible(true);
            ui->pictureRadioButton->setVisible(true);
            ui->otherRadioButton->setVisible(true);
            ui->reasonTitleLabel->setVisible(false);

            auto text_edit = new PlainTextEditWithMaxChars();
            text_edit->setMaxChar((int) globals->maximum_number_allowed_bytes_inactive_message());
            text_edit->setObjectName(TEXT_EDIT_NAME);

            ui->otherTextEditVerticalWidget->layout()->addWidget(text_edit);

            ui->durationHorizontalWidget->setVisible(false);

            break;
        }
        case STATUS_SUSPENDED: {

            ui->titleMoveReasonLabel->setVisible(false);
            ui->titleNoteLabel->setVisible(true);
            ui->languageRadioButton->setVisible(true);
            ui->pictureRadioButton->setVisible(true);
            ui->otherRadioButton->setVisible(true);
            ui->reasonTitleLabel->setVisible(false);

            auto text_edit = new PlainTextEditWithMaxChars();
            text_edit->setMaxChar((int) globals->maximum_number_allowed_bytes_inactive_message());
            text_edit->setObjectName(TEXT_EDIT_NAME);

            ui->otherTextEditVerticalWidget->layout()->addWidget(text_edit);

            ui->durationHorizontalWidget->setVisible(true);

            break;
        }
        default:
            ui->titleLabel->setText(
                    QString("An error occurred invalid user account status of '%1' passed.")
                            .arg(QString::fromStdString(UserAccountStatus_Name(status)))
            );
            ui->primaryHorizontalWidget->setVisible(false);
            ui->okPushButton->setVisible(false);
            return;
    }
}

AccessStatusInvalidMessageDialog::AccessStatusInvalidMessageDialog(DeleteInputForDialog _delete_input, QWidget* parent)
        : QDialog(parent), ui(new Ui::AccessStatusInvalidMessageDialog) {
    ui->setupUi(this);
    delete_input = _delete_input;

    switch (delete_input) {
        case DELETE_INPUT_SINGLE: {

            ui->titleMoveReasonLabel->setVisible(false);
            ui->titleNoteLabel->setVisible(false);

            ui->languageRadioButton->setVisible(true);
            ui->pictureRadioButton->setVisible(false);
            ui->otherRadioButton->setVisible(true);

            ui->reasonTitleLabel->setVisible(false);

            ui->durationHorizontalWidget->setVisible(false);

            ui->languageRadioButton->setText("Too Long");

            auto text_edit = new PlainTextEditWithMaxChars();
            text_edit->setMaxChar((int) globals->maximum_number_allowed_bytes_modify_error_message_reason());
            text_edit->setObjectName(TEXT_EDIT_NAME);

            ui->otherTextEditVerticalWidget->layout()->addWidget(text_edit);

            break;
        }
        case DELETE_INPUT_ERROR_HANDLED: {

            ui->titleMoveReasonLabel->setVisible(true);
            ui->titleNoteLabel->setVisible(false);

            ui->languageRadioButton->setVisible(true);
            ui->pictureRadioButton->setVisible(true);
            ui->otherRadioButton->setVisible(true);

            ui->reasonTitleLabel->setVisible(true);
            ui->durationHorizontalWidget->setVisible(false);

            ui->reasonTitleLabel->setText("Description");
            ui->languageRadioButton->setText("Bug Fixed");
            ui->pictureRadioButton->setText("Bug Not Relevant");
            ui->otherRadioButton->setText("Do Not Set To Handled");
            ui->otherRadioButton->setToolTip("This option will 'delete' all of the selected errors. However "
                                             "it will not set them to 'Handled'. This means future errors can still be"
                                             "stored of this (version number)_(file name)_(line number).");

            ui->titleLabel->setText("State a short description of how the bug was handled.");

            auto text_edit = new PlainTextEditWithMaxChars();
            text_edit->setMaxChar((int) globals->maximum_number_allowed_bytes_modify_error_message_reason());
            text_edit->setObjectName(TEXT_EDIT_NAME);

            ui->otherTextEditVerticalWidget->layout()->addWidget(text_edit);

            break;
        }
        default:
            ui->titleLabel->setText(
                    QString("An error occurred invalid user delete input of '%1' passed.")
                    .arg(QString::number(delete_input))
                    );
            ui->primaryHorizontalWidget->setVisible(false);
            ui->okPushButton->setVisible(false);
            return;
    }
}

AccessStatusInvalidMessageDialog::~AccessStatusInvalidMessageDialog() {
    delete ui;
}

void AccessStatusInvalidMessageDialog::on_okPushButton_clicked() {

    ReturnStatusForDialog return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_OK;

    if (user_account_status != UserAccountStatus(-1)) {
        switch (user_account_status) {
            case STATUS_ACTIVE:
            case STATUS_REQUIRES_MORE_INFO: {

                auto text_edit = ui->otherTextEditVerticalWidget->findChild<PlainTextEditWithMaxChars*>(
                        TEXT_EDIT_NAME);

                if (text_edit != nullptr) {
                    reason = text_edit->toPlainText();
                } else {
                    error_string = "Text edit not found, unable to process message.";
                    return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                }

                break;
            }
            case STATUS_SUSPENDED: {

                if (ui->languageRadioButton->isChecked()) {
                    reason = "Inappropriate Language.";
                } else if (ui->pictureRadioButton->isChecked()) {
                    reason = "Inappropriate Picture.";
                } else if (ui->otherRadioButton->isChecked()) {
                    auto text_edit = ui->otherTextEditVerticalWidget->findChild<PlainTextEditWithMaxChars*>(
                            TEXT_EDIT_NAME);

                    if (text_edit != nullptr) {
                        reason = text_edit->toPlainText();
                    } else {
                        error_string = "Text edit not found, unable to process message.";
                        return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                    }
                } else {
                    error_string = "Invalid radio button was selected, unable to process message.";
                    return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                }

                duration_in_hours = ui->durationSpinBox->value();

                break;
            }
            case STATUS_BANNED: {

                if (ui->languageRadioButton->isChecked()) {
                    reason = "Inappropriate Language.";
                } else if (ui->pictureRadioButton->isChecked()) {
                    reason = "Inappropriate Picture.";
                } else if (ui->otherRadioButton->isChecked()) {
                    auto text_edit = ui->otherTextEditVerticalWidget->findChild<PlainTextEditWithMaxChars*>(
                            TEXT_EDIT_NAME);

                    if (text_edit != nullptr) {
                        reason = text_edit->toPlainText();
                    } else {
                        error_string = "Text edit not found, unable to process message.";
                        return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                    }
                } else {
                    error_string = "Invalid radio button was selected, unable to process message.";
                    return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                }

                break;
            }
            default: {
                error_string = QString("An error occurred invalid user account status of '%1' passed.")
                        .arg(QString::fromStdString(UserAccountStatus_Name(user_account_status)));
                return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                break;
            }
        }
    } else {
        switch (delete_input) {
            case DELETE_INPUT_SINGLE: {

                if (ui->languageRadioButton->isChecked()) {
                    reason = "Error message too long.";
                } else if (ui->otherRadioButton->isChecked()) {
                    auto text_edit = ui->otherTextEditVerticalWidget->findChild<PlainTextEditWithMaxChars*>(
                            TEXT_EDIT_NAME);

                    if (text_edit != nullptr) {
                        reason = text_edit->toPlainText();
                    } else {
                        error_string = "Text edit not found, unable to process message.";
                        return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                    }
                } else {
                    error_string = "Invalid radio button was selected, unable to process message.";
                    return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                }

                break;
            }
            case DELETE_INPUT_ERROR_HANDLED: {

                if (ui->languageRadioButton->isChecked()) {
                    error_handled_move_reason = ErrorHandledMoveReason::ERROR_HANDLED_REASON_BUG_FIXED;
                } else if (ui->pictureRadioButton->isChecked()) {
                    error_handled_move_reason = ErrorHandledMoveReason::ERROR_HANDLED_REASON_BUG_NOT_RELEVANT;
                } else if (ui->otherRadioButton->isChecked()) {
                    error_handled_move_reason = ErrorHandledMoveReason::ERROR_HANDLED_REASON_DELETE_ALL_DO_NOT_SET_HANDLED;
                } else {
                    error_string = "Invalid radio button was selected, unable to process message.";
                    return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                }

                auto text_edit = ui->otherTextEditVerticalWidget->findChild<PlainTextEditWithMaxChars*>(
                        TEXT_EDIT_NAME);

                if (text_edit != nullptr) {
                    reason = text_edit->toPlainText();
                } else {
                    error_string = "Text edit not found, unable to process message.";
                    return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                }

                break;
            }
            default: {
                error_string = QString("An error occurred invalid user delete input of '%1' passed.")
                        .arg(QString::fromStdString(UserAccountStatus_Name(user_account_status)));
                return_status = ReturnStatusForDialog::ACCESS_STATUS_RETURN_ERROR;
                break;
            }
        }
    }

    done(return_status);
}

void AccessStatusInvalidMessageDialog::on_cancelPushButton_clicked() {
    done(ReturnStatusForDialog::ACCESS_STATUS_RETURN_CANCEL);
}

