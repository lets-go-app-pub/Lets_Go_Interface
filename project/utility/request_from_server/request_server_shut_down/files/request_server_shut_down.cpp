//
// Created by jeremiah on 10/28/21.
//

#include "request_server_shut_down.h"

#include <ManageServerCommands.pb.h>
#include <ManageServerCommands.grpc.pb.h>
#include <user_login_info.h>
#include <setup_login_info.h>

void RequestServerShutDownObject::runRequestServerShutDown(
        const std::shared_ptr<grpc::ChannelInterface>& passed_channel,
        const std::string& server_address,
        int channel_vector_index,
        void* item_id
        ) {

    if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>([
            passed_channel,
            server_address,
            channel_vector_index,
            item_id,
            this] (
                    const std::stop_token& stop_token
                    ) {

        grpc::ClientContext context;

        manage_server_commands::RequestServerShutdownRequest request;
        manage_server_commands::RequestServerShutdownResponse response;

        setup_login_info(request.mutable_login_info());
        request.set_server_address(server_address);

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        std::unique_ptr<manage_server_commands::ManageServerCommandsService::Stub> request_fields_stub =
                manage_server_commands::ManageServerCommandsService::NewStub(passed_channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        //NOTE: this is a client stream to make it possible to programmatically send to server
        // however for GUI purposes it is treated as a unary call
        auto status = request_fields_stub->RequestServerShutdownRPC(&context, request, &response);

        if(stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled([](){});
            return;
        } else if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Request server shut down failed.\nGrpc returned status.ok() == false; code: " +
                    std::to_string(status.error_code()) +
                    " message: " + status.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    [](){}
                    );
            return;
        } else if (!response.successful()) { //if failed to log in
            std::string errorMessage = "Request server shut down failed.\n" + response.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    [](){}
                    );
            return;
        }

        emit signal_internalCompleted();
        emit signal_requestServerShutdownCompleted(
                channel_vector_index,
                item_id
                );
    });
}
