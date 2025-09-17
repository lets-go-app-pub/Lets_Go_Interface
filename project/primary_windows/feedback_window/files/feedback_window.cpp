//
// Created by jeremiah on 8/30/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_feedback_window.h" resolved

#include <QMessageBox>

#include "setup_login_info.h"
#include "grpc_channel.h"
#include "user_login_info.h"
#include "feedback_window.h"
#include "ui_feedback_window.h"
#include "homewindow.h"

FeedbackWindow::FeedbackWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::FeedbackWindow) {
    ui->setupUi(this);

    connect(this, &FeedbackWindow::signal_passNewVector, this, &FeedbackWindow::slot_updatedFeedbackUpdateVector);
    connect(this, &FeedbackWindow::signal_sendWarning, this, &FeedbackWindow::slot_displayWarning);
    connect(this, &FeedbackWindow::signal_updateMostRecentlyViewedFeedbackTime,
            this, &FeedbackWindow::slot_updateMostRecentlyViewedFeedbackTime);
    connect(this, &FeedbackWindow::signal_updateSetFeedbackToSpam,
            this, &FeedbackWindow::slot_updateSetFeedbackToSpam);

    activity_suggestion_feedback.feedback_type = FeedbackType::FEEDBACK_TYPE_ACTIVITY_SUGGESTION;
    other_feedback.feedback_type = FeedbackType::FEEDBACK_TYPE_OTHER_FEEDBACK;
    bug_report_feedback.feedback_type = FeedbackType::FEEDBACK_TYPE_BUG_REPORT;

    activity_suggestion_feedback.feedback_english_name = "Activity Suggestion";
    other_feedback.feedback_english_name = "Other";
    bug_report_feedback.feedback_english_name = "Bug Report";

    //disable buttons until initial call is made
    allButtonsSetEnabled(false);

    showCurrentFeedback();

}

FeedbackWindow::~FeedbackWindow() {
    delete ui;
}

void FeedbackWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

void FeedbackWindow::on_activityFeedbackButton_clicked() {
    ui->feedbackTypeSelectedLabel->setText("Activity Feedback Selected");
    selectFeedbackButtonClicked(activity_suggestion_feedback);
}

void FeedbackWindow::on_otherFeedbackButton_clicked() {
    ui->feedbackTypeSelectedLabel->setText("Other Feedback Selected");
    selectFeedbackButtonClicked(other_feedback);
}

void FeedbackWindow::on_bugFeedbackButton_clicked() {
    ui->feedbackTypeSelectedLabel->setText("Bug Feedback Selected");
    selectFeedbackButtonClicked(bug_report_feedback);
}

void FeedbackWindow::selectFeedbackButtonClicked(
        FeedbackTypeObject& feedback_object
) {

    if (feedback_object.feedback_list.size() <= 2) { //if only caps OR vector is empty
        ui->feedbackPlainTextEdit->setPlainText("Loading...");
        requestInitialFeedbackFromServer(feedback_object.feedback_type);
    } else { //list is already initialized

        current_feedback_type = feedback_object.feedback_type;

        setupButtonsBasedOnIndex(
                feedback_object
        );

        showCurrentFeedback();
    }
}

void FeedbackWindow::on_feedbackNextButton_clicked() {

    FeedbackTypeObject& feedback_type_object = getFeedbackTypeObject(current_feedback_type);

    switch (current_feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
        case FEEDBACK_TYPE_BUG_REPORT:
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            break;
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 ("Error inside of on_feedbackNextButton_clicked(). Button"
                                  " should not be enabled when current_feedback_type == " +
                                  FeedbackType_Name(current_feedback_type)).c_str()
            );
            return;
    }

    beginNavigationToNextFeedback(
            feedback_type_object
    );
}

void FeedbackWindow::on_feedbackPreviousButton_clicked() {
    FeedbackTypeObject& feedback_type_object = getFeedbackTypeObject(current_feedback_type);

    switch (current_feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
        case FEEDBACK_TYPE_BUG_REPORT:
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            break;
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 ("Error inside of on_feedbackPreviousButton_clicked(). Button"
                                  " should not be enabled when current_feedback_type == " +
                                  FeedbackType_Name(current_feedback_type)).c_str()
            );
            return;
    }

    beginNavigationToPreviousFeedback(
            feedback_type_object
    );
}

void FeedbackWindow::on_feedbackSpamButton_clicked() {

    FeedbackTypeObject& feedback_type_object = getFeedbackTypeObject(current_feedback_type);

    switch (current_feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
        case FEEDBACK_TYPE_BUG_REPORT:
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            break;
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 ("Error inside of on_feedbackSpamButton_clicked(). Button"
                                  " should not be enabled when current_feedback_type == " +
                                  FeedbackType_Name(current_feedback_type)).c_str()
            );
            return;
    }

    if (feedback_type_object.getAtIndex().has_single_feedback_unit() //not an end cap
        &&
        feedback_type_object.getAtIndex().single_feedback_unit().marked_as_spam_by_admin_name().empty() //is not already marked as spam
        &&
        !feedback_type_object.getAtIndex().single_feedback_unit().marking_as_spam() //not currently being sent to server
            ) {
        feedback_type_object.feedback_list[feedback_type_object.current_index_in_feedback_list].mutable_single_feedback_unit()->set_marking_as_spam(
                true);
        setFeedbackToSpam(
                current_feedback_type,
                feedback_type_object.getAtIndex().single_feedback_unit().feedback_oid()
        );
    } //else {} //if this is an end cap, ignore the click

}

FeedbackWindow::FeedbackTypeObject& FeedbackWindow::getFeedbackTypeObject(FeedbackType feedback_type) {
    switch (feedback_type) {
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            //NOTE: handle these errors outside the function
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
            return activity_suggestion_feedback;
        case FEEDBACK_TYPE_BUG_REPORT:
            return bug_report_feedback;
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            return other_feedback;
    }

    return activity_suggestion_feedback;
}

void FeedbackWindow::slot_updatedFeedbackUpdateVector(
        FeedbackType feedback_type,
        const handle_feedback::GetFeedbackResponse& response,
        RequestUpdateType update_type
) {
    //NOTE: this is done on the main thread to guarantee no concurrency issues, with the vector and bool mainly

    FeedbackTypeObject& feedback_type_object = getFeedbackTypeObject(feedback_type);

    switch (feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
        case FEEDBACK_TYPE_BUG_REPORT:
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            break;
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 ("Error inside of slot_updatedFeedbackUpdateVector() invalid feedback_type passed.\nfeedback_type" +
                                  FeedbackType_Name(feedback_type)).c_str()
            );
            return;
    }

    switch (update_type) {
        case NO_UPDATE_REQUESTED:
            break;
        case UPDATE_NEXT_REQUESTED: {
            //must be atomic because the reference can be accessed by other threads
            std::atomic_store(&feedback_type_object.next_update_thread, null_pointer);
            feedback_type_object.requesting_next = false;
            bool update_showing_user_feedback = false;
            const size_t initial_vector_size = feedback_type_object.feedback_list.size();

            if (
                    current_feedback_type == feedback_type //current feedback is still selected

                    && response.feedback_list().size() > 1 //more than just a cap returned

                    && (feedback_type_object.current_index_in_feedback_list == //at an end of the feedback list
                        feedback_type_object.feedback_list.size() - 1
                        || feedback_type_object.current_index_in_feedback_list == 0)) {
                update_showing_user_feedback = true;
            }

            feedback_type_object.feedback_list.pop_back();

            //'cap' is included in list from server
            for (const auto& feedback : response.feedback_list()) {
                feedback_type_object.feedback_list.emplace_back(feedback);
            }

            unsigned long number_elements_to_remove = 0;
            if (feedback_type_object.feedback_list.size() > MAX_SIZE_OF_FEEDBACK_TYPE_VECTOR) {

                number_elements_to_remove = (long) (feedback_type_object.feedback_list.size() -
                                                    MAX_SIZE_OF_FEEDBACK_TYPE_VECTOR);

                feedback_type_object.feedback_list.erase(
                        feedback_type_object.feedback_list.begin(),
                        feedback_type_object.feedback_list.begin() + (long) number_elements_to_remove
                );

                //convert to a cap, if items were removed, then there are more feedback elements available
                feedback_type_object.feedback_list[0].mutable_more_feedback_elements_available();

            }

            const size_t final_vector_size = feedback_type_object.feedback_list.size();

            //index must move relative to the removed elements
            //also it is possible that elements were added and none were removed so must account for that
            feedback_type_object.current_index_in_feedback_list -= (unsigned int) number_elements_to_remove +
                                                                   (final_vector_size - initial_vector_size);

            if (update_showing_user_feedback) {
                showCurrentFeedback();
                setupButtonsBasedOnIndex(feedback_type_object);
            }

            break;
        }
        case UPDATE_PREVIOUS_REQUESTED: {
            //must be atomic because the reference can be accessed by other threads
            std::atomic_store(&feedback_type_object.previous_update_thread, null_pointer);
            feedback_type_object.requesting_previous = false;
            bool user_viewing_cap = false;
            const size_t initial_vector_size = feedback_type_object.feedback_list.size();

            if (
                    current_feedback_type == feedback_type //current feedback is still selected

                    && response.feedback_list().size() > 1 //more than just a cap returned

                    && (feedback_type_object.current_index_in_feedback_list == //at an end of the feedback list
                        feedback_type_object.feedback_list.size() - 1
                        || feedback_type_object.current_index_in_feedback_list == 0)) {
                user_viewing_cap = true;
            }

            //erase cap
            feedback_type_object.feedback_list.erase(
                    feedback_type_object.feedback_list.begin(),
                    feedback_type_object.feedback_list.begin() + 1
            );

            //insert new values into vector
            //'cap' is included in list from server
            feedback_type_object.feedback_list.insert(
                    feedback_type_object.feedback_list.begin(),
                    response.feedback_list().begin(),
                    response.feedback_list().end()
            );

            unsigned long number_elements_to_remove = 0;
            if (feedback_type_object.feedback_list.size() > MAX_SIZE_OF_FEEDBACK_TYPE_VECTOR) {

                number_elements_to_remove = (long) (feedback_type_object.feedback_list.size() -
                                                    MAX_SIZE_OF_FEEDBACK_TYPE_VECTOR);

                for (unsigned long i = 0; i < number_elements_to_remove; i++) {
                    feedback_type_object.feedback_list.pop_back();
                }

                //convert to a cap, if items were removed, then there are more feedback elements available
                feedback_type_object.feedback_list.back().mutable_more_feedback_elements_available();
            }

            const size_t final_vector_size = feedback_type_object.feedback_list.size();

            //index must move relative to the removed elements
            //also it is possible that elements were added and none were removed so must account for that
            feedback_type_object.current_index_in_feedback_list += (unsigned int) number_elements_to_remove +
                                                                   (final_vector_size - initial_vector_size);

            //update the plain text edit if user was viewing a cap
            if (user_viewing_cap) {
                showCurrentFeedback();
                setupButtonsBasedOnIndex(feedback_type_object);
            }

            break;
        }
    }
}

void
FeedbackWindow::slot_displayWarning(const QString& title, const QString& text, const std::function<void()>& lambda) {

    lambda();
    std::cout << "title: " << title.toStdString();
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(this,
                         title,
                         text
    );
}

void FeedbackWindow::slot_updateMostRecentlyViewedFeedbackTime(FeedbackType feedback_type,
                                                               const std::chrono::milliseconds& new_timestamp) {
    switch (feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
            activity_suggestion_feedback.updateTimestamp(new_timestamp);
            break;
        case FEEDBACK_TYPE_BUG_REPORT:
            bug_report_feedback.updateTimestamp(new_timestamp);
            break;
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            other_feedback.updateTimestamp(new_timestamp);
            break;
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 ("Error inside of slot_updateMostRecentlyViewedFeedbackTime() invalid feedback_type passed.\nfeedback_type" +
                                  FeedbackType_Name(feedback_type)).c_str()
            );
            break;
    }
}

void FeedbackWindow::slot_updateSetFeedbackToSpam(FeedbackType feedback_type, const std::string& feedback_oid) {

    FeedbackTypeObject& feedback_type_object = getFeedbackTypeObject(feedback_type);

    switch (feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
        case FEEDBACK_TYPE_BUG_REPORT:
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            break;
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 ("Error inside of slot_updateSetFeedbackToSpam() invalid feedback_type passed.\nfeedback_type" +
                                  FeedbackType_Name(feedback_type)).c_str()
            );
            return;
    }

    unsigned int index = 0;
    for (auto& element : feedback_type_object.feedback_list) {
        if (element.has_single_feedback_unit() && element.single_feedback_unit().feedback_oid() == feedback_oid) {

            element.mutable_single_feedback_unit()->set_marking_as_spam(false);
            element.mutable_single_feedback_unit()->set_marked_as_spam_by_admin_name(USER_NAME);

            if (current_feedback_type == feedback_type
                && index == feedback_type_object.current_index_in_feedback_list
                    ) {
                showCurrentFeedback();
            }
            break;
        }
        index++;
    }

}

void FeedbackWindow::saveInitialFeedbackToVector(
        FeedbackTypeObject& feedback_object,
        const handle_feedback::GetFeedbackResponse& response
) {

    feedback_object.feedback_list.clear();
    feedback_object.most_recently_viewed_feedback_timestamp = std::chrono::milliseconds{
            response.timestamp_of_most_recently_viewed_feedback()};

    int current_index = 0;
    //need to set this to let the app know the initial request was made
    feedback_object.current_index_in_feedback_list = 1;
    for (auto& feedback_element : response.feedback_list()) {

        if (feedback_element.has_single_feedback_unit()
            && feedback_element.single_feedback_unit().timestamp_stored_on_server() ==
               feedback_object.most_recently_viewed_feedback_timestamp.count()
                ) {
            //NOTE: this should never be the final element because there should always be an 'end cap' at the end
            // The plus 1 is to go to the NEXT element after the most recently observed element
            feedback_object.current_index_in_feedback_list = current_index + 1;
        }

        feedback_object.feedback_list.emplace_back(feedback_element);
        current_index++;
    }

    if (feedback_object.feedback_list.size() > 2
        && feedback_object.getAtIndex().single_feedback_unit().timestamp_stored_on_server() >
           feedback_object.most_recently_viewed_feedback_timestamp.count()
            ) { //if the newly displayed feedback requires updating the timestamp

        //NOTE: technically the internet could go down and not send this causing the client and server
        // to be out of sync, however any click to Next, Previous or selecting a feedback should fix it
        feedback_object.most_recently_viewed_feedback_timestamp = std::chrono::milliseconds{
                feedback_object.getAtIndex().single_feedback_unit().timestamp_stored_on_server()};
        updateMostRecentlyViewedFeedback(feedback_object.feedback_type,
                                         feedback_object.most_recently_viewed_feedback_timestamp);
    }

    std::cout << "FeedbackObject list size(): " << feedback_object.feedback_list.size() << '\n';
}

void FeedbackWindow::setupSpamButton(const handle_feedback::FeedbackElement& feedback_element) {
    if (feedback_element.has_single_feedback_unit()
        && !feedback_element.single_feedback_unit().marking_as_spam()
        && feedback_element.single_feedback_unit().marked_as_spam_by_admin_name().empty()
            ) {
        ui->feedbackSpamButton->setEnabled(true);
    } else {
        ui->feedbackSpamButton->setEnabled(false);
    }
}

void FeedbackWindow::setupButtonsBasedOnIndex(FeedbackTypeObject& feedback_object) {

    if (feedback_object.feedback_list.size() < 2) { //invalid vector size
        QMessageBox::warning(this,
                             "Error",
                             ("setupButtonsBasedOnIndex() was called with feedback size of " +
                              std::to_string(feedback_object.feedback_list.size())).c_str()
        );
        return;
    } else if (feedback_object.feedback_list.size() == 2) { //only 'caps'
        allButtonsSetEnabled(false);
    } else if (feedback_object.current_index_in_feedback_list == 0) { //at beginning 'cap'
        ui->feedbackNextButton->setEnabled(true);
        ui->feedbackPreviousButton->setEnabled(false);
        ui->feedbackSpamButton->setEnabled(false);
    } else if (feedback_object.current_index_in_feedback_list ==
               (feedback_object.feedback_list.size() - 1)) { //at end 'cap'
        ui->feedbackNextButton->setEnabled(false);
        ui->feedbackPreviousButton->setEnabled(true);
        ui->feedbackSpamButton->setEnabled(false);
    } else { //not at either end cap
        allButtonsSetEnabled(true);

        setupSpamButton(feedback_object.getAtIndex());
    }

}

void FeedbackWindow::allButtonsSetEnabled(bool enabled) {
    ui->feedbackNextButton->setEnabled(enabled);
    ui->feedbackPreviousButton->setEnabled(enabled);
    ui->feedbackSpamButton->setEnabled(enabled);
}

void FeedbackWindow::requestInitialFeedbackFromServer(FeedbackType feedback_type) {

    allButtonsSetEnabled(false);

    grpc::ClientContext unary_context;

    handle_feedback::GetInitialFeedbackRequest request;
    handle_feedback::GetFeedbackResponse response;
    setup_login_info(request.mutable_login_info());
    request.set_feedback_type(feedback_type);

    std::unique_ptr<handle_feedback::HandleFeedbackService::Stub> request_statistics_stub =
            handle_feedback::HandleFeedbackService::NewStub(channel);

    //NOTE: long deadline here to give it time to calculate
    unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

    grpc::Status status = request_statistics_stub->GetInitialFeedbackRPC(&unary_context, request,
                                                                         &response);

    if (!status.ok()) { //if grpc call failed
        const std::string errorMessage = "Grpc returned status.ok() == false; code: " +
                                         std::to_string(status.error_code()) +
                                         " message: " + status.error_message();

        QMessageBox::warning(this,
                             "Error",
                             errorMessage.c_str()
        );

        //NOTE: no reason to enable buttons if failed to initialize
        return;
    } else if (!response.success()) { //if failed to log in
        QMessageBox::warning(this,
                             "Error",
                             response.error_msg().c_str()
        );

        //NOTE: no reason to enable buttons if failed to initialize
        return;
    }

    if (response.feedback_list().size() < 2) {
        QMessageBox::warning(this,
                             "No Data",
                             "Error feedback list should always have at least 2 elements inside of it, the 'end caps' representing no feedback at the fewest."
        );

        //NOTE: no reason to enable buttons if failed to initialize
        return;
    }

    current_feedback_type = feedback_type;

    if (!runForAllFeedbackTypes([&](FeedbackTypeObject& feedback_object) {

        saveInitialFeedbackToVector(
                feedback_object,
                response
        );

        setupButtonsBasedOnIndex(
                feedback_object
        );
    })) {
        return;
    }

    showCurrentFeedback();
}

void FeedbackWindow::individualFeedbackDisplay(const FeedbackTypeObject& feedback_type_object,
                                               const std::string& feedback_message_text) {
    ui->feedbackTypeSelectedLabel->setText((feedback_type_object.feedback_english_name + " Feedback").c_str());

    switch (feedback_type_object.getAtIndex().TypeOfElement_case()) {
        case handle_feedback::FeedbackElement::kSingleFeedbackUnit:
            ui->feedbackPlainTextEdit->setPlainText(feedback_message_text.c_str());
            break;
        case handle_feedback::FeedbackElement::kEndOfFeedbackElement:
            if (feedback_type_object.current_index_in_feedback_list == 0) {
                ui->feedbackPlainTextEdit->setPlainText(
                        QString(BEGINNING_OF_LIST_REACHED_TEMPLATE).arg(
                                feedback_type_object.feedback_english_name.c_str()));
            } else {
                ui->feedbackPlainTextEdit->setPlainText(
                        QString(END_OF_LIST_REACHED_TEMPLATE).arg(feedback_type_object.feedback_english_name.c_str())
                );
            }
            break;
        case handle_feedback::FeedbackElement::kMoreFeedbackElementsAvailable: {
            QString string;

            int my_random_num = rand() % 5;

            switch (my_random_num) {
                case 0: {
                    string = "Loading... Probably?";
                    break;
                }
                case 1: {
                    string = "Your probably navigating too fast... Or maybe the internet connection/server is slow";
                    break;
                }
                case 2: {
                    string = "Are you navigating too fast again?";
                    break;
                }
                case 3: {
                    string = "Don't forget to take your time...";
                    break;
                }
                default: {
                    string = "Downloading more feedback, please wait.";
                    break;
                }
            }
            ui->feedbackPlainTextEdit->setPlainText(string);
            break;
        }
        case handle_feedback::FeedbackElement::TYPEOFELEMENT_NOT_SET:
            QMessageBox::warning(this,
                                 "Error",
                                 "Error hit FeedbackElement::TYPEOFELEMENT_NOT_SET inside individualFeedbackDisplay().");
            ui->feedbackPlainTextEdit->clear();
            break;
    }

}

void FeedbackWindow::showCurrentFeedback() {

    if (!makeSureCurrentFeedbackIndexIsValid()) {
        return;
    }

    FeedbackTypeObject& feedback_type_object = getFeedbackTypeObject(current_feedback_type);

    std::string message_text;
    std::string body;

    switch (current_feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION:
            body += "Activity Name: " +
                    feedback_type_object.getAtIndex().single_feedback_unit().activity_name() +
                    "\n\nUser Message: " +
                    feedback_type_object.getAtIndex().single_feedback_unit().message();
            break;
        case FEEDBACK_TYPE_BUG_REPORT:
        case FEEDBACK_TYPE_OTHER_FEEDBACK:
            body += "User Message: " +
                    feedback_type_object.getAtIndex().single_feedback_unit().message();
            break;
        case FEEDBACK_TYPE_UNKNOWN:
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 "Invalid feedback type selected inside showCurrentFeedback().");
            return;
    }

    //show if message has been marked as spam
    if (feedback_type_object.getAtIndex().has_single_feedback_unit()
        &&
        !feedback_type_object.getAtIndex().single_feedback_unit().marked_as_spam_by_admin_name().empty()) {
        message_text = "MARKED AS SPAM BY " +
                       feedback_type_object.getAtIndex().single_feedback_unit().marked_as_spam_by_admin_name() +
                       ".\n\n";
    }

    message_text += body;

    //show user sending user account OID if relevant
    if ((user_admin_privileges.find_single_users()
         || user_admin_privileges.update_single_users())
        && feedback_type_object.getAtIndex().has_single_feedback_unit()) {
        message_text += "\n\nUser ID: " +
                        feedback_type_object.getAtIndex().single_feedback_unit().account_oid();
    }

    individualFeedbackDisplay(
            feedback_type_object,
            message_text
    );

}

void FeedbackWindow::updateFeedbackThread(const long& timestamp_stored_on_server,
                                          FeedbackType feedback_type,
                                          const std::stop_token& stop_token,
                                          handle_feedback::GetFeedbackResponse& response,
                                          bool request_before_timestamp) {

    //NOTE: this currently runs on the MAIN thread so this function stops the UI from functioning
    grpc::ClientContext unary_context;

    //NOTE: this is OK to call from inside the function, it seems to be removed when the destructor is called
    //NOTE: I think TryCancel() should be thread safe, not even sure how it COULD be called from
    // the same thread. However, even if it doesn't cancel the message it should not hurt anything.
    std::stop_callback callback(stop_token, [&] {
        unary_context.TryCancel();
    });

    handle_feedback::GetFeedbackUpdateRequest request;
    setup_login_info(request.mutable_login_info());
    request.set_feedback_type(feedback_type);
    request.set_timestamp_of_message_at_end_of_list(timestamp_stored_on_server);
    request.set_request_before_timestamp(request_before_timestamp);

    std::unique_ptr<handle_feedback::HandleFeedbackService::Stub> request_statistics_stub = handle_feedback::HandleFeedbackService::NewStub(
            channel);

    unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

    grpc::Status status = request_statistics_stub->GetFeedbackUpdateRPC(&unary_context, request,
                                                                        &response);

    if (stop_token.stop_requested()) {
        return;
    } else if (!status.ok()) { //if grpc call failed
        const std::string errorMessage = "Grpc returned status.ok() == false; code: " +
                                         std::to_string(status.error_code()) +
                                         " message: " + status.error_message();

        emit signal_sendWarning("Error", errorMessage.c_str());
        return;
    } else if (!response.success()) { //if failed to log in

        emit signal_sendWarning("Error", response.error_msg().c_str());
        return;
    }
}

//NOTE: pass in feedback type, so it is constant through the function
void FeedbackWindow::updateMostRecentlyViewedFeedback(
        FeedbackType feedback_type,
        const std::chrono::milliseconds& most_recently_observed_feedback_time
) {

    std::jthread update_feedback_thread([feedback_type, most_recently_observed_feedback_time, this] {
        //NOTE: this currently runs on the MAIN thread so this function stops the UI from functioning
        grpc::ClientContext unary_context;

        handle_feedback::UpdateFeedbackTimeViewedRequest request;
        handle_feedback::UpdateFeedbackTimeViewedResponse response;
        setup_login_info(request.mutable_login_info());
        request.set_feedback_type(feedback_type);
        request.set_timestamp_feedback_observed_time(most_recently_observed_feedback_time.count());

        std::unique_ptr<handle_feedback::HandleFeedbackService::Stub> request_statistics_stub = handle_feedback::HandleFeedbackService::NewStub(
                channel);

        unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        grpc::Status status = request_statistics_stub->UpdateFeedbackTimeViewedRPC(&unary_context, request,
                                                                                   &response);

        if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Grpc returned status.ok() == false; code: " +
                                             std::to_string(status.error_code()) +
                                             " message: " + status.error_message();

            emit signal_sendWarning("Error", errorMessage.c_str());
            return;
        } else if (!response.success()) { //if failed to log in

            emit signal_sendWarning("Error", response.error_msg().c_str());
            return;
        }

        //update timestamp
        emit signal_updateMostRecentlyViewedFeedbackTime(
                feedback_type,
                std::chrono::milliseconds{response.timestamp_feedback_observed_time()});

    });

    //as far as I can tell the destructor inside std::jthread will run a join() for the
    // thread, so without detaching here, this would essentially be running
    // on the main thread
    update_feedback_thread.detach();
}

void FeedbackWindow::setMarkingAsSpam(
        FeedbackType feedback_type,
        const std::string& feedback_oid,
        const bool value
) {

    FeedbackTypeObject& feedback_type_object = getFeedbackTypeObject(feedback_type);

    for (auto& feedback : feedback_type_object.feedback_list) {
        if (feedback.has_single_feedback_unit()
            && feedback.single_feedback_unit().feedback_oid() == feedback_oid) {
            feedback.mutable_single_feedback_unit()->set_marking_as_spam(value);
            break;
        }
    }
}

void FeedbackWindow::setFeedbackToSpam(
        FeedbackType feedback_type,
        const std::string& feedback_oid
) {

    std::jthread set_as_spam_thread([feedback_type, feedback_oid, this] {
        //NOTE: this currently runs on the MAIN thread so this function stops the UI from functioning
        grpc::ClientContext unary_context;

        handle_feedback::SetFeedbackToSpamRequest request;
        handle_feedback::SetFeedbackToSpamResponse response;
        setup_login_info(request.mutable_login_info());
        request.set_feedback_type(feedback_type);
        request.set_feedback_oid(feedback_oid);

        std::unique_ptr<handle_feedback::HandleFeedbackService::Stub> handle_feedback_stub =
                handle_feedback::HandleFeedbackService::NewStub(channel);

        unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE);

        grpc::Status status = handle_feedback_stub->SetFeedbackToSpamRPC(&unary_context, request,
                                                                         &response);

        if (!status.ok()) { //if grpc call failed
            const std::string errorMessage = "Grpc returned status.ok() == false; code: " +
                                             std::to_string(status.error_code()) +
                                             " message: " + status.error_message();

            emit signal_sendWarning("Error", errorMessage.c_str(), [feedback_type, feedback_oid, this]() {
                setMarkingAsSpam(feedback_type, feedback_oid, false);
            });
            return;
        } else if (!response.success()) { //if failed to log in

            emit signal_sendWarning("Error", response.error_msg().c_str(), [feedback_type, feedback_oid, this]() {
                setMarkingAsSpam(feedback_type, feedback_oid, false);
            });
            return;
        }

        emit signal_updateSetFeedbackToSpam(feedback_type, feedback_oid);

    });

    set_as_spam_thread.detach();
}

//NOTE: pass in feedback type, so it is constant through the function
void FeedbackWindow::beginNavigationToNextFeedback(FeedbackTypeObject& feedback_type_object) {

    if (!makeSureCurrentFeedbackIndexIsValid()) {
        return;
    }

    if (feedback_type_object.atEndOfVector()
        &&
        feedback_type_object.getAtIndex().has_end_of_feedback_element()) { //if at the end of the list

        ui->feedbackNextButton->setEnabled(false);
        setupSpamButton(feedback_type_object.getAtIndex());
        showCurrentFeedback();
        return;
    } else { //if not at the end of the list AND not end of server feedback list

        switch (feedback_type_object.getAtIndex().TypeOfElement_case()) {
            case handle_feedback::FeedbackElement::kSingleFeedbackUnit:
            case handle_feedback::FeedbackElement::kEndOfFeedbackElement: //this can happen if at the BEGINNING of the list
                feedback_type_object.incrementIndex();
                //NOTE: when pressing previous it will be disabled at the beginning, so when next is pressed
                // it will need to enable previous if navigating
                ui->feedbackPreviousButton->setEnabled(true);

                if (!feedback_type_object.atEndOfVector()) { //if new index is NOT the end of the vector
                    updateMostRecentlyViewedFeedback(
                            feedback_type_object.feedback_type,
                            std::chrono::milliseconds{
                                    feedback_type_object.getAtIndex().single_feedback_unit().timestamp_stored_on_server()}
                    );
                } else if (feedback_type_object.getAtIndex().has_end_of_feedback_element()) { //if at the end of the list

                    ui->feedbackNextButton->setEnabled(false);

                    //NOTE: NOT returning here because new info could be available and so allowing an update to
                    // occur
                    //return;
                }

                setupSpamButton(feedback_type_object.getAtIndex());
                showCurrentFeedback();

                break;
            case handle_feedback::FeedbackElement::kMoreFeedbackElementsAvailable:
                ui->feedbackNextButton->setEnabled(false);
                break;
            case handle_feedback::FeedbackElement::TYPEOFELEMENT_NOT_SET:
                QMessageBox::warning(this,
                                     "Error",
                                     "Error beginNavigationToNextFeedback() found an element with TYPEOFELEMENT_NOT_SET. Restarting the app is recommended");
                return;
        }

        if (
                feedback_type_object.current_index_in_feedback_list >=
                feedback_type_object.getBeginningIndexToRequestNext() //close enough to the end to need more feedback

                && !feedback_type_object.requesting_next // NOT currently requesting more feedback
                ) {

            feedback_type_object.requesting_next = true;
            long timestamp_stored_on_server = feedback_type_object.lastTimestampInList();

            feedback_type_object.next_update_thread = std::make_shared<std::jthread>(
                    [timestamp_stored_on_server, &feedback_type_object, this](
                            const std::stop_token& stop_token) {
                        std::shared_ptr<std::jthread> previous_thread;

                        //NOTE: technically std::atomic_store() for std::shared_pointer is deprecated in favor of
                        // std::atomic<std::shared_ptr> however g++ has not yet implemented it at the time of
                        // writing this code
                        std::atomic_store(&previous_thread,
                                          feedback_type_object.previous_update_thread);

                        if (previous_thread != nullptr) {
                            previous_thread->request_stop();
                            previous_thread->join();
                        }

                        handle_feedback::GetFeedbackResponse response;

                        this->updateFeedbackThread(
                                timestamp_stored_on_server,
                                feedback_type_object.feedback_type,
                                stop_token,
                                response, false);

                        emit signal_passNewVector(
                                feedback_type_object.feedback_type,
                                response,
                                RequestUpdateType::UPDATE_NEXT_REQUESTED
                        );

                    }
            );

        } // else {} //currently, running next feedback, possible if user clicks next WHILE other is still running
    }
}

void FeedbackWindow::beginNavigationToPreviousFeedback(FeedbackTypeObject& feedback_type_object) {

    if (!makeSureCurrentFeedbackIndexIsValid()) {
        return;
    }

    if (feedback_type_object.atBeginningOfVector()
        && feedback_type_object.getAtIndex().has_end_of_feedback_element()) { //if at the end of the list

        //NOTE: this should technically never happen

        ui->feedbackPreviousButton->setEnabled(false);
        setupSpamButton(feedback_type_object.getAtIndex());
        showCurrentFeedback();
        return;
    } else { //if not at the end of the list with element type 'end_of_feedback_element'

        switch (feedback_type_object.getAtIndex().TypeOfElement_case()) {
            case handle_feedback::FeedbackElement::kSingleFeedbackUnit:
            case handle_feedback::FeedbackElement::kEndOfFeedbackElement: //this can happen if at the END of the list
                feedback_type_object.decrementIndex();
                //NOTE: when pressing next it will be disabled at the end, so when previous is pressed
                // it will need to enable next if navigating
                ui->feedbackNextButton->setEnabled(true);

                setupSpamButton(feedback_type_object.getAtIndex());
                showCurrentFeedback();

                if (feedback_type_object.atBeginningOfVector()
                    &&
                    feedback_type_object.getAtIndex().has_end_of_feedback_element()) { //if at the beginning of the list

                    //This can happen after the index has been decremented
                    ui->feedbackPreviousButton->setEnabled(false);

                    //return here, no reason to run an update if no elements are before this point
                    return;
                }

                break;
            case handle_feedback::FeedbackElement::kMoreFeedbackElementsAvailable:
                ui->feedbackPreviousButton->setEnabled(false);
                break;
            case handle_feedback::FeedbackElement::TYPEOFELEMENT_NOT_SET:
                QMessageBox::warning(this,
                                     "Error",
                                     "Error beginNavigationToPreviousFeedback() found an element with TYPEOFELEMENT_NOT_SET. Restarting the app is recommended");
                return;
        }

        if (
                feedback_type_object.current_index_in_feedback_list <=
                feedback_type_object.getFinalIndexToRequestPrevious() //close enough to the beginning to need more feedback

                && !feedback_type_object.requesting_previous // NOT currently requesting more feedback
                ) {

            feedback_type_object.requesting_previous = true;
            long timestamp_stored_on_server = feedback_type_object.firstTimestampInList();

            feedback_type_object.previous_update_thread = std::make_shared<std::jthread>(
                    [timestamp_stored_on_server, &feedback_type_object, this](
                            const std::stop_token& stop_token) {
                        std::shared_ptr<std::jthread> next_thread;

                        //NOTE: technically std::atomic_store() for std::shared_pointer is deprecated in favor of
                        // std::atomic<std::shared_ptr<>> however g++ has not yet implemented it at the time of
                        // writing this code
                        std::atomic_store(&next_thread,
                                          feedback_type_object.next_update_thread);

                        if (next_thread != nullptr) {
                            next_thread->request_stop();
                            next_thread->join();
                        }

                        handle_feedback::GetFeedbackResponse response;

                        this->updateFeedbackThread(
                                timestamp_stored_on_server,
                                feedback_type_object.feedback_type,
                                stop_token,
                                response, true);

                        emit signal_passNewVector(
                                feedback_type_object.feedback_type,
                                response,
                                RequestUpdateType::UPDATE_PREVIOUS_REQUESTED
                        );

                    }
            );

        } // else {} //currently, running next feedback, possible if user clicks next WHILE other is still running
    }
}

bool FeedbackWindow::checkSpecificFeedbackIndex(int feedback_index,
                                                const std::vector<handle_feedback::FeedbackElement>& feedback_list) {

    const QString LOADING_PROBABLY("Loading probably?...");

    if (feedback_index < 0) {
        ui->feedbackPlainTextEdit->setPlainText(LOADING_PROBABLY);
        return false;
    }

    if (feedback_index >= (int)feedback_list.size()) {
        QMessageBox::warning(this,
                             "Error",
                             ("index_other_feedback_list was out of range.\nindex_other_feedback_list:" +
                              std::to_string(feedback_index) +
                              "\nother_feedback_list.size(): " +
                              std::to_string(feedback_list.size())).c_str()
        );
        return false;
    }

    return true;
}

bool FeedbackWindow::makeSureCurrentFeedbackIndexIsValid() {

    bool return_value = false;

    runForAllFeedbackTypes([&](FeedbackTypeObject& feedback_object) {

        return_value = checkSpecificFeedbackIndex(
                feedback_object.current_index_in_feedback_list,
                feedback_object.feedback_list
        );
    });

    return return_value;
}

bool FeedbackWindow::runForAllFeedbackTypes(
        const std::function<void(FeedbackTypeObject& feedback_object)>& block
) {

    switch (current_feedback_type) {
        case FEEDBACK_TYPE_ACTIVITY_SUGGESTION: {
            block(activity_suggestion_feedback);
            break;
        }
        case FEEDBACK_TYPE_BUG_REPORT: {
            block(bug_report_feedback);
            break;
        }
        case FEEDBACK_TYPE_OTHER_FEEDBACK: {
            block(other_feedback);
            break;
        }
        case FEEDBACK_TYPE_UNKNOWN:
            ui->feedbackPlainTextEdit->setPlainText("Please select a feedback type.");
            return false;
        case FeedbackType_INT_MIN_SENTINEL_DO_NOT_USE_:
        case FeedbackType_INT_MAX_SENTINEL_DO_NOT_USE_:
            QMessageBox::warning(this,
                                 "Error",
                                 "Invalid feedback type selected inside runForAllFeedbackTypes().");
            return false;
    }

    return true;

}

