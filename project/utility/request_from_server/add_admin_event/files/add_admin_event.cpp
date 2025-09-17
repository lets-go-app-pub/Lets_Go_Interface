//
// Created by jeremiah on 3/24/23.
//

#include "add_admin_event.h"
#include "grpc_channel.h"
#include "setup_login_info.h"

void AddAdminEventObject::runAddAdminEvent(
        std::unique_ptr<EventRequestMessage>&& event_request,
        const std::function<void()>& completed_lambda
        ) {

    if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>(
        [
            completed_lambda,
            this,
            event_request=std::move(event_request)
        ]
        (const std::stop_token& stop_token) {

        grpc::ClientContext context;
        admin_event_commands::AddAdminEventRequest request;
        admin_event_commands::AddAdminEventResponse response;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        std::unique_ptr<admin_event_commands::AdminEventCommandsService::Stub> request_fields_stub =
                admin_event_commands::AdminEventCommandsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds{60});

        setup_login_info(request.mutable_login_info());

        request.mutable_event_request()->Swap(event_request.get());

        auto status = request_fields_stub->AddAdminEventRPC(&context, request, &response);

        if (stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled(completed_lambda);
            return;
        } else if (!status.ok()) { //if grpc call failed
            const std::string errorMessage =
                    "Add admin event failed..\nGrpc returned status.ok() == false; code: " +
                    std::to_string(status.error_code()) +
                    " message: " + status.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
            );
            return;
        } else if (!response.successful()) { //if failed to log in
            std::string errorMessage = "Add admin event failed.\n" + response.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
            );
            return;
        }

        emit signal_internalCompleted();
        emit signal_addAdminEventSuccessfullyCompleted(
            response.event_oid(),
            completed_lambda
        );

    });

}