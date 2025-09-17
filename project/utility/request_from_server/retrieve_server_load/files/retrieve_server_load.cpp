//
// Created by jeremiah on 10/27/21.
//

#include "retrieve_server_load.h"

#include <RetrieveServerLoad.pb.h>
#include <RetrieveServerLoad.grpc.pb.h>
#include <user_login_info.h>

void RetrieveServerLoadObject::runRetrieveServerLoad(
        int channel_vector_index,
        void* item_id,
        const std::shared_ptr<grpc::ChannelInterface>& passed_channel
        ) {

    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>(
            [channel_vector_index, item_id, passed_channel, this]
            (const std::stop_token& stop_token) {

                grpc::ClientContext context;

                retrieve_server_load::RetrieveServerLoadRequest request;
                retrieve_server_load::RetrieveServerLoadResponse response;

                request.set_request_num_clients(true);

                std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
                    context.TryCancel();
                });

                std::unique_ptr<retrieve_server_load::RetrieveServerLoadService::Stub> retrieve_server_load_stub =
                        retrieve_server_load::RetrieveServerLoadService::NewStub(passed_channel);

                context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

                auto status = retrieve_server_load_stub->RetrieveServerLoadRPC(&context, request, &response);

                if (stop_token.stop_requested()) {
                    emit signal_internalCompleted();
                    emit signal_requestCanceled([](){});
                    return;
                }

                emit signal_internalCompleted();
                emit signal_requestServerLoadFinished(
                        channel_vector_index,
                        item_id,
                        status.ok(),
                        status.error_code(),
                        response.accepting_connections(),
                        response.num_clients()
                    );
            });

}

