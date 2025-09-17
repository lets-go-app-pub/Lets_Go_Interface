//
// Created by jeremiah on 9/8/21.
//

#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <thread>

#include <RequestUserAccountInfo.grpc.pb.h>
#include <StatusEnum.grpc.pb.h>
#include <QComboBox>
#include <dismiss_report.h>
#include <time_out_user.h>

#include "user_login_info.h"
#include "general_utility.h"


class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class DisplayUserWindow; }
QT_END_NAMESPACE

class DisplayUserWindow : public QWidget {
Q_OBJECT

public:
    explicit DisplayUserWindow(bool show_dismiss_report_button, QWidget* parent = nullptr);

    void findUser(
            const BsoncxxOIDWrapper& oid,
            bool request_event,
            const std::function<void()>& completed_lambda
    );

    void findUser(
            const std::string& phone_number,
            const std::function<void()>& completed_lambda
    );

    void setupGUI(CompleteUserAccountInfo* _user_account_response);

    std::string getCurrentDisplayedUserOID();

    void clearWidget(bool clear_user_info);

    ~DisplayUserWindow() override;

signals:

    //will be set to completed if successful
    void signal_requestSuccessfullyCompleted(const std::function<void()>& completed_lambda = []() {});

    //this will be set to the error message if one occurs
    void signal_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = []() {}
    );

    //this will be called if the request was canceled
    void signal_requestCanceled(const std::function<void()>& completed_lambda = []() {});

    void signal_internalCompleted(bool successfully_loaded);

    void signal_userAccessStatusCompleted(
            const std::string& current_user_account_id,
            bool success,
            const std::string& error_string,
            const std::chrono::milliseconds& current_timestamp = std::chrono::milliseconds{},
            const std::chrono::milliseconds& final_timestamp = std::chrono::milliseconds{},
            UserAccountStatus updated_access_status = UserAccountStatus(-1),
            const QString& reason = ""
    );

    void signal_userNameUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_name
    );

    void signal_userGenderUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_gender
    );

    void signal_userBirthdayUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            int updated_birth_year = -1,
            int updated_birth_month = -1,
            int updated_birth_day_of_month = -1,
            int updated_age = -1,
            int updated_min_age_range = -1,
            int updated_max_age_range = -1
    );

    void signal_userEmailUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_email = ""
    );

    void signal_userBioUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_bio = ""
    );

    void signal_userCityUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_city = ""
    );

    void signal_userGenderRangeUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::vector<std::string>& gender_ranges = {}
    );

    void signal_userAgeRangeUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            int updated_min_age = -1,
            int updated_max_age = -1
    );

    void signal_userMaxDistanceUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            int updated_max_distance = -1
    );

    void signal_handleReportCompleted(const std::string& reported_user_oid);

    void signal_findEvent(const std::string& event_oid);

private slots:

    void slot_internalCompleted(bool successfully_loaded);

    void slot_setUserAccessStatus(bool clicked);

    void slot_setUserFirstName(bool clicked);

    void slot_setUserGender(bool clicked);

    void slot_setUserBirthday(bool clicked);

    void slot_setUserEmail(bool clicked);

    void slot_setUserBio(bool clicked);

    void slot_setUserCity(bool clicked);

    void slot_setUserGenderRange(bool clicked);

    void slot_setUserAgeRange(bool clicked);

    void slot_setUserMaxDistance(bool clicked);

    void setupNewAccessStatus(
            const std::chrono::milliseconds& current_timestamp,
            const std::chrono::milliseconds& final_timestamp,
            UserAccountStatus updated_access_status,
            const QString& reason,
            DisciplinaryActionTypeEnum action_taken,
            QComboBox* combo_box
    );

    void slot_userAccessStatusCompleted(
            const std::string& current_user_account_id,
            bool success,
            const std::string& error_string,
            const std::chrono::milliseconds& current_timestamp,
            const std::chrono::milliseconds& final_timestamp,
            UserAccountStatus updated_access_status,
            const QString& reason
    );

    void slot_userNameUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_name
    );

    void slot_userGenderUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_gender
    );

    void slot_userBirthdayUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            int updated_birth_year,
            int updated_birth_month,
            int updated_birth_day_of_month,
            int updated_age,
            int updated_min_age_range,
            int updated_max_age_range
    );

    void slot_userEmailUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_email
    );

    void slot_userBioUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_bio
    );

    void slot_userCityUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::string& updated_city
    );

    void slot_userGenderRangeUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            const std::vector<std::string>& gender_ranges
    );

    void slot_userAgeRangeUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            int updated_min_age,
            int updated_max_age
    );

    void slot_userMaxDistanceUpdateCompleted(
            const std::string& current_user_account_id,
            ReturnStatus status,
            const std::string& error_string,
            int updated_max_distance
    );

    void slot_removePicture(QWidget* w, const std::string& picture_oid);

    void on_timeOutPushButton_clicked();

    void on_dismissReportPushButton_clicked();

    void updateInfoForSuccessfulTimeOut(
            const std::string& timed_out_user_oid,
            const std::string& reason,
            const handle_reports::TimeOutUserResponse& time_out_response
    );

    //will be set to the timeOutUser and dismissReport result
    void slot_handleTimeOutCompleted(
            const std::string& timed_out_user_oid,
            const std::string& reason,
            const handle_reports::TimeOutUserResponse& time_out_response
    );

    //will be set to the timeOutUser and dismissReport result
    void slot_handleDismissReportCompleted(const std::string& reported_user_oid);

    //will be set to the timeOutUser and dismissReport result
    void slot_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = []() {}
    );

    //will be set to the timeOutUser and dismissReport result
    void slot_handleReportRequestCanceled(const std::function<void()>& completed_lambda = []() {});

    void slot_treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    Ui::DisplayUserWindow* ui;

    void findUser(
            const RequestUserAccountInfoRequest& request,
            const std::function<void()>& completed_lambda
    );

    QTreeWidgetItem* setupSingleRowInTreeWidgetLineEdit(
            const std::string& label_constant,
            const QString& text,
            unsigned int max_chars,
            void (DisplayUserWindow::*func)(bool) = nullptr
    );

    void setupSingleRowInTreeWidgetSpinBox(
            const std::string& label_constant,
            int min_value,
            int max_value,
            int value,
            void (DisplayUserWindow::*func)(bool) = nullptr
    );

    [[nodiscard]] QWidget* generatePushButtonWidget(
            const std::string& label_constant,
            void (DisplayUserWindow::*func)(bool)
    ) const;

    void clearLayout(QLayout* layout);

    void setupGUI();

    //Only set/get this variable in the UI Thread.
    std::unique_ptr<std::jthread> function_thread;

    std::unique_ptr<CompleteUserAccountInfo> user_account_response = std::make_unique<CompleteUserAccountInfo>();

    //NOTE: During creating the label '_' will be turned into ' '.
    inline static const std::string ACCESS_STATUS_NAME = "Access_Status";
    inline static const std::string NUMBER_TIMES_TIMED_OUT_NAME = "Number_Times_Timed_Out";
    inline static const std::string INACTIVE_MESSAGE_NAME = "Inactive_Message";
    inline static const std::string INACTIVE_END_TIME_NAME = "Suspension_End_Time";
    inline static const std::string DISCIPLINARY_ACTIONS_RECORD = "Disciplinary_Actions_Record";
    inline static const std::string LAST_TIME_ACCOUNT_VERIFIED_NAME = "Last_Time_Account_Verified";
    inline static const std::string OPTED_IN_TO_PROMO_EMAIL_NAME = "Opted_In_To_Promo_Email";
    inline static const std::string TIME_ACCOUNT_CREATED = "Time_Account_Created";
    inline static const std::string ACCOUNT_ID_NAME = "Account_ID";
    inline static const std::string SUBSCRIPTION_STATUS_NAME = "Subscription_Status";
    inline static const std::string SUBSCRIPTION_EXPIRATION_TIME_NAME = "Subscription_Expiration_Time";
    inline static const std::string ACCOUNT_TYPE_NAME = "Account_Type";
    inline static const std::string PHONE_NUMBER_NAME = "Phone Number";
    inline static const std::string ALGORITHM_SEARCH_BY_NAME = "User Searching For Matches By";
    inline static const std::string USER_NAME_NAME = "Name";
    inline static const std::string GENDER_NAME = "Gender";
    inline static const std::string BIRTHDAY_NAME = "Birthday";
    inline static const std::string AGE_NAME = "Age";
    inline static const std::string EMAIL_ADDRESS_NAME = "Email_Address";
    inline static const std::string EMAIL_ADDRESS_VERIFIED_NAME = "Email_Address_Verified";
    inline static const std::string USER_ACTIVITIES_NAME = "User_Activities";
    inline static const std::string EVENT_ACTIVITIES_NAME = "Event_Activities";
    inline static const std::string BIO_NAME = "Bio";
    inline static const std::string DESCRIPTION_NAME = "Description";
    inline static const std::string CITY_NAME = "City";
    inline static const std::string GENDERS_MATCHING_WITH_NAME = "Genders_Matching_With";
    inline static const std::string AGE_RANGE_MATCHING_WITH_NAME = "Age_Range_Matching_With";
    inline static const std::string MAX_DISTANCE_MATCHING_WITH_NAME = "Max_Distance_Matching_With";
    inline static const std::string LOCATION_LATITUDE_NAME = "Location_Latitude";
    inline static const std::string LOCATION_LONGITUDE_NAME = "Location_Longitude";
    inline static const std::string LAST_TIME_ALGORITHM_ATTEMPTED_NAME = "Last_Time_Algorithm_Attempted";
    inline static const std::string CURRENT_NUM_OTHER_USERS_BLOCKED_NAME = "Current_Num_Other_Users_Blocked";
    inline static const std::string USER_CREATED_EVENTS_NAME = "User_Created_Events";
    inline static const std::string NUM_TIMES_SWIPED_YES_NAME = "Num_Times_Swiped_Yes";
    inline static const std::string NUM_TIMES_SWIPED_NO_NAME = "Num_Times_Swiped_No";
    inline static const std::string NUM_TIMES_SWIPED_BLOCK_AND_REPORT_NAME = "Num_Times_Swiped_Block_And_Report";
    inline static const std::string NUM_TIMES_SWIPED_YES_ON_BY_OTHERS_NAME = "Num_Times_Swiped_Yes_On_By_Others";
    inline static const std::string NUM_TIMES_SWIPED_NO_ON_BY_OTHERS_NAME = "Num_Times_Swiped_No_On_By_Others";
    inline static const std::string NUM_TIMES_SWIPED_BLOCK_AND_REPORT_ON_BY_OTHERS_NAME = "Num_Times_Swiped_Block_And_Report_On_By_Others";
    inline static const std::string NUM_TIMES_SENT_ACTIVITY_SUGGESTION_NAME = "Num_Times_Sent_Activity_Suggestion";
    inline static const std::string NUM_TIMES_SENT_BUG_REPORT_NAME = "Num_Times_Sent_Bug_Report";
    inline static const std::string NUM_TIMES_SENT_OTHER_SUGGESTION_NAME = "Num_Times_Sent_Other_Suggestion";
    inline static const std::string NUM_TIMES_SPAM_FEEDBACK_SENT_NAME = "Num_Times_Spam_Feedback_Sent";
    inline static const std::string NUM_TIMES_SPAM_REPORT_SENT_NAME = "Num_Times_Spam_Reports_Sent";
    inline static const std::string NUM_TIMES_BLOCK_AND_REPORT_USED_IN_CHAT_NAME = "Num_Times_Block_And_Report_Used_In_Chat";
    inline static const std::string NUM_TIMES_BLOCK_AND_REPORT_USED_ON_BY_OTHERS_IN_CHAT_NAME = "Num_Times_Block_And_Report_Used_On_By_Others_In_Chat";

    inline static const std::string EVENT_TITLE_NAME = "Event_Title";
    inline static const std::string EVENT_CREATED_BY_NAME = "Event_Created_By";
    inline static const std::string EVENT_CHAT_ROOM_ID_NAME = "Event_Chat_Room_ID";
    inline static const std::string EVENT_EXPIRATION_TIME_NAME = "Event_Expiration_Time";
    inline static const std::string EVENT_STATUS_NAME = "Event_Status";

    inline static const QString ACCOUNT_STATUS_ACTIVE = "ACTIVE";
    inline static const QString ACCOUNT_STATUS_REQUIRES_INFO = "REQUIRES MORE INFO";
    inline static const QString ACCOUNT_STATUS_SUSPENDED = "SUSPENDED";
    inline static const QString ACCOUNT_STATUS_BANNED = "BANNED";

    inline static const QString DOUBLE_CLICK_JSON_ENUM_KEY = "dbl_click_enum_key";
    inline static const QString DOUBLE_CLICK_JSON_OID_KEY = "dbl_click_oid_key";

    enum class TypeOfDoubleClick {
        USER_CREATED_EVENT
    };

    //storing jthread objects inside the class so that the jthread itself will
    // always have a valid reference to the class (inside the thread 'this' is passed)
    //NOTE: only modify these from the main thread
    //these objects guarantee only 1 thread per user ID
    //key = user ID; value = thread
    std::map<std::string, std::jthread> set_user_access_status{};
    std::map<std::string, std::jthread> set_user_name_threads{};
    std::map<std::string, std::jthread> set_user_gender{};
    std::map<std::string, std::jthread> set_user_birthday{};
    std::map<std::string, std::jthread> set_user_email{};
    std::map<std::string, std::jthread> set_user_bio{};
    std::map<std::string, std::jthread> set_user_city{};
    std::map<std::string, std::jthread> set_user_gender_range{};
    std::map<std::string, std::jthread> set_user_age_range{};
    std::map<std::string, std::jthread> set_user_max_distance{};

    void displayDisciplinaryActionInItem(
            QTreeWidgetItem* disciplinary_actions_item,
            const UserDisciplinaryAction& action,
            int index) const;

    void displayUserCreatedEventInItem(
            QTreeWidgetItem* user_created_event_item,
            const UserCreatedEvent& event_message
    ) const;

    void setActiveStatusToComboBox(QComboBox* combo_box);

    DismissReport dismiss_report;
    TimeOutUser time_out_user;

    bool current_user_can_be_updated = false;
};

