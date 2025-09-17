//
// Created by jeremiah on 3/27/23.
//

#include "get_events.h"
#include "grpc_channel.h"
#include "setup_login_info.h"

void GetEventsObject::runGetEvents(
        admin_event_commands::GetEventsRequest&& request,
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
                    event_request = std::move(request)
            ]
                    (const std::stop_token& stop_token) mutable {

                grpc::ClientContext context;
                admin_event_commands::GetEventsResponse response;

                std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
                    context.TryCancel();
                });

                std::unique_ptr<admin_event_commands::AdminEventCommandsService::Stub> request_fields_stub =
                        admin_event_commands::AdminEventCommandsService::NewStub(channel);
                context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds{60});

                setup_login_info(event_request.mutable_login_info());

                auto status = request_fields_stub->GetEventsRPC(
                        &context,
                        event_request,
                        &response
                );

                if (stop_token.stop_requested()) {
                    emit signal_internalCompleted();
                    emit signal_requestCanceled(completed_lambda);
                    return;
                } else if (!status.ok()) { //if grpc call failed
                    const std::string errorMessage =
                            "Get events failed..\nGrpc returned status.ok() == false; code: " +
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
                    std::string errorMessage = "Get events failed.\n" + response.error_message();

                    emit signal_internalCompleted();
                    emit signal_sendWarning(
                            "Error",
                            errorMessage.c_str(),
                            completed_lambda
                    );
                    return;
                }

                std::sort(response.mutable_events()->begin(), response.mutable_events()->end(), [](
                        const admin_event_commands::SingleEvent& lhs,
                        const admin_event_commands::SingleEvent& rhs
                        ){
                    return lhs.number_swipes_yes() > rhs.number_swipes_yes();
                });
                
                emit signal_internalCompleted();
                emit signal_getEventsSuccessfullyCompleted(
                    response,
                    completed_lambda
            );

            }
    );
}