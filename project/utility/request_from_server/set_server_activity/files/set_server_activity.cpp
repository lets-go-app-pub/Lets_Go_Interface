//
// Created by jeremiah on 9/7/21.
//

#include "grpc_channel.h"
#include "user_login_info.h"
#include "setup_login_info.h"
#include "set_server_activity.h"

void SetServerActivity::runSetServerActivity(
        const bool delete_this,
        const std::string& activity_name,
        const std::string& icon_display_name,
        const int min_age,
        const int category_index,
        const int icon_index,
        const std::function<void()>& completed_lambda
        ) {

    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>([delete_this, activity_name, icon_display_name, min_age, category_index, icon_index, completed_lambda, this](
            const std::stop_token& stop_token
    ) {

        grpc::ClientContext context;
        set_admin_fields::SetServerActivityOrCategoryRequest request;
        set_admin_fields::SetServerValuesResponse response;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        std::unique_ptr <set_admin_fields::SetAdminFieldsService::Stub> request_fields_stub =
                set_admin_fields::SetAdminFieldsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        setup_login_info(request.mutable_login_info());
        request.set_delete_this(delete_this);
        request.set_display_name(activity_name);
        request.set_icon_display_name(icon_display_name.empty() ? activity_name : icon_display_name);
        request.set_min_age(min_age);
        request.set_category_index(category_index);
        request.set_icon_index(icon_index);

        //NOTE: this is a client stream to make it possible to programmatically send to server
        // however for GUI purposes it is treated as a unary call
        auto writer = request_fields_stub->SetServerActivityRPC(&context, &response);

        //grpc documentation will show to break out of the loop sending if writer returns false
        // however there is no loop because only sending 1 message and the writer does not
        // need to be checked for its return value (the status will be checked below)
        writer->Write(request);

        writer->WritesDone();
        grpc::Status status = writer->Finish();

        if (stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled(completed_lambda);
            return;
        } else if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Set server Activity failed.\nGrpc returned status.ok() == false; code: " +
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
            std::string errorMessage = "Set server Activity failed.\n";
            for (const auto& err : response.error_messages()) {
                errorMessage += err + '\n';
            }

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
            );
            return;
        }

        emit signal_internalCompleted();
        emit signal_requestSuccessfullyCompleted(completed_lambda);
    });

}