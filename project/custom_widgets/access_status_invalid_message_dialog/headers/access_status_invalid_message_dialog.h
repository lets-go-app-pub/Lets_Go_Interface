//
// Created by jeremiah on 9/13/21.
//

#pragma once

#include <QDialog>

#include <UserAccountStatusEnum.grpc.pb.h>
#include <ErrorHandledMoveReasonEnum.grpc.pb.h>

QT_BEGIN_NAMESPACE
namespace Ui { class AccessStatusInvalidMessageDialog; }
QT_END_NAMESPACE

class AccessStatusInvalidMessageDialog : public QDialog {
Q_OBJECT

public:
    enum DeleteInputForDialog {
    DELETE_INPUT_SINGLE,
    DELETE_INPUT_ERROR_HANDLED,
    };

    explicit AccessStatusInvalidMessageDialog(UserAccountStatus status, QWidget* parent = nullptr);

    explicit AccessStatusInvalidMessageDialog(DeleteInputForDialog _delete_input, QWidget* parent = nullptr);

    ~AccessStatusInvalidMessageDialog() override;

    [[nodiscard]] QString getReasonString() const {
        return reason;
    }

    [[nodiscard]] QString getErrorString() const {
        return error_string;
    }

    [[nodiscard]] int getDurationInHours() const {
        return duration_in_hours;
    }

    [[nodiscard]] ErrorHandledMoveReason getErrorHandledMoveReason() const {
        return error_handled_move_reason;
    }

    enum ReturnStatusForDialog {
        ACCESS_STATUS_RETURN_OK,
        ACCESS_STATUS_RETURN_CANCEL,
        ACCESS_STATUS_RETURN_ERROR,
    };

private:
    Ui::AccessStatusInvalidMessageDialog* ui;

    UserAccountStatus user_account_status = UserAccountStatus(-1);
    DeleteInputForDialog delete_input = DeleteInputForDialog(-1);
    ErrorHandledMoveReason error_handled_move_reason = ErrorHandledMoveReason(-1);

    QString reason;
    QString error_string;
    int duration_in_hours = -1;

    const QString TEXT_EDIT_NAME = "DialogTextEdit";

private slots:

    void on_okPushButton_clicked();

    void on_cancelPushButton_clicked();
};

