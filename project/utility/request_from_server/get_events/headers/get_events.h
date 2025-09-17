//
// Created by jeremiah on 3/27/23.
//

#pragma once

#include <QObject>
#include <AdminEventCommands.grpc.pb.h>
#include "run_server_call_base_class.h"

#include "thread"

class GetEventsObject: public RunServerCallBaseClass  {
    Q_OBJECT
public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runGetEvents(
            admin_event_commands::GetEventsRequest&& request,
            const std::function<void()>& completed_lambda
    );

    signals:

    //will be set to completed if successful
    void signal_getEventsSuccessfullyCompleted(
        const admin_event_commands::GetEventsResponse& response,
        const std::function<void()>& completed_lambda = [](){}
        );

};