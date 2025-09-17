//
// Created by jeremiah on 9/22/21.
//

#pragma once

#include <QObject>
#include <HandleReports.grpc.pb.h>
#include "run_server_call_base_class.h"

class TimeOutUser : public RunServerCallBaseClass {
Q_OBJECT

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void timeOutUser(
            const std::string& user_oid,
            const std::string& inactive_message,
            const std::function<void()>& completed_lambda
    );

signals:

    //will be set to completed if successful
    void signal_reportRequestSuccessfullyCompleted(
            const std::string& timed_out_user_oid,
            const std::string& reason,
            const handle_reports::TimeOutUserResponse& time_out_response
            );

};
