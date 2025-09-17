//
// Created by jeremiah on 11/5/21.
//

#pragma once

#include <run_server_call_base_class.h>

#include <QModelIndex>

#include <HandleErrors.grpc.pb.h>

class SetErrorToHandledObject : public RunServerCallBaseClass {
    Q_OBJECT
public:

    void setErrorToHandled(
            handle_errors::ErrorParameters* error_parameters,
            ErrorHandledMoveReason error_handled_move_reason,
            const std::string& reason_for_deletion,
            const QModelIndex& model_index,
            const std::function<void()>& completed_lambda
            );

    signals:

    //will be set to completed if successful
    void signal_setErrorToHandledSuccessfullyCompleted(
            ErrorOriginType origin,
            unsigned int version_number,
            const std::string& file_name,
            unsigned int line_number,
            const QModelIndex& model_index,
            const std::function<void()>& completed_lambda = [](){}
            );

};