//
// Created by jeremiah on 9/18/21.
//

#pragma once

#pragma once

#include <QObject>
#include <SetAdminFields.grpc.pb.h>
#include "run_server_call_base_class.h"

class RemoveUserPicture : public RunServerCallBaseClass {

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runRemoveUserPicture(
            const std::string& user_oid,
            const std::string& picture_oid,
            const std::function<void()>& completed_lambda
            );
};