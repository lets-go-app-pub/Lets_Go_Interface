//
// Created by jeremiah on 9/6/21.
//

#include <sstream>
#include "extract_errors.h"

#include "grpc_channel.h"
#include "user_login_info.h"
#include "setup_login_info.h"

void ExtractErrorsObject::extractErrors(ErrorUrgencyLevel urgency_level, long max_number_bytes,
                                        handle_errors::ErrorParameters* error_parameters,
                                        const std::function<void()>& completed_lambda) {

    if (function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    //NOTE: this function is responsible for deleting error_parameters until it reaches
    // request.set_allocated_error_parameters(error_parameters) at which point protobuf
    // will handle it

    function_thread = std::make_unique<std::jthread>([
                                                             urgency_level,
                                                             max_number_bytes,
                                                             error_parameters,
                                                             completed_lambda,
                                                             this
                                                     ](
            const std::stop_token& stop_token
    ) mutable {

        grpc::ClientContext context;
        handle_errors::ExtractErrorsRequest request;
        handle_errors::ExtractErrorsResponse response;

        [[maybe_unused]] std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        std::unique_ptr<handle_errors::HandleErrorsService::Stub> handle_errors_stub =
                handle_errors::HandleErrorsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        setup_login_info(request.mutable_login_info());

        request.set_allocated_error_parameters(error_parameters);
        request.set_max_number_bytes(max_number_bytes);

        auto reader = handle_errors_stub->ExtractErrorsRPC(&context, request);

        std::string error_message;
        auto errors = std::make_shared<std::vector<ExtractedErrorsWithIndexing>>();

        if (stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled(completed_lambda);
            return;
        }

        while (reader->Read(&response)) {
            if (stop_token.stop_requested()) {
                emit signal_internalCompleted();
                emit signal_requestCanceled(completed_lambda);
                return;
            } else if (!response.success()) {
                error_message = response.error_msg();
                break;
            } else {
                //to conserve bandwidth these values are not passed back from the server because
                // they do not change, however it is useful to have them set
                response.mutable_error_message()->set_error_origin(request.error_parameters().error_origin());
                response.mutable_error_message()->set_error_urgency_level(urgency_level);
                response.mutable_error_message()->set_version_number(request.error_parameters().version_number());
                response.mutable_error_message()->set_line_number(request.error_parameters().line_number());
                response.mutable_error_message()->set_file_name(request.error_parameters().file_name());

                errors->push_back(ExtractedErrorsWithIndexing(response));
            }
        }

        if (!error_message.empty()) {

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    QString::fromStdString(error_message),
                    completed_lambda
            );
            return;
        }

        grpc::Status extract_errors_status = reader->Finish();

        if (!extract_errors_status.ok()) {
            const std::string errorMessage = "Failed to extract errors.\nGrpc returned status.ok() == false; code: " +
                                             std::to_string(extract_errors_status.error_code()) +
                                             " message: " + extract_errors_status.error_message();

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    errorMessage.c_str(),
                    completed_lambda
            );
            return;
        }

        auto meta_data = context.GetServerTrailingMetadata();

        auto total_bytes_iterator = meta_data.find(globals->extract_errors_total_bytes_key());
        auto extracted_bytes_iterator = meta_data.find(globals->extract_errors_extracted_bytes_key());
        auto total_docs_iterator = meta_data.find(globals->extract_errors_total_docs_key());
        auto extracted_docs_iterator = meta_data.find(globals->extract_errors_extracted_docs_key());


        if (total_bytes_iterator == meta_data.end()
            || extracted_bytes_iterator == meta_data.end()
            || total_docs_iterator == meta_data.end()
            || extracted_docs_iterator == meta_data.end()
                ) {

            std::stringstream error_string;
            error_string
                    << "Invalid meta data received from server.\n"
                    << "total_bytes_iterator valid: " << std::to_string(total_bytes_iterator == meta_data.end()) << '\n'
                    << "extracted_bytes_iterator valid: " << std::to_string(
                    extracted_bytes_iterator == meta_data.end()) << '\n'
                    << "total_docs_iterator valid: " << std::to_string(total_docs_iterator == meta_data.end()) << '\n'
                    << "extracted_docs_iterator valid: " << std::to_string(
                    extracted_docs_iterator == meta_data.end()) << '\n';

            emit signal_internalCompleted();
            emit signal_sendWarning(
                    "Error",
                    QString::fromStdString(error_string.str()),
                    completed_lambda
            );
            return;
        }

        emit signal_internalCompleted();
        emit signal_extractErrorsSuccessfullyCompleted(
                request.error_parameters().error_origin(),
                urgency_level,
                (int)request.error_parameters().version_number(),
                request.error_parameters().file_name(),
                (int)request.error_parameters().line_number(),
                std::string(total_bytes_iterator->second.data(),
                                      total_bytes_iterator->second.length()),
                std::string(extracted_bytes_iterator->second.data(),
                                      extracted_bytes_iterator->second.length()),
                std::string(total_docs_iterator->second.data(),
                                      total_docs_iterator->second.length()),
                std::string(extracted_docs_iterator->second.data(),
                                      extracted_docs_iterator->second.length()),
                errors,
                completed_lambda
        );

    });

}
