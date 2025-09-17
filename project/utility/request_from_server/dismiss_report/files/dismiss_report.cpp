//
// Created by jeremiah on 9/22/21.
//

#include "dismiss_report.h"

#include <setup_login_info.h>
#include <grpc_channel.h>
#include <user_login_info.h>
#include <log.h>

void DismissReport::dismissReport(
        const std::string& user_oid,
        const std::function<void()>& completed_lambda
        ) {

    if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    function_thread = std::make_unique<std::jthread>([
            user_oid,
            completed_lambda,
            this] (
                    const std::stop_token& stop_token
                    ) {

        grpc::ClientContext context;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        handle_reports::ReportUnaryCallRequest request;
        handle_reports::ReportResponseUnaryCallResponse response;

        setup_login_info(request.mutable_login_info());
        request.set_user_oid(user_oid);

        std::unique_ptr<handle_reports::HandleReportsService::Stub> request_fields_stub =
                handle_reports::HandleReportsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        //NOTE: this is a client stream to make it possible to programmatically send to server
        // however for GUI purposes it is treated as a unary call
        auto status = request_fields_stub->DismissReportRPC(&context, request, &response);

        if(stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled(completed_lambda);
            return;
        } else if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Dismiss report failed..\nGrpc returned status.ok() == false; code: " +
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
            std::string errorMessage = "Dismiss report failed.\n" + response.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
                    );
            return;
        }

        emit signal_internalCompleted();
        emit signal_reportRequestSuccessfullyCompleted(user_oid);
    });

}