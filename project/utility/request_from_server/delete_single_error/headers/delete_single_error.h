//
// Created by jeremiah on 11/4/21.
//

#pragma once

//
// Created by jeremiah on 10/30/21.
//

#pragma once

#include <run_server_call_base_class.h>

#include <QModelIndex>

#include <HandleErrors.grpc.pb.h>

class DeleteSingleErrorObject : public RunServerCallBaseClass {
    Q_OBJECT
public:

    void deleteSingleError(
            const std::string& error_oid,
            const std::string& reason_for_deletion,
            const QModelIndex& model_index,
            const std::function<void()>& completed_lambda
            );

    signals:

    //will be set to completed if successful
    void signal_runDeleteSuccessfullyCompleted(
            const std::string& error_oid,
            const QModelIndex& model_index,
            const std::function<void()>& completed_lambda = [](){}
            );

};