//
// Created by jeremiah on 9/6/21.
//

#pragma once

#include <QObject>
#include <RequestFields.grpc.pb.h>
#include "run_server_call_base_class.h"

#include "thread"

class RequestActivitiesCategoriesObject: public RunServerCallBaseClass {

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runRequestActivitiesCategories(const std::function<void()>& completed_lambda);
};