//
// Created by jeremiah on 9/18/21.
//

#include <setup_login_info.h>
#include <grpc_channel.h>
#include <user_login_info.h>
#include "set_default_picture.h"

void SetDefaultPicture::runSetDefaultPicture(
        [[maybe_unused]] const QByteArray& image,
        [[maybe_unused]] const QByteArray& thumbnail,
        [[maybe_unused]] const std::function<void()>& completed_lambda
) {
    /*if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
    }

    request.Clear();

    //This will make a deep copy of the QByteArray values. This means that they can be changed
    // freely after the request variable is generated.
    setup_login_info(request.mutable_login_info());
    request.set_compressed_image(std::string(image.constData(), image.size()));
    request.set_compressed_image_size(image.size());
    request.set_compressed_thumbnail(std::string(thumbnail.constData(), thumbnail.size()));
    request.set_compressed_thumbnail_size(thumbnail.size());

    function_thread = std::make_unique<std::jthread>([
            completed_lambda,
            this] (
                    const std::stop_token& stop_token
                    ) {

        grpc::ClientContext context;

        set_admin_fields::SetAdminUnaryCallResponse response;

        std::stop_callback stop_callback = std::stop_callback(stop_token, [&context]() {
            context.TryCancel();
        });

        std::unique_ptr<set_admin_fields::SetAdminFieldsService::Stub> request_fields_stub =
                set_admin_fields::SetAdminFieldsService::NewStub(channel);
        context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        //NOTE: this is a client stream to make it possible to programmatically send to server
        // however for GUI purposes it is treated as a unary call
        auto status = request_fields_stub->SetDefaultUserPictureRPC(&context, request, &response);

        if(stop_token.stop_requested()) {
            emit signal_internalCompleted();
            emit signal_requestCanceled(completed_lambda);
            return;
        } else if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Set default picture failed..\nGrpc returned status.ok() == false; code: " +
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
            std::string errorMessage = "Set default picture failed.\n" + response.error_message();

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
    });*/
}

