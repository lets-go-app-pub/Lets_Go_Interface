//
// Created by jeremiah on 10/28/21.
//

#pragma once

#include <functional>
#include <run_server_call_base_class.h>
#include <grpcpp/impl/codegen/client_context.h>

class RequestServerShutDownObject : public RunServerCallBaseClass {
Q_OBJECT

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runRequestServerShutDown(
            const std::shared_ptr<grpc::ChannelInterface>& passed_channel,
            const std::string& server_address,
            int channel_vector_index,
            void* item_id
    );

signals:

    void signal_requestServerShutdownCompleted(
            int channel_vector_index,
            void* item_id
    );
};