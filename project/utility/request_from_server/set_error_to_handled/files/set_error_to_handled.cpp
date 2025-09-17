//
// Created by jeremiah on 11/5/21.
//

#include <setup_login_info.h>
#include <grpc_channel.h>
#include <user_login_info.h>
#include "set_error_to_handled.h"

void SetErrorToHandledObject::setErrorToHandled(
        handle_errors::ErrorParameters* error_parameters,
        ErrorHandledMoveReason error_handled_move_reason,
        const std::string& reason_for_deletion,
        const QModelIndex& model_index,
        const std::function<void()>& completed_lambda
        ) {

    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    //NOTE: this function is responsible for deleting error_parameters until it reaches
    // request.set_allocated_error_parameters(error_parameters) at which point protobuf
    // will handle it

    function_thread = std::make_unique<std::jthread>(
            [
                    error_parameters,
                    error_handled_move_reason,
                    reason_for_deletion,
                    model_index,
                    completed_lambda,
                    this
                    ]
                    (
                            const std::stop_token& stop_token
                            ) {

                grpc::ClientContext context;

                handle_errors::SetErrorToHandledRequest request;
                handle_errors::SetErrorToHandledResponse response;

                request.set_allocated_error_parameters(error_parameters);
                request.set_reason_for_description(reason_for_deletion);
                request.set_reason(error_handled_move_reason);

                setup_login_info(request.mutable_login_info());

                std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
                    context.TryCancel();
                });

                std::unique_ptr<handle_errors::HandleErrorsService::Stub> handle_errors_stub =
                        handle_errors::HandleErrorsService::NewStub(channel);
                context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

                //NOTE: this is a client stream to make it possible to programmatically send to server
                // however for GUI purposes it is treated as a unary call
                auto status = handle_errors_stub->SetErrorToHandledRPC(&context, request, &response);

                if (stop_token.stop_requested()) {
                    emit signal_internalCompleted();
                    emit signal_requestCanceled(completed_lambda);
                    return;
                } else if (!status.ok()) { //if grpc call failed
                    const std::string errorMessage =
                            "Set error to handled failed..\nGrpc returned status.ok() == false; code: " +
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
                    std::string errorMessage = "Set error to handled failed.\n" + response.error_msg();

                    emit signal_internalCompleted();
                    emit signal_sendWarning(
                            "Error",
                            errorMessage.c_str(),
                            completed_lambda
                            );
                    return;
                }

                emit signal_internalCompleted();
                emit signal_setErrorToHandledSuccessfullyCompleted(
                        request.error_parameters().error_origin(),
                        request.error_parameters().version_number(),
                        request.error_parameters().file_name(),
                        request.error_parameters().line_number(),
                        model_index,
                        completed_lambda
                        );
            });
}