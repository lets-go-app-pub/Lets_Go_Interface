//
// Created by jeremiah on 3/28/23.
//

#include "cancel_event.h"
#include "grpc_channel.h"
#include "setup_login_info.h"

void CancelEventObject::runCancelEvent(
        const std::string& _event_oid,
        const std::string& _event_title,
        const std::function<void()>& completed_lambda
) {

    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>(
            [
                    completed_lambda,
                    this,
                    event_title = _event_title,
                    event_oid = _event_oid
            ]
                    (const std::stop_token& stop_token) {

                grpc::ClientContext context;
                user_event_commands::CancelEventRequest request;
                user_event_commands::CancelEventResponse response;

                std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
                    context.TryCancel();
                });

                std::unique_ptr<user_event_commands::UserEventCommandsService::Stub> request_fields_stub =
                        user_event_commands::UserEventCommandsService::NewStub(channel);
                context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds{60});

                setup_login_info(request.mutable_login_info());

                request.set_event_oid(event_oid);

                auto status = request_fields_stub->CancelEventRPC(&context, request, &response);

                if (stop_token.stop_requested()) {
                    emit signal_internalCompleted();
                    emit signal_requestCanceled(completed_lambda);
                    return;
                } else if (!status.ok()) { //if grpc call failed
                    const std::string errorMessage =
                            "Cancel event failed..\nGrpc returned status.ok() == false; code: " +
                            std::to_string(status.error_code()) +
                            " message: " + status.error_message();

                    emit signal_internalCompleted();
                    emit signal_sendWarning(
                            "Error",
                            errorMessage.c_str(),
                            completed_lambda
                    );
                    return;
                } else if (response.return_status() != ReturnStatus::SUCCESS) { //if failed to log in
                    std::string errorMessage = "Cancel event failed.\n" + response.error_string();

                    emit signal_internalCompleted();
                    emit signal_sendWarning(
                            "Error",
                            errorMessage.c_str(),
                            completed_lambda
                    );
                    return;
                }

                emit signal_internalCompleted();
                emit signal_cancelEventSuccessfullyCompleted(
                    event_oid,
                    event_title,
                    completed_lambda
            );

            });

}