//
// Created by jeremiah on 9/7/21.
//

#pragma once

#include <QObject>
#include <SetAdminFields.grpc.pb.h>
#include "run_server_call_base_class.h"

class SetServerIcon: public RunServerCallBaseClass  {

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runSetServerIcon(
            bool push_back,
            int index_number,
            bool icon_active,
            const QByteArray& icon_image,
            const std::function<void()>& completed_lambda
            );

private:
    set_admin_fields::SetServerIconRequest request;

};