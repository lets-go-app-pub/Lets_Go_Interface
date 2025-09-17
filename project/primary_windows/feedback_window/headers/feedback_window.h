//
// Created by jeremiah on 8/30/21.
//

#pragma once

#include <QWidget>
#include <HandleFeedback.grpc.pb.h>
#include "thread"
#include "memory"

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class FeedbackWindow; }
QT_END_NAMESPACE

class FeedbackWindow : public QWidget {
Q_OBJECT

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    enum RequestUpdateType {
        NO_UPDATE_REQUESTED,
        UPDATE_NEXT_REQUESTED,
        UPDATE_PREVIOUS_REQUESTED
    };

    explicit FeedbackWindow(QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~FeedbackWindow() override;

private slots:

    void on_activityFeedbackButton_clicked();

    void on_otherFeedbackButton_clicked();

    void on_bugFeedbackButton_clicked();

    void on_feedbackNextButton_clicked();

    void on_feedbackPreviousButton_clicked();

    void on_feedbackSpamButton_clicked();

    void slot_updatedFeedbackUpdateVector(
            FeedbackType feedback_type,
            const handle_feedback::GetFeedbackResponse& response,
            FeedbackWindow::RequestUpdateType update_type = RequestUpdateType::NO_UPDATE_REQUESTED
    );

    void slot_displayWarning(const QString& title, const QString& text, const std::function<void()>& lambda = {});

    void slot_updateMostRecentlyViewedFeedbackTime(
            FeedbackType feedback_type,
            const std::chrono::milliseconds& new_timestamp
    );

    void slot_updateSetFeedbackToSpam(FeedbackType feedback_type, const std::string& feedback_oid);

signals:

    void signal_passNewVector(FeedbackType feedback_type,
                              const handle_feedback::GetFeedbackResponse& response,
                              FeedbackWindow::RequestUpdateType from_next);

    void signal_sendWarning(const QString& title, const QString& text, const std::function<void()>& lambda = {});

    void signal_updateMostRecentlyViewedFeedbackTime(FeedbackType feedback_type,
                                                     const std::chrono::milliseconds& new_timestamp);

    void signal_updateSetFeedbackToSpam(FeedbackType feedback_type, const std::string& feedback_oid);

private:

    const std::shared_ptr<std::jthread> null_pointer = nullptr;

    //20 elements and 2 caps
    const unsigned long MAX_SIZE_OF_FEEDBACK_TYPE_VECTOR = 20 + 2;
    const char* BEGINNING_OF_LIST_REACHED_TEMPLATE = "Beginning of '%1' user feedback.";
    const char* END_OF_LIST_REACHED_TEMPLATE = "End of '%1' user feedback.";

    struct FeedbackTypeObject {

    private:
        const unsigned int SIZE_FROM_END_OF_VECTOR_TO_REQUEST_NEW = 5;

    public:
        FeedbackType feedback_type;
        std::string feedback_english_name;

        //this bool is only checked and modified on the main thread, it doesn't need to be atomic
        bool requesting_next = false;
        //NOTE: std::atomic<std::shared_ptr<std::thread>> would allow me to drop the mutex and
        // should be allowed inside C++ 20 however I don't think g++ has implemented it yet when
        // writing this
        std::shared_ptr<std::jthread> next_update_thread = nullptr;

        //this bool is only checked and modified on the main thread, it doesn't need to be atomic
        bool requesting_previous = false;
        std::shared_ptr<std::jthread> previous_update_thread = nullptr;

        std::chrono::milliseconds most_recently_viewed_feedback_timestamp{};

        //set to -1 until the Initial request has been made
        unsigned int current_index_in_feedback_list = -1;

        //NOTE: only update this vector inside the main thread
        std::vector<handle_feedback::FeedbackElement> feedback_list;

        [[nodiscard]] const handle_feedback::FeedbackElement& getAtIndex() const {
            return feedback_list[current_index_in_feedback_list];
        }

        //in the list, will return the first index to request a 'next' updated to the list
        // example: ["a","b","c","d","e","f","g"] if it is in the final 5 elements it will
        // need to be requested, so 2 is returned
        // example: ["a", "b"] less than 5 elements so the first index is returned (0)
        [[nodiscard]] unsigned long getBeginningIndexToRequestNext() const {

            if (feedback_list.size() < SIZE_FROM_END_OF_VECTOR_TO_REQUEST_NEW) {
                return 0L;
            } else {
                //+ 1 is to account for size being 1 off of index
                return feedback_list.size() - (SIZE_FROM_END_OF_VECTOR_TO_REQUEST_NEW + 1);
            }
        }

        //in the list, will return the final index to request a 'previous' updated to the list
        // example: ["a","b","c","d","e","f","g"] if it is in the first 5 elements it will
        // need to be requested, so 4 is returned
        // example: ["a", "b"] less than 5 elements so the final index is returned (1)
        [[nodiscard]] unsigned long getFinalIndexToRequestPrevious() const {

            if (feedback_list.size() < SIZE_FROM_END_OF_VECTOR_TO_REQUEST_NEW) {
                return feedback_list.size() - 1;
            } else {
                //- 1 is to account for size being 1 off of index
                return SIZE_FROM_END_OF_VECTOR_TO_REQUEST_NEW - 1;
            }
        }

        [[nodiscard]] bool atEndOfVector() const {
            return current_index_in_feedback_list == (feedback_list.size() - 1);
        }

        [[nodiscard]] bool atBeginningOfVector() const {
            return current_index_in_feedback_list == 0;
        }

        [[nodiscard]] long lastTimestampInList() const {

            if(feedback_list.size() < 2) {
                return -1L;
            }

            return feedback_list[feedback_list.size() - 2].single_feedback_unit().timestamp_stored_on_server();
        }

        [[nodiscard]] long firstTimestampInList() const {
            if(feedback_list.size() < 2) {
                return -1L;
            }

            return feedback_list[1].single_feedback_unit().timestamp_stored_on_server();
        }

        void incrementIndex() {
            ++current_index_in_feedback_list;
        }

        void decrementIndex() {
            current_index_in_feedback_list--;
        }

        //NOTE: call this from the UI Thread for concurrency
        void updateTimestamp(const std::chrono::milliseconds& new_timestamp) {
            if (new_timestamp > most_recently_viewed_feedback_timestamp) {
                most_recently_viewed_feedback_timestamp = new_timestamp;
            }
        }
    };

    Ui::FeedbackWindow* ui = nullptr;

    HomeWindow* home_window_handle = nullptr;

    void requestInitialFeedbackFromServer(FeedbackType feedback_type);

    void showCurrentFeedback();

    void beginNavigationToNextFeedback(FeedbackTypeObject& feedback_type_object);

    void beginNavigationToPreviousFeedback(FeedbackTypeObject& feedback_type_object);

    void updateMostRecentlyViewedFeedback(
            FeedbackType feedback_type,
            const std::chrono::milliseconds& most_recently_observed_feedback_time
    );

    void setMarkingAsSpam(
            FeedbackType feedback_type,
            const std::string& feedback_oid,
            bool value
            );

    void setFeedbackToSpam(
            FeedbackType feedback_type,
            const std::string& feedback_oid
            );

    //this is built to be called from a separate thread
    void updateFeedbackThread(const long& timestamp_stored_on_server, FeedbackType feedback_type,
                              const std::stop_token& stop_token,
                              handle_feedback::GetFeedbackResponse& response,
                              bool request_before_timestamp);

    void saveInitialFeedbackToVector(
            FeedbackTypeObject& feedback_object,
            const handle_feedback::GetFeedbackResponse& response
    );

    void individualFeedbackDisplay(const FeedbackTypeObject& feedback_type_object,
                                   const std::string& feedback_message_text);

    bool checkSpecificFeedbackIndex(
            int feedback_index,
            const std::vector<handle_feedback::FeedbackElement>& feedback_list
    );

    void selectFeedbackButtonClicked(FeedbackTypeObject& feedback_object);

    bool makeSureCurrentFeedbackIndexIsValid();

    bool runForAllFeedbackTypes(const std::function<void(FeedbackTypeObject& feedback_object)>& block);

    void allButtonsSetEnabled(bool enabled);

    //this expects the list inside feedback_object to be at least size 2
    void setupButtonsBasedOnIndex(FeedbackTypeObject& feedback_object);

    //sets spam button to enabled or disabled based on the passed feedback element
    void setupSpamButton(const handle_feedback::FeedbackElement& feedback_element);

    FeedbackWindow::FeedbackTypeObject& getFeedbackTypeObject(FeedbackType feedback_type);

    //NOTE: these lists rely on the idea that the beginning and end should always have

    FeedbackTypeObject activity_suggestion_feedback;
    FeedbackTypeObject other_feedback;
    FeedbackTypeObject bug_report_feedback;

    FeedbackType current_feedback_type = FeedbackType::FEEDBACK_TYPE_UNKNOWN;

};

