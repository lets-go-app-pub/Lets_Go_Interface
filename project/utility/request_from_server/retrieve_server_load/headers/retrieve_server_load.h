//
// Created by jeremiah on 10/27/21.
//

#pragma once

#include <run_server_call_base_class.h>
#include <grpcpp/impl/codegen/channel_interface.h>

class RetrieveServerLoadObject : public RunServerCallBaseClass {
Q_OBJECT

public:

    //the completed_lambda will be sent back with the signal in case any 'clean up' work
    // needs to be done (like stopping loading) especially relevant for UI thread
    void runRetrieveServerLoad(
            int channel_vector_index,
            void* item_id,
            const std::shared_ptr<grpc::ChannelInterface>& passed_channel
    );


signals:

    void signal_requestServerLoadFinished(
            int channel_vector_index,
            void* item_id,
            bool status_ok,
            grpc::StatusCode error_code,
            bool accepting_connections,
            int number_connected_clients
    );

};