//
// Created by jeremiah on 9/6/21.
//

#include "grpc_channel.h"
#include "user_login_info.h"
#include "setup_login_info.h"
#include "request_server_activities_categories.h"

void RequestActivitiesCategoriesObject::runRequestActivitiesCategories(const std::function<void()>& completed_lambda) {

    if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>([completed_lambda, this] (
            const std::stop_token& stop_token
            ) {

        grpc::ClientContext context;
        request_fields::InfoFieldRequest request;
        request_fields::ServerActivitiesResponse response;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        setup_login_info(request.mutable_login_info());

        std::unique_ptr<request_fields::RequestFieldsService::Stub> request_fields_stub =
                request_fields::RequestFieldsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        grpc::Status status = request_fields_stub->RequestServerActivitiesRPC(&context, request, &response);

        if(stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled(completed_lambda);
            return;
        } else if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Request server activities and categories failed.\nGrpc returned status.ok() == false; code: " +
                    std::to_string(status.error_code()) +
                    " message: " + status.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
                    );
            return;
        } else if (response.return_status() != SUCCESS) { //if failed
            const std::string errorMessage = "Request server activities and categories failed.\nReturned ReturnStatus: " +
                    ReturnStatus_Name(response.return_status());

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
                    );
            return;
        }

        std::unique_lock<std::mutex> categories_lock(categories_mutex, std::defer_lock);
        std::unique_lock<std::mutex> activities_lock(activities_mutex, std::defer_lock);

        std::lock(categories_lock, activities_lock);
        categories = response.server_categories();
        activities = response.server_activities();

        emit signal_internalCompleted();
        emit signal_requestSuccessfullyCompleted(completed_lambda);

    });

}
