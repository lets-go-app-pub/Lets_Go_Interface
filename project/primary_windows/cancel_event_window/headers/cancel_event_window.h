//
// Created by jeremiah on 3/26/23.
//

#pragma once

#include <QWidget>
#include <QTreeWidgetItem>
#include "AdminEventCommands.pb.h"
#include "get_events.h"
#include "cancel_event.h"

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class CancelEventWindow; }
QT_END_NAMESPACE

class CancelEventWindow : public QWidget {
Q_OBJECT

public:
    explicit CancelEventWindow(QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~CancelEventWindow() override;

signals:

    void signal_findEvent(const std::string& event_oid);

    void signal_findUser(const std::string& event_oid);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:

    void on_searchPushButton_clicked();

    void on_cancelEventPushButton_clicked();

    void on_allEventsRadioButton_toggled(bool checked);

    void on_letsGoEventsOnlyRadioButton_toggled(bool checked);

    void on_userEventsOnlyRadioButton_toggled(bool checked);

    void on_specificUserRadioButton_toggled(bool checked);

    void on_anyCreatedTimeRadioButton_toggled(bool checked);

    void on_greaterThanCreatedTimeRadioButton_toggled(bool checked);

    void on_lessThanCreatedTimeRadioButton_toggled(bool checked);

    void on_anyExpirationTimeRadioButton_toggled(bool checked);

    void on_greaterThanExpirationTimeRadioButton_toggled(bool checked);

    void on_lessThanExpirationTimeRadioButton_toggled(bool checked);

    void slot_treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column);

    void slot_getEventsSuccessfullyCompleted(
            const admin_event_commands::GetEventsResponse& response,
            const std::function<void()>& completed_lambda
    );

    void slot_cancelEventSuccessfullyCompleted(
            const std::string& event_oid,
            const std::string& event_title,
            const std::function<void()>& completed_lambda
    );

    //this will be called if the request was canceled
    void slot_requestCanceled(const std::function<void()>& completed_lambda = []() {});

    void slot_displayWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& lambda = []() {}
    );

private:
    HomeWindow* home_window_handle = nullptr;
    Ui::CancelEventWindow* ui;

    GetEventsObject get_events_object;
    CancelEventObject cancel_event_object;

    enum class TypeOfDoubleClick {
        OPEN_EVENT,
        OPEN_USER
    };

    inline static const QString DOUBLE_CLICK_JSON_ENUM_KEY = "dbl_click_enum_key";
    inline static const QString DOUBLE_CLICK_JSON_STRING_KEY = "dbl_click_string_key";

    inline static const int EVENT_TITLE_COLUMN_NUM = 0;
    inline static const int CREATED_BY_COLUMN_NUM = 1;
    inline static const int CHAT_ROOM_ID_COLUMN_NUM = 2;
    inline static const int EVENT_TYPE_COLUMN_NUM = 3;
    inline static const int TIME_CREATED_COLUMN_NUM = 4;
    inline static const int EXPIRATION_TIME_COLUMN_NUM = 5;
    inline static const int EVENT_STATUS_COLUMN_NUM = 6;
    inline static const int NUMBER_SWIPED_YES_COLUMN_NUM = 7;
    inline static const int PERCENTAGE_SWIPED_YES_COLUMN_NUM = 8;

    inline static const QString NOT_ONGOING_EXPIRATION_TIME_PLACEHOLDER = "-";

    void connectInitialSignalsAndSlots();

    void setupInitialUIState();

    void setEventTypeSelectionLineEditVisible();

    void setCreatedTimeDateTimeEditVisible();

    void setExpirationTimeDateTimeEditVisible();

};
