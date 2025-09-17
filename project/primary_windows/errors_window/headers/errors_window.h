//
// Created by jeremiah on 10/28/21.
//

#pragma once

#include <QWidget>
#include <QTreeWidgetItem>

#include <HandleErrors.grpc.pb.h>
#include <extract_errors.h>
#include <delete_single_error.h>
#include <set_error_to_handled.h>

#include "extract_errors_with_indexing.h"
#include "search_errors.h"

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class ErrorsWindow; }
QT_END_NAMESPACE

class ErrorsWindow : public QWidget {
Q_OBJECT

public:
    explicit ErrorsWindow(QWidget* parent = nullptr);

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    ~ErrorsWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:

    void on_extractErrorPushButton_clicked();

    void on_searchPushButton_clicked();

    void on_setErrorHandledPushButton_clicked();

    void on_filterPushButton_clicked();

    void on_deleteSinglePushButton_clicked();

    void on_searchDateAllCheckBox_toggled(bool checked);

    void on_searchDeviceNameAllCheckBox_toggled(bool checked);

    void on_lineNumberAllCheckBox_toggled(bool checked);

    void on_fileNameAllCheckBox_toggled(bool checked);

    void on_searchAndroidApiAllCheckBox_toggled(bool checked);

    void on_versionNumberAllCheckBox_toggled(bool checked);

    void on_filterDateAllCheckBox_toggled(bool checked);

    void on_filterAndroidApiAllCheckBox_toggled(bool checked);

    void on_filterDeviceNameAllCheckBox_toggled(bool checked);

    void slot_runSearchSuccessfullyCompleted(
            const handle_errors::SearchErrorsResponse& response,
            const std::function<void()>& completed_lambda = []() {}
    );

    void slot_extractErrorsSuccessfullyCompleted(
            ErrorOriginType origin,
            ErrorUrgencyLevel urgency_level,
            int version_number,
            const std::string& file_name,
            int line_number,
            const std::string& total_bytes,
            const std::string& extracted_bytes,
            const std::string& total_docs,
            const std::string& extracted_docs,
            std::shared_ptr<std::vector<ExtractedErrorsWithIndexing>> errors,
            const std::function<void()>& completed_lambda = []() {}
    );

    void slot_runDeleteSuccessfullyCompleted(
            const std::string& error_oid,
            const QModelIndex& model_index,
            const std::function<void()>& completed_lambda = [](){}
            );

    void slot_setErrorToHandledSuccessfullyCompleted(
            ErrorOriginType origin,
            unsigned int version_number,
            const std::string& file_name,
            unsigned int line_number,
            const QModelIndex& model_index,
            const std::function<void()>& completed_lambda = [](){}
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
    Ui::ErrorsWindow* ui;

    SearchErrors search_errors_obj;

    const std::string ERROR_ORIGIN_ = "ERROR_ORIGIN_";
    const std::string ERROR_URGENCY_LEVEL_ = "ERROR_URGENCY_LEVEL_";
    const std::string STACK_TRACE_DATA = "STACK_TRACE_DATA";
    const std::string ERROR_MESSAGE_DATA = "ERROR_MESSAGE_DATA";

    const int SEARCH_TREE_URGENCY_COL_NUM = 0;
    const int SEARCH_TREE_ORIGIN_COL_NUM = 1;
    const int SEARCH_TREE_NUM_OCCURRENCE_COL_NUM = 2;
    const int SEARCH_TREE_TIMESTAMP_COL_NUM = 3;
    const int SEARCH_TREE_VERSION_COL_NUM = 4;
    const int SEARCH_TREE_FILE_COL_NUM = 5;
    const int SEARCH_TREE_LINE_COL_NUM = 6;

    QString makeBytesLegible(const std::string& bytes);

    static QString makeBytesLegible(unsigned long bytes);

    static bool stringIsEmptyOrAllWhiteSpace(const std::string& str);

    static std::string createLegibleFileName(
            const std::string& file_name,
            int number_dash_to_extract_to = 1
    );

    void makeProtoEnumNameLegible(
            std::string& un_modified_name,
            const std::string& prefix
    );

    //assumes text_value has been converted to lower case already
    static bool searchIndexedTextForValue(
            const std::unordered_multimap<char, int>& indexing,
            const std::string& text_value,
            const std::string& text_to_search
            );

    void extractErrorParametersFromTreeWidgetItem(
            QTreeWidgetItem* item,
            handle_errors::ErrorParameters* error_parameters,
            const QString& error_string = "Error extracting '%1' from selected error."
            );

    //if bool == true function completed successfully
    //if QString.isNotEmpty() then a warning is expected to be displayed by the caller
    std::tuple<QString, bool> extractForCSVStringFields(
            const std::string& input_string,
            const std::function<bool(const std::string& /*stored_string*/)>& move_value
    );

    handle_errors::SearchErrorsResponse search_errors_response;
    ExtractErrorsObject extract_errors_object;
    DeleteSingleErrorObject delete_single_error_object;
    SetErrorToHandledObject set_error_to_handled_object;
    std::shared_ptr<std::vector<ExtractedErrorsWithIndexing>> extracted_errors =
            std::make_shared<std::vector<ExtractedErrorsWithIndexing>>();

    //expects a non, empty string consisting of more than whitespace
    std::tuple<bool, std::chrono::milliseconds> convertDateStringToLong(const std::string& date_input);

    void
    displayFullErrorMessageOnFilterTree(
            int extracted_error_index
            ) const;
};

