//
// Created by jeremiah on 3/28/23.
//

#pragma once

#include <QObject>
#include "UserEventCommands.grpc.pb.h"
#include "run_server_call_base_class.h"

#include "thread"

class CancelEventObject: public RunServerCallBaseClass  {
Q_OBJECT
public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runCancelEvent(
            const std::string& _event_oid,
            const std::string& _event_title,
            const std::function<void()>& completed_lambda
    );

signals:

    //will be set to completed if successful
    void signal_cancelEventSuccessfullyCompleted(
            const std::string& event_oid,
            const std::string& event_title,
            const std::function<void()>& completed_lambda = [](){}
    );

};