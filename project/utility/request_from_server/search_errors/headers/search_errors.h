//
// Created by jeremiah on 10/30/21.
//

#pragma once

#include <run_server_call_base_class.h>

#include <HandleErrors.grpc.pb.h>

class SearchErrors : public RunServerCallBaseClass {
    Q_OBJECT
public:

    void runSearchErrors(
            handle_errors::SearchErrorsRequest& request,
            const std::function<void()>& completed_lambda
            );

    signals:

    //will be set to completed if successful
    void signal_runSearchSuccessfullyCompleted(
            const handle_errors::SearchErrorsResponse& response,
            const std::function<void()>& completed_lambda = [](){}
            );

};