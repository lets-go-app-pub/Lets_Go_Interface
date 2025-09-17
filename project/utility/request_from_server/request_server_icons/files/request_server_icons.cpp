//
// Created by jeremiah on 9/6/21.
//

#include "grpc_channel.h"
#include "user_login_info.h"
#include "setup_login_info.h"
#include "request_server_icons.h"

void RequestIconsObject::runRequestIcons(const std::function<void()>& completed_lambda) {

    if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>([completed_lambda, this](
            const std::stop_token& stop_token
            ) {

        grpc::ClientContext context;
        request_fields::ServerIconsRequest request;
        request_fields::ServerIconsResponse response;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        std::unique_ptr<request_fields::RequestFieldsService::Stub> request_fields_stub =
                request_fields::RequestFieldsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds{60});

        setup_login_info(request.mutable_login_info());

        auto reader = request_fields_stub->RequestServerIconsRPC(&context, request);

        std::vector<request_fields::ServerIconsResponse> temp_icons;

        while (reader->Read(&response)) {
            if(stop_token.stop_requested()) {
                emit signal_internalCompleted();
                emit signal_requestCanceled(completed_lambda);
                return;
            } else if (response.return_status() == ReturnStatus::SUCCESS) {
                temp_icons.push_back(response);
            }
        }

        std::sort(temp_icons.begin(), temp_icons.end(),
                  [](const request_fields::ServerIconsResponse& lhs, const request_fields::ServerIconsResponse& rhs) {
            return lhs.index_number() < rhs.index_number();
        });

        grpc::Status icons_status = reader->Finish();

        if(stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled(completed_lambda);
            return;
        }

        if (!icons_status.ok()) {
            const std::string errorMessage = "Failed to request icons.\nGrpc returned status.ok() == false; code: " +
                    std::to_string(icons_status.error_code()) +
                    " message: " + icons_status.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
                    );
            return;
        }

        std::unique_lock<std::mutex> icons_lock(icons_mutex);

        icons.clear();
        icons.insert(icons.begin(), std::make_move_iterator(temp_icons.begin()),
                     std::make_move_iterator(temp_icons.end()));

        emit signal_internalCompleted();
        emit signal_requestSuccessfullyCompleted(completed_lambda);

    });

}
