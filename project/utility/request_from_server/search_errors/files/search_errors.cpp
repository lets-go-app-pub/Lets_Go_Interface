//
// Created by jeremiah on 10/30/21.
//

#include <user_login_info.h>
#include <grpc_channel.h>
#include <setup_login_info.h>
#include "search_errors.h"

void SearchErrors::runSearchErrors(
        handle_errors::SearchErrorsRequest& request,
        const std::function<void()>& completed_lambda
) {
    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    setup_login_info(request.mutable_login_info());

    function_thread = std::make_unique<std::jthread>(
            [
                    completed_lambda,
                    request, //need to copy this because it will be destroyed
                    this
            ]
                    (
                            const std::stop_token& stop_token
                    ) {

                grpc::ClientContext context;

                handle_errors::SearchErrorsResponse response;

                std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
                    context.TryCancel();
                });

                std::unique_ptr<handle_errors::HandleErrorsService::Stub> handle_errors_stub =
                        handle_errors::HandleErrorsService::NewStub(channel);
                context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

                //NOTE: this is a client stream to make it possible to programmatically send to server
                // however for GUI purposes it is treated as a unary call
                auto status = handle_errors_stub->SearchErrorsRPC(&context, request, &response);

                if (stop_token.stop_requested()) {
                    emit signal_internalCompleted();
                    emit signal_requestCanceled(completed_lambda);
                    return;
                } else if (!status.ok()) { //if grpc call failed
                    const std::string errorMessage =
                            "Search errors failed..\nGrpc returned status.ok() == false; code: " +
                            std::to_string(status.error_code()) +
                            " message: " + status.error_message();

                    emit signal_internalCompleted();
                    emit signal_sendWarning(
                            "Error",
                            errorMessage.c_str(),
                            completed_lambda
                    );
                    return;
                } else if (!response.success()) { //if failed to log in
                    std::string errorMessage = "Search errors failed.\n" + response.error_msg();

                    emit signal_internalCompleted();
                    emit signal_sendWarning(
                            "Error",
                            errorMessage.c_str(),
                            completed_lambda
                    );
                    return;
                }

                emit signal_internalCompleted();
                emit signal_runSearchSuccessfullyCompleted(response, completed_lambda);
            });
}
