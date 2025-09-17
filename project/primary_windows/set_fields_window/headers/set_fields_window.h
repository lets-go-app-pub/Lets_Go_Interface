//
// Created by jeremiah on 9/5/21.
//

#pragma once

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QLabel>

#include "request_server_icons.h"
#include "request_server_activities_categories.h"
#include "set_server_activity.h"
#include "set_server_category.h"
#include "set_server_icon.h"

#include "RequestFields.grpc.pb.h"

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class SetFieldsWindow; }
QT_END_NAMESPACE

class SetFieldsWindow : public QWidget {
Q_OBJECT

private:
    enum TreeDisplayingType {
        DISPLAYING_ACTIVITIES,
        DISPLAYING_CATEGORIES,
        DISPLAYING_ICONS
    };

protected:
    void closeEvent(QCloseEvent* event) override;

public:
    explicit SetFieldsWindow(QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~SetFieldsWindow() override;

private slots:
    void on_iconActiveCheckBox_toggled(bool checked);

    void on_colorCodeLineEdit_textChanged(const QString& new_text);

    void on_selectTypeCategoryRadioButton_toggled(bool checked);

    void on_selectTypeActivityRadioButton_toggled(bool checked);

    void on_selectTypeIconRadioButton_toggled(bool checked);

    void on_selectOperationUpdateRadioButton_toggled(bool checked);

    void on_selectOperationInsertRadioButton_toggled(bool checked);

    void on_viewFieldsActivitiesPushButton_clicked();

    void on_viewFieldsCategoriesPushButton_clicked();

    void on_viewFieldsIconsPushButton_clicked();

    void on_viewFieldsRefreshPushButton_clicked();

    void on_setFieldsSubmitPushButton_clicked();

    void on_basicIconPushButton_clicked();

    void slot_treeWidgetItemDoubleClicked(QTreeWidgetItem* item, int);

    //will be set to the icons_response
    void slot_requestCompleted(const std::function<void()>& completed_lambda = [](){});

    //this will be set to the error message if one occurs, it will return the passed error_lambda
    void slot_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = [](){}
    );

    //this will be called if the request was canceled
    void slot_requestCanceled(const std::function<void()>& completed_lambda = [](){});

private:
    HomeWindow* home_window_handle = nullptr;
    Ui::SetFieldsWindow* ui;

    TreeDisplayingType currently_displaying = TreeDisplayingType::DISPLAYING_ACTIVITIES;

    void setupUIForSelectTypeRadioButton();

    void setupUIForActivityTypeRadioButton();

    void setupUIForCategoryTypeRadioButton();

    void setupUIForIconTypeRadioButton();

    void setupListViewWidgetForActivities();

    void setupListViewWidgetForCategories();

    void setupListViewWidgetForIcons();

    void addQTreeWidgetItemForActivity(
            const std::string& icon_string,
            unsigned int icon_string_length,
            int activity_index,
            const QString& text
    );

    void addQTreeWidgetItemForCategory(const std::string& color_string,
                                       const QString& text,
                                       int category_index,
                                       int number_columns
    );

    void addQTreeWidgetItemForIcon(const request_fields::ServerIconsResponse& icons_message,
                                   unsigned long icon_index, const QString& text);

    void updateActivitiesCategoriesAndIcons();

    void refreshTreeWidget();

    void setUpUIForRequestingIconsCompleted();

    void setUpUIForRequestingActivitiesCategoriesCompleted();

    bool colorCodeIsValid(const std::string& color_code);

    void selectIconPressed(
            QLabel* image_label,
            QByteArray& bytes
    );

    const std::string CATEGORY = "Category";
    const std::string CATEGORY_LOWER = "category";
    const std::string ACTIVITY = "Activity";
    const std::string ACTIVITY_LOWER = "activity";

    const std::string SET_FIELDS_NAME_TITLE_MIDDLE_ONE = " Display Name (Try to make it short).\nIf name ";
    const std::string SET_FIELDS_NAME_TITLE_MIDDLE_TWO = ", it will update the\n";
    const std::string SET_FIELDS_NAME_TITLE_END = +" instead.";
    const std::string SET_FIELDS_MIN_AGE_TITLE = " Min Age";
    const std::string SET_FIELDS_DELETE_TEXT = "Disable ";

    RequestIconsObject request_icons_object;
    RequestActivitiesCategoriesObject request_activities_categories_object;
    SetServerActivity set_server_activities;
    SetServerCategory set_server_category;
    SetServerIcon set_server_icon;

    //set the refresh button to enabled when both of these are false
    bool requesting_icons = false;
    bool requesting_activities_categories = false;

    QByteArray basic_image_bytes;

    enum QTreeWidgetItemType {
        TREE_ITEM_TYPE_ACTIVITY,
        TREE_ITEM_TYPE_CATEGORY,
        TREE_ITEM_TYPE_ICON,
    };

    struct CategoriesIndexValues {
        int index = 0;
        double order_number = 0;

        CategoriesIndexValues(
                int index,
                double order_number
                ): index(index), order_number(order_number) {}

        bool operator<(const CategoriesIndexValues& rhs) const {
            return order_number < rhs.order_number;
        }
    };

};

