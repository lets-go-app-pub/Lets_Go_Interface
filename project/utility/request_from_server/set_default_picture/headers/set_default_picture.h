//
// Created by jeremiah on 9/18/21.
//

#pragma once

#include <QObject>
#include <SetAdminFields.grpc.pb.h>
#include "run_server_call_base_class.h"

class SetDefaultPicture : public RunServerCallBaseClass {

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runSetDefaultPicture(
            const QByteArray& image,
            const QByteArray& thumbnail,
            const std::function<void()>& completed_lambda
            );

private:
    //set_admin_fields::SetDefaultUserPictureRequest request;
};
