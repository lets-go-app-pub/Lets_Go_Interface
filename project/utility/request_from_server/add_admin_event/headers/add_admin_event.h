//
// Created by jeremiah on 3/24/23.
//

#pragma once

#include <QObject>
#include <AdminEventCommands.grpc.pb.h>
#include "run_server_call_base_class.h"

#include "thread"


class AddAdminEventObject: public RunServerCallBaseClass  {
Q_OBJECT
public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runAddAdminEvent(
            std::unique_ptr<EventRequestMessage>&& event_request,
            const std::function<void()>& completed_lambda
    );

signals:

    //will be set to completed if successful
    void signal_addAdminEventSuccessfullyCompleted(
            const std::string& event_oid,
            const std::function<void()>& completed_lambda = [](){}
    );

};