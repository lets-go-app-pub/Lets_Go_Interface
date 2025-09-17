//
// Created by jeremiah on 9/7/21.
//

#pragma once

#include <QObject>
#include <SetAdminFields.grpc.pb.h>
#include "run_server_call_base_class.h"

class SetServerActivity : public RunServerCallBaseClass {

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runSetServerActivity(
            bool delete_this,
            const std::string& activity_name,
            const std::string& icon_display_name,
            int min_age,
            int category_index,
            int icon_index,
            const std::function<void()>& completed_lambda
    );
};