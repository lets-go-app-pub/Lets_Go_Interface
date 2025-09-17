//
// Created by jeremiah on 10/28/21.
//

#include <general_utility.h>
#include <QMessageBox>
#include <sstream>
#include <regex>
#include <utility>
#include <user_login_info.h>
#include <access_status_invalid_message_dialog.h>
#include "errors_window.h"
#include "ui_errors_window.h"
#include "homewindow.h"

ErrorsWindow::ErrorsWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::ErrorsWindow) {
    ui->setupUi(this);

    ui->searchOptionsTreeWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui->filterOptionsTreeWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);

    connect(&search_errors_obj, &SearchErrors::signal_sendWarning, this,
            &ErrorsWindow::slot_displayWarning);
    connect(&search_errors_obj, &SearchErrors::signal_requestCanceled, this,
            &ErrorsWindow::slot_requestCanceled);
    connect(&search_errors_obj, &SearchErrors::signal_runSearchSuccessfullyCompleted, this,
            &ErrorsWindow::slot_runSearchSuccessfullyCompleted);

    connect(&extract_errors_object, &ExtractErrorsObject::signal_sendWarning, this,
            &ErrorsWindow::slot_displayWarning);
    connect(&extract_errors_object, &ExtractErrorsObject::signal_requestCanceled, this,
            &ErrorsWindow::slot_requestCanceled);
    connect(&extract_errors_object, &ExtractErrorsObject::signal_extractErrorsSuccessfullyCompleted, this,
            &ErrorsWindow::slot_extractErrorsSuccessfullyCompleted);

    connect(&delete_single_error_object, &DeleteSingleErrorObject::signal_sendWarning, this,
            &ErrorsWindow::slot_displayWarning);
    connect(&delete_single_error_object, &DeleteSingleErrorObject::signal_requestCanceled, this,
            &ErrorsWindow::slot_requestCanceled);
    connect(&delete_single_error_object, &DeleteSingleErrorObject::signal_runDeleteSuccessfullyCompleted, this,
            &ErrorsWindow::slot_runDeleteSuccessfullyCompleted);

    connect(&set_error_to_handled_object, &SetErrorToHandledObject::signal_sendWarning, this,
            &ErrorsWindow::slot_displayWarning);
    connect(&set_error_to_handled_object, &SetErrorToHandledObject::signal_requestCanceled, this,
            &ErrorsWindow::slot_requestCanceled);
    connect(&set_error_to_handled_object, &SetErrorToHandledObject::signal_setErrorToHandledSuccessfullyCompleted, this,
            &ErrorsWindow::slot_setErrorToHandledSuccessfullyCompleted);

    ui->searchOptionsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->searchOptionsTreeWidget->setUniformRowHeights(false);

    ui->filterOptionsTreeWidget->setVerticalScrollMode(QTreeView::ScrollPerPixel);
    ui->filterOptionsTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->filterOptionsTreeWidget->setUniformRowHeights(false);

    ui->filterTreeDocCountNoteLabel->setVisible(false);
    ui->filterTreeMaxDocCountNoteLabel->setVisible(false);

}

void ErrorsWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

ErrorsWindow::~ErrorsWindow() {
    delete ui;
}

void ErrorsWindow::on_filterDeviceNameAllCheckBox_toggled(bool checked) {
    ui->filterDeviceNameInputLineEdit->setEnabled(!checked);
}

void ErrorsWindow::on_filterAndroidApiAllCheckBox_toggled(bool checked) {
    ui->filterAndroidApiMinSpinBox->setEnabled(!checked);
    ui->filterAndroidApiMaxSpinBox->setEnabled(!checked);
}

void ErrorsWindow::on_filterDateAllCheckBox_toggled(bool checked) {
    ui->filterDateBeforeRadioButton->setEnabled(!checked);
    ui->filterDateAfterRadioButton->setEnabled(!checked);
    ui->filterDateInputLineEdit->setEnabled(!checked);
}

void ErrorsWindow::on_versionNumberAllCheckBox_toggled(bool checked) {
    ui->versionNumberMaxSpinBox->setEnabled(!checked);
    ui->versionNumberMinSpinBox->setEnabled(!checked);
}

void ErrorsWindow::on_searchAndroidApiAllCheckBox_toggled(bool checked) {
    ui->searchAndroidApiMinSpinBox->setEnabled(!checked);
    ui->searchAndroidApiMaxSpinBox->setEnabled(!checked);
}

void ErrorsWindow::on_fileNameAllCheckBox_toggled(bool checked) {
    ui->fileNameInputLineEdit->setEnabled(!checked);
}

void ErrorsWindow::on_lineNumberAllCheckBox_toggled(bool checked) {
    ui->lineNumberInputLineEdit->setEnabled(!checked);
}

void ErrorsWindow::on_searchDeviceNameAllCheckBox_toggled(bool checked) {
    ui->searchDeviceNameInputLineEdit->setEnabled(!checked);
}

void ErrorsWindow::on_searchDateAllCheckBox_toggled(bool checked) {
    ui->searchDateBeforeRadioButton->setEnabled(!checked);
    ui->searchDateAfterRadioButton->setEnabled(!checked);
    ui->searchDateInputLineEdit->setEnabled(!checked);
}

void ErrorsWindow::makeProtoEnumNameLegible(
        std::string& un_modified_name,
        const std::string& prefix
) {
    if (un_modified_name.starts_with(prefix)) {
        un_modified_name.erase(std::begin(un_modified_name), std::begin(un_modified_name) + (int) prefix.size());
        std::transform(std::begin(un_modified_name), std::end(un_modified_name), std::begin(un_modified_name),
                       [](char c) -> char { return c == '_' ? ' ' : c; });
    }
}

void ErrorsWindow::slot_runSearchSuccessfullyCompleted(
        const handle_errors::SearchErrorsResponse& response,
        const std::function<void()>& completed_lambda
) {

    if (completed_lambda) {
        completed_lambda();
    }

    if (!response.success()) {
        slot_displayWarning(
                "Search Error",
                "Run search returned 'completed' however when the response came back success == false."
        );
        return;
    }

    ui->searchOptionsTreeWidget->clear();

    search_errors_response = response;

    std::sort(
            search_errors_response.mutable_error_message_statistics()->begin(),
            search_errors_response.mutable_error_message_statistics()->end(),
            [](const handle_errors::ErrorStatistics& lhs, const handle_errors::ErrorStatistics& rhs) -> bool {

                if (lhs.error_urgency_level() == rhs.error_urgency_level()) {
                    return lhs.number_times_error_occurred() > rhs.number_times_error_occurred();
                } else if (lhs.error_urgency_level() == ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_UNKNOWN) {
                    return false;
                } else if (rhs.error_urgency_level() == ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_UNKNOWN) {
                    return true;
                }

                return lhs.error_urgency_level() < rhs.error_urgency_level();
            }
    );

    for (const auto& error_message: search_errors_response.error_message_statistics()) {
        auto* item = new QTreeWidgetItem(ui->searchOptionsTreeWidget);

        std::string urgency_level = ErrorUrgencyLevel_Name(error_message.error_urgency_level());

        makeProtoEnumNameLegible(
                urgency_level,
                ERROR_URGENCY_LEVEL_
        );

        item->setText(SEARCH_TREE_URGENCY_COL_NUM, QString::fromStdString(urgency_level));
        item->setData(SEARCH_TREE_URGENCY_COL_NUM, Qt::UserRole, (int) error_message.error_urgency_level());

        std::string origin = ErrorOriginType_Name(error_message.error_origin());

        makeProtoEnumNameLegible(
                origin,
                ERROR_ORIGIN_
        );

        item->setText(SEARCH_TREE_ORIGIN_COL_NUM, QString::fromStdString(origin));
        item->setData(SEARCH_TREE_ORIGIN_COL_NUM, Qt::UserRole, (int) error_message.error_origin());

        item->setText(SEARCH_TREE_NUM_OCCURRENCE_COL_NUM, QString::number(error_message.number_times_error_occurred()));

        item->setText(SEARCH_TREE_TIMESTAMP_COL_NUM,
                      QString::fromStdString(
                              getDateTimeStringFromTimestamp(
                                      std::chrono::milliseconds{error_message.most_recent_timestamp()}
                              )
                      )
        );

        item->setText(SEARCH_TREE_VERSION_COL_NUM, QString::number(error_message.version_number()));

        item->setText(SEARCH_TREE_FILE_COL_NUM,
                      QString::fromStdString(createLegibleFileName(error_message.file_name(), 2)));
        item->setData(SEARCH_TREE_FILE_COL_NUM, Qt::UserRole, QString::fromStdString(error_message.file_name()));

        item->setText(SEARCH_TREE_LINE_COL_NUM, QString::number(error_message.line_number()));
    }

    ui->searchOptionsTreeWidget->viewport()->update();
}

std::string ErrorsWindow::createLegibleFileName(
        const std::string& file_name,
        const int number_dash_to_extract_to
) {

    std::string temp_file_name = file_name;
    int number_found = 0;

    temp_file_name.erase(temp_file_name.begin(),
                         std::find_if(temp_file_name.rbegin(), temp_file_name.rend(),
                                      [&number_found, number_dash_to_extract_to](char c) -> bool {
                                          if (c == '\\' || c == '/') {
                                              number_found++;
                                          }
                                          return number_found >= number_dash_to_extract_to;
                                      }).base());

    return temp_file_name;
}

void ErrorsWindow::slot_extractErrorsSuccessfullyCompleted(
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
        const std::function<void()>& completed_lambda
) {

    if (completed_lambda) {
        completed_lambda();
    }

    extracted_errors = std::move(errors);

    std::sort(extracted_errors->begin(), extracted_errors->end(), [](
            const ExtractedErrorsWithIndexing& lhs, const ExtractedErrorsWithIndexing& rhs
    ) -> bool {
        return lhs.message.error_message().timestamp_stored() > rhs.message.error_message().timestamp_stored();
    });

    ui->filterOptionsTreeWidget->clear();

    QString tree_top_item_left_string;
    QString tree_top_item_right_string;

    std::string urgency_level_str = ErrorUrgencyLevel_Name(urgency_level);

    makeProtoEnumNameLegible(
            urgency_level_str,
            ERROR_URGENCY_LEVEL_
    );

    tree_top_item_left_string += QString("Urgency: %1").arg(QString::fromStdString(urgency_level_str));

    std::string origin_str = ErrorOriginType_Name(origin);

    makeProtoEnumNameLegible(
            origin_str,
            ERROR_ORIGIN_
    );

    tree_top_item_right_string += QString("Origin: %1").arg(QString::fromStdString(origin_str));

    tree_top_item_left_string += QString("\nVersion: %1").arg(QString::number(version_number));

    tree_top_item_right_string += QString("\nFile: %1").arg(QString::fromStdString(createLegibleFileName(file_name)));

    tree_top_item_left_string += QString("\nLine: %1").arg(QString::number(line_number));

    ui->filterTreeTitleLeftLabel->setText(tree_top_item_left_string);
    ui->filterTreeTitleRightLabel->setText(tree_top_item_right_string);

    ui->filterTreeDocCountLeftLabel->setText(
            QString("Total Docs: %1\nTotal Bytes: %2")
                    .arg(QString::fromStdString(total_docs),
                         makeBytesLegible(total_bytes))
    );
    ui->filterTreeDocCountRightLabel->setText(
            QString("Extracted Docs: %1\nExtracted Bytes: %2")
                    .arg(QString::fromStdString(extracted_docs),
                         makeBytesLegible(extracted_bytes))
    );

    try {
        long total_docs_long = std::stol(total_docs);
        long extracted_docs_long = std::stol(extracted_docs);

        ui->filterTreeDocCountNoteLabel->setVisible(total_docs_long > extracted_docs_long);
        ui->filterTreeMaxDocCountNoteLabel->setVisible(total_docs_long >= globals->extract_errors_max_search_results());
    } catch (const std::out_of_range& e) {
        slot_displayWarning(
                "Error",
                QString("A string being converted into a long was out_of_range.\ntotal_docs: %1\nextracted_docs: %2\nexception: %3")
                        .arg(QString::fromStdString(total_docs), QString::fromStdString(extracted_docs), e.what())
        );
        return;
    } catch (const std::invalid_argument& e) {
        slot_displayWarning(
                "Error",
                QString("A string being converted into a long was an invalid_argument.\ntotal_docs: %1\nextracted_docs: %2\nexception: %3")
                        .arg(QString::fromStdString(total_docs), QString::fromStdString(extracted_docs), e.what())
        );
        return;
    }

    auto header_item = ui->filterOptionsTreeWidget->headerItem();

    if (origin == ErrorOriginType::ERROR_ORIGIN_ANDROID) {
        header_item->setText(0, "Timestamp; API Number; Device Name");
    } else {
        header_item->setText(0, "Timestamp");
    }

    for (const auto& extracted_error: *extracted_errors) {
        displayFullErrorMessageOnFilterTree((int) (&extracted_error - &(*extracted_errors)[0]));
    }

    ui->filterOptionsTreeWidget->viewport()->update();
}

void ErrorsWindow::slot_runDeleteSuccessfullyCompleted(
        const std::string& error_oid,
        const QModelIndex& model_index,
        const std::function<void()>& completed_lambda
) {

    if (completed_lambda) {
        completed_lambda();
    }

    QTreeWidgetItem* item = ui->filterOptionsTreeWidget->itemFromIndex(model_index);

    //remove item from filter tree if exists
    if (item != nullptr) {
        std::string extracted_error_oid = item->data(0, Qt::UserRole).toString().toStdString();

        if (error_oid == extracted_error_oid) {
            auto taken_item = ui->filterOptionsTreeWidget->takeTopLevelItem(model_index.row());
            delete taken_item;
        }
    }

    //remove item from vector list if exists
    auto it = (*extracted_errors).begin();
    while (it != (*extracted_errors).end()) {
        if ((*it).message.error_message().error_id() == error_oid) {
            (*extracted_errors).erase(it);
            break;
        }
        it++;
    }

    //NOTE: not updating the searchOptionsTreeWidget count, shouldn't really matter

    ui->filterOptionsTreeWidget->viewport()->update();
}

void ErrorsWindow::slot_setErrorToHandledSuccessfullyCompleted(
        ErrorOriginType origin,
        unsigned int version_number,
        const std::string& file_name,
        unsigned int line_number,
        const QModelIndex& model_index,
        const std::function<void()>& completed_lambda
) {

    if (completed_lambda) {
        completed_lambda();
    }

    QTreeWidgetItem* item = ui->searchOptionsTreeWidget->itemFromIndex(model_index);

    //remove item from filter tree if exists
    if (item != nullptr) {

        handle_errors::ErrorParameters error_parameters;

        extractErrorParametersFromTreeWidgetItem(
                item,
                &error_parameters
        );

        if (origin == error_parameters.error_origin()
            && version_number == error_parameters.version_number()
            && file_name == error_parameters.file_name()
            && line_number == error_parameters.line_number()
                ) {
            auto taken_item = ui->searchOptionsTreeWidget->takeTopLevelItem(model_index.row());
            delete taken_item;
        }
    }

    //remove item from list of searched errors
    auto it = search_errors_response.mutable_error_message_statistics()->begin();
    while (it != search_errors_response.mutable_error_message_statistics()->end()) {

        if (origin == (*it).error_origin()
            && version_number == (*it).version_number()
            && file_name == (*it).file_name()
            && line_number == (*it).line_number()
                ) {
            search_errors_response.mutable_error_message_statistics()->erase(it);
            break;
        }
        it++;
    }

    ui->searchOptionsTreeWidget->viewport()->update();
}

void ErrorsWindow::displayFullErrorMessageOnFilterTree(
        int extracted_error_index
) const {
    QString error_top_text;
    const auto origin = ErrorOriginType(
            (*extracted_errors)[extracted_error_index].message.error_message().error_origin());

    error_top_text = QString::fromStdString(
            getDateTimeStringFromTimestamp(
                    std::chrono::milliseconds{
                            (*extracted_errors)[extracted_error_index].message.error_message().timestamp_stored()}
            )
    );

    if (origin == ERROR_ORIGIN_ANDROID) {
        error_top_text += QString(";  {%1};  {%2}")
                .arg(
                        QString::number(
                                (*extracted_errors)[extracted_error_index].message.error_message().api_number()),
                        QString::fromStdString(
                                (*extracted_errors)[extracted_error_index].message.error_message().device_name())
                );
    }

    auto* item = new QTreeWidgetItem(ui->filterOptionsTreeWidget);

    //putting all top text in 1 column because otherwise when the child items are expanded
    // the auto formatting will mess up the look of it
    item->setText(0, error_top_text);

    item->setData(0, Qt::UserRole, QString::fromStdString(
            (*extracted_errors)[extracted_error_index].message.error_message().error_id()));

    auto* error_message_item = new QTreeWidgetItem(item);

    error_message_item->setText(0, QString::fromStdString(
            (*extracted_errors)[extracted_error_index].message.error_message().error_message()));

    error_message_item->setData(0, Qt::UserRole, QString::fromStdString(ERROR_MESSAGE_DATA));
    if (!(*extracted_errors)[extracted_error_index].message.error_message().stack_trace().empty()) {
        auto* stack_trace_item = new QTreeWidgetItem(item);

        stack_trace_item->setText(0, QString("Trace:\n%1").arg(
                QString::fromStdString(
                        (*extracted_errors)[extracted_error_index].message.error_message().stack_trace())));

        stack_trace_item->setData(0, Qt::UserRole, QString::fromStdString(STACK_TRACE_DATA));
    }
}

void ErrorsWindow::slot_requestCanceled(const std::function<void()>& completed_lambda) {
    if (completed_lambda) {
        completed_lambda();
    }
}

void ErrorsWindow::slot_displayWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& lambda
) {

    if (lambda) {
        lambda();
    }

    std::cout << "title: " << title.toStdString() << '\n';
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(
            this,
            title,
            text
    );
}

struct DateStruct {

    //2^12 = 4096
    //actual year
    int year: 13 = 0;

    //2^4 = 16
    //follow 1-12
    int month: 5 = 0;

    //2^5 = 32
    //follows 1-31
    int day: 6 = 0;

    //2^5 = 32
    //1-24
    int hours: 6 = 0;

    //2^6 = 64
    //0-61
    int minutes: 7 = 0;
};

std::tuple<QString, bool> ErrorsWindow::extractForCSVStringFields(
        const std::string& input_string,
        const std::function<bool(const std::string& /*stored_string*/)>& move_value
) {

    std::set<std::string> no_duplicates;
    std::istringstream ss{input_string};

    std::string temp;
    while (std::getline(ss, temp, ',')) {

        if (temp.size() > globals->maximum_number_allowed_bytes()) {
            return {"Individual %1 string too long for server to accept.", false};
        }

        auto checking = [](char c) -> bool {
            return !isspace(c);
        };

        //remove leading whitespace
        temp.erase(std::begin(temp), std::find_if(std::begin(temp), std::end(temp), checking));

        //remove trailing whitespace
        temp.erase(std::find_if(temp.rbegin(), temp.rend(), checking).base(), temp.end());

        if (temp.empty()) {
            continue;
        }

        no_duplicates.insert(std::move(temp));

        if (no_duplicates.size() > 1000) {
            return {"%1 has too many values for the server to accept", false};
        }
    }

    for (const auto& str: no_duplicates) {
        bool return_value = move_value(str);

        if (!return_value) {
            return {"", false};
        }
    }

    return {"", true};
}

bool ErrorsWindow::stringIsEmptyOrAllWhiteSpace(const std::string& str) {
    return str.find_first_not_of(" \t\n\v\f\r") == std::string::npos;
}

QString ErrorsWindow::makeBytesLegible(const std::string& bytes) {
    try {
        return makeBytesLegible(std::stoul(bytes));
    } catch (const std::out_of_range& e) {
        slot_displayWarning(
                "Error",
                QString("Bytes string %1 could was too large.").arg(QString::fromStdString(bytes))
        );
        return "INVALID";
    } catch (const std::invalid_argument& e) {
        slot_displayWarning(
                "Error",
                QString("Bytes string %1 was not a valid number.").arg(QString::fromStdString(bytes))
        );
        return "INVALID";
    }

}

QString ErrorsWindow::makeBytesLegible(unsigned long bytes) {

    QString string;

    unsigned long kilo_bytes = bytes / 1024L, mega_bytes = kilo_bytes / 1024L, giga_bytes = mega_bytes / 1024L;
    QString space;

    if (giga_bytes > 0) {
        string += QString("%1 Gb").arg(QString::number(giga_bytes));

        mega_bytes %= 1024L;
        kilo_bytes %= 1024L;
        space = " ";
    }

    if (mega_bytes > 0) {
        string += QString("%1%2 Mb")
                .arg(
                        space,
                        QString::number(mega_bytes)
                );

        kilo_bytes %= 1024L;
        space = " ";
    }

    if (kilo_bytes > 0) {

        QString bytes_string;

        if (string.isEmpty()) {
            bytes_string = QString(" %1 Bytes").arg(bytes % 1024);
        }

        string += QString("%1%2 Kb%3")
                .arg(
                        space,
                        QString::number(kilo_bytes),
                        bytes_string
                );
    }

    if (string.isEmpty()) {
        string = QString("%1 Bytes").arg(QString::number(bytes));
    }

    return string;
}

void ErrorsWindow::extractErrorParametersFromTreeWidgetItem(
        QTreeWidgetItem* item,
        handle_errors::ErrorParameters* error_parameters,
        const QString& error_string
) {

    ErrorOriginType error_origin;
    int version_number = -1;
    std::string file_name;
    int line_number;

    bool flag;
    error_origin = ErrorOriginType(item->data(SEARCH_TREE_ORIGIN_COL_NUM, Qt::UserRole).toInt(&flag));

    if (!flag) {
        slot_displayWarning(
                "Error",
                error_string.arg("Error Origin")
        );
        return;
    }

    version_number = item->text(SEARCH_TREE_VERSION_COL_NUM).toInt(&flag);

    if (!flag) {
        slot_displayWarning(
                "Error",
                error_string.arg("Version Number")
        );
        return;
    }

    file_name = item->data(SEARCH_TREE_FILE_COL_NUM, Qt::UserRole).toString().toStdString();

    line_number = item->text(SEARCH_TREE_LINE_COL_NUM).toInt(&flag);

    if (!flag) {
        slot_displayWarning(
                "Error",
                error_string.arg("Version Number")
        );
        return;
    }

    error_parameters->set_error_origin(error_origin);
    error_parameters->set_version_number(version_number);
    error_parameters->set_file_name(file_name);
    error_parameters->set_line_number(line_number);
}

void ErrorsWindow::on_extractErrorPushButton_clicked() {
    auto selected_items = ui->searchOptionsTreeWidget->selectedItems();

    auto* error_parameters = new handle_errors::ErrorParameters();
    ErrorUrgencyLevel urgency_level = ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_UNKNOWN;
    long max_number_bytes = 0;

    QString error_string = "Error extracting '%1' from selected error.";

    //there should only be 1 item in this object because the tree selection mode is QAbstractItemView::SelectionMode::SingleSelection
    for (const auto& item: selected_items) {

        extractErrorParametersFromTreeWidgetItem(item, error_parameters, error_string);

        bool flag;

        urgency_level = ErrorUrgencyLevel(item->data(SEARCH_TREE_URGENCY_COL_NUM, Qt::UserRole).toInt(&flag));

        if (!flag) {
            slot_displayWarning(
                    "Error",
                    error_string.arg("Urgency Level")
            );
            delete error_parameters;
            return;
        }

        std::string max_bytes_string = ui->maxBytesInputLineEdit->text().toStdString();

        std::string max_bytes_numbers;
        std::string max_bytes_letters;

        for (unsigned char c: max_bytes_string) {
            if (std::isalpha(c)) {
                max_bytes_letters.push_back(static_cast<char>(std::tolower(c)));
            } else if (std::isdigit(c)) {
                max_bytes_numbers.push_back(static_cast<char>(c));
            }
        }

        if (max_bytes_numbers.empty()) {
            slot_displayWarning(
                    "Error",
                    "Please enter a number for 'Max Bytes'."
            );
            delete error_parameters;
            return;
        }

        try {
            if (max_bytes_letters.empty()) {
                max_number_bytes = std::stol(max_bytes_numbers);
            } else if (max_bytes_letters == "kb") {
                max_number_bytes = 1024L * std::stol(max_bytes_numbers);
            } else if (max_bytes_letters == "mb") {
                max_number_bytes = 1024L * 1024L * std::stol(max_bytes_numbers);
            } else if (max_bytes_letters == "gb") {
                max_number_bytes = 1024L * 1024L * 1024L * std::stol(max_bytes_numbers);
            } else {
                slot_displayWarning(
                        "Error",
                        "Please enter a valid suffix for 'Max Bytes' ('Gb', 'Mb', 'Kb' or '')."
                );
                delete error_parameters;
                return;
            }
        } catch (const std::out_of_range& e) {
            slot_displayWarning(
                    "Error",
                    "Please enter a valid number for 'Max Bytes' (number is too large to store in a long)."
            );
            delete error_parameters;
            return;
        } catch (const std::invalid_argument& e) {
            slot_displayWarning(
                    "Error",
                    "Please enter a valid number for 'Max Bytes' (invalid number)."
            );
            delete error_parameters;
            return;
        }

        if (max_number_bytes > (long) globals->maximum_number_allowed_bytes_to_request_error_message()) {

            slot_displayWarning(
                    "Error",
                    QString("Please enter a valid number for 'Max Bytes' max size is %1.")
                            .arg(makeBytesLegible(globals->maximum_number_allowed_bytes_to_request_error_message()))
            );
            delete error_parameters;
            return;
        } else if (max_number_bytes > 1024L * 1024L * 100L) { //greater than 100Mb
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Warning",
                                          "MongoDB performance suffers when over 100Mb is accessed. Continue anyways?",
                                          QMessageBox::Ok | QMessageBox::Cancel);

            if (reply == QMessageBox::Cancel) {
                delete error_parameters;
                return;
            }
        }
    }

    if (error_parameters->version_number() == 0) {
        //values not set
        delete error_parameters;
        return;
    }

    ui->extractErrorPushButton->setEnabled(false);

    //NOTE: extractErrors() will take ownership of the error_parameters pointer
    extract_errors_object.extractErrors(
            urgency_level,
            max_number_bytes,
            error_parameters,
            [&]() {
                ui->extractErrorPushButton->setEnabled(true);
            });

}

void ErrorsWindow::on_searchPushButton_clicked() {
    handle_errors::SearchErrorsRequest request;

    if (ui->serverCheckBox->isChecked()
        && ui->androidCheckBox->isChecked()
        && ui->desktopInterfaceCheckBox->isChecked()
        && ui->webServerCheckBox->isChecked()
            ) {
        request.set_all_error_origin_types(true);
    } else {
        if (ui->serverCheckBox->isChecked()) {
            request.add_types(ErrorOriginType::ERROR_ORIGIN_SERVER);
        }
        if (ui->androidCheckBox->isChecked()) {
            request.add_types(ErrorOriginType::ERROR_ORIGIN_ANDROID);
        }
        if (ui->desktopInterfaceCheckBox->isChecked()) {
            request.add_types(ErrorOriginType::ERROR_ORIGIN_DESKTOP_INTERFACE);
        }
        if (ui->webServerCheckBox->isChecked()) {
            request.add_types(ErrorOriginType::ERROR_ORIGIN_WEB_SERVER);
        }

        if (request.types().empty()) {
            slot_displayWarning(
                    "Error",
                    "Please select at least one 'Error Origin'."
            );
            return;
        }
    }

    if (ui->searchUnknownCheckBox->isChecked()
        && ui->searchMaxCheckBox->isChecked()
        && ui->searchVeryHighCheckBox->isChecked()
        && ui->searchHighCheckBox->isChecked()
        && ui->searchMediumCheckBox->isChecked()
        && ui->searchLowCheckBox->isChecked()
        && ui->searchVeryLowCheckBox->isChecked()
            ) {
        request.set_all_error_urgency_levels(true);
    } else {
        if (ui->searchUnknownCheckBox->isChecked()) {
            request.add_levels(ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_UNKNOWN);
        }
        if (ui->searchMaxCheckBox->isChecked()) {
            request.add_levels(ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_MAX);
        }
        if (ui->searchVeryHighCheckBox->isChecked()) {
            request.add_levels(ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_VERY_HIGH);
        }
        if (ui->searchHighCheckBox->isChecked()) {
            request.add_levels(ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_HIGH);
        }
        if (ui->searchMediumCheckBox->isChecked()) {
            request.add_levels(ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_MEDIUM);
        }
        if (ui->searchLowCheckBox->isChecked()) {
            request.add_levels(ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_LOW);
        }
        if (ui->searchVeryLowCheckBox->isChecked()) {
            request.add_levels(ErrorUrgencyLevel::ERROR_URGENCY_LEVEL_VERY_LOW);
        }

        if (request.levels().empty()) {
            slot_displayWarning(
                    "Error",
                    "Please select at least one 'Error Urgency'."
            );
            return;
        }
    }

    if (ui->versionNumberAllCheckBox->isChecked()) {
        request.set_all_version_numbers(true);
    } else {

        int min = ui->versionNumberMinSpinBox->value();
        int max = ui->versionNumberMinSpinBox->value();

        if (min <= max) {
            request.set_min_version_number(min);
            request.set_max_version_number(max);
        } else {
            slot_displayWarning(
                    "Error",
                    "Min version number can not be larger than max version number."
            );
            return;
        }
    }

    if (ui->searchAndroidApiAllCheckBox->isChecked()) {
        request.set_all_android_api(true);
    } else {

        int min = ui->searchAndroidApiMinSpinBox->value();
        int max = ui->searchAndroidApiMaxSpinBox->value();

        if (min <= max) {
            request.set_min_api_number(min);
            request.set_max_api_number(max);
        } else {
            slot_displayWarning(
                    "Error",
                    "Min api number can not be larger than max api number."
            );
            return;
        }
    }

    if (ui->searchDateAllCheckBox->isChecked()) {
        request.set_all_timestamps(true);
    } else {
        const std::string date_input = ui->searchDateInputLineEdit->text().toStdString();

        if (stringIsEmptyOrAllWhiteSpace(date_input)) {
            ui->searchDateInputLineEdit->clear();
            ui->searchDateAllCheckBox->setChecked(true);
            request.set_all_timestamps(true);
        } else {

            auto [successful, timestamp] = convertDateStringToLong(date_input);

            if (!successful) {
                //error has already been sent
                return;
            }

            request.set_search_timestamp(timestamp.count());
            request.set_before_timestamp(ui->searchDateBeforeRadioButton->isChecked());
        }
    }

    if (ui->fileNameAllCheckBox->isChecked()) {
        request.set_all_file_names(true);
    } else {

        const std::string file_names = ui->fileNameInputLineEdit->text().toStdString();

        if (stringIsEmptyOrAllWhiteSpace(file_names)) {
            ui->fileNameInputLineEdit->clear();
            ui->fileNameAllCheckBox->setChecked(true);
            request.set_all_file_names(true);
        } else {

            //NOTE: there are several checks I could do here such as removing all
            // whitespace, removing extra spaces between chars,
            // checking for specific chars however JetBrains doesn't seem to have many
            // restrictions on file naming so only doing convenience checks.

            auto [return_str, success] = extractForCSVStringFields(
                    file_names,
                    [&](const std::string& stored_string) -> bool {
                        request.add_file_names(stored_string);
                        return true;
                    }
            );

            if (!return_str.isEmpty()) {
                slot_displayWarning(
                        "Error",
                        return_str.arg("'File Names'")
                );
            }

            if (!success) {
                return;
            }
        }
    }

    if (ui->lineNumberAllCheckBox->isChecked()) {
        request.set_all_line_numbers(true);
    } else {

        const std::string line_numbers = ui->lineNumberInputLineEdit->text().toStdString();

        if (stringIsEmptyOrAllWhiteSpace(line_numbers)) {
            ui->lineNumberInputLineEdit->clear();
            ui->lineNumberAllCheckBox->setChecked(true);
            request.set_all_line_numbers(true);
        } else {

            auto [return_str, success] = extractForCSVStringFields(
                    line_numbers,
                    [&](const std::string& stored_string) {
                        for (char c: stored_string) {
                            if (!std::isdigit(c)) {
                                slot_displayWarning(
                                        "Error",
                                        "Invalid line number entered."
                                );
                                return false;
                            }
                        }
                        try {
                            request.add_line_numbers(std::stoi(stored_string));
                        } catch (const std::out_of_range& e) {
                            slot_displayWarning(
                                    "Error",
                                    "Line number entered is too long to be stored in an int."
                            );
                            return false;
                        }
                        return true;
                    }
            );

            if (!return_str.isEmpty()) {
                slot_displayWarning(
                        "Error",
                        return_str.arg("'Line Numbers'")
                );
            }

            if (!success) {
                return;
            }
        }
    }

    if (ui->searchDeviceNameAllCheckBox->isChecked()) {
        request.set_all_device_names(true);
    } else {
        //NOTE: there are several checks I could do here such as removing all
        // whitespace, removing extra spaces between chars,
        // checking for specific chars however JetBrains doesn't seem to have many
        // restrictions on file naming so only doing convenience checks.

        const std::string device_names = ui->searchDeviceNameInputLineEdit->text().toStdString();

        if (stringIsEmptyOrAllWhiteSpace(device_names)) {
            ui->searchDeviceNameInputLineEdit->clear();
            ui->searchDeviceNameAllCheckBox->setChecked(true);
            request.set_all_device_names(true);
        } else {

            auto [return_str, success] = extractForCSVStringFields(
                    device_names,
                    [&](const std::string& stored_string) {
                        request.add_device_names(stored_string);
                        return true;
                    }
            );

            if (!return_str.isEmpty()) {
                slot_displayWarning(
                        "Error",
                        return_str.arg("'Device Names'")
                );
            }

            if (!success) {
                return;
            }

        }
    }

    ui->searchPushButton->setEnabled(false);

    search_errors_obj.runSearchErrors(
            request,
            [this]() {
                ui->searchPushButton->setEnabled(true);
            }
    );

}

void ErrorsWindow::on_setErrorHandledPushButton_clicked() {

    auto selected_items = ui->searchOptionsTreeWidget->selectedItems();
    std::string error_oid_string;
    QTreeWidgetItem* item = nullptr;

    //there should only be 1 item in this object because the tree selection mode is QAbstractItemView::SelectionMode::SingleSelection
    for (const auto& ele: selected_items) {
        item = ele;
    }

    if (item == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Selected item not found."
        );
        return;
    }

    auto* error_parameters = new handle_errors::ErrorParameters();
    QModelIndex model_index;

    model_index = ui->searchOptionsTreeWidget->indexFromItem(item);

    extractErrorParametersFromTreeWidgetItem(
            item,
            error_parameters
    );

    AccessStatusInvalidMessageDialog dialog(
            AccessStatusInvalidMessageDialog::DELETE_INPUT_ERROR_HANDLED);
    auto return_status = AccessStatusInvalidMessageDialog::ReturnStatusForDialog(dialog.exec());
    QString reason;
    ErrorHandledMoveReason error_handled_move_reason;

    switch (return_status) {
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_OK:
            reason = dialog.getReasonString();
            error_handled_move_reason = dialog.getErrorHandledMoveReason();
            break;
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_CANCEL:
            delete error_parameters;
            return;
        default:
            QMessageBox::warning(this,
                                 "Error",
                                 dialog.getErrorString()
            );
            delete error_parameters;
            return;
    }

    if (reason.size() < globals->minimum_number_allowed_bytes_modify_error_message_reason()) {
        QMessageBox::warning(this,
                             "Error",
                             "Description too short, " +
                             QString::number(globals->minimum_number_allowed_bytes_modify_error_message_reason()) +
                             " characters required."
        );
        delete error_parameters;
        return;
    }

    if (error_handled_move_reason != ErrorHandledMoveReason::ERROR_HANDLED_REASON_BUG_FIXED
        && error_handled_move_reason != ErrorHandledMoveReason::ERROR_HANDLED_REASON_BUG_NOT_RELEVANT
        && error_handled_move_reason != ErrorHandledMoveReason::ERROR_HANDLED_REASON_DELETE_ALL_DO_NOT_SET_HANDLED
            ) {
        QMessageBox::warning(this,
                             "Error",
                             "Error handled move reason invalid, '" +
                             QString::fromStdString(ErrorHandledMoveReason_Name(error_handled_move_reason)) +
                             "'."
        );
        delete error_parameters;
        return;
    }

    ui->setErrorHandledPushButton->setEnabled(false);

    set_error_to_handled_object.setErrorToHandled(
            error_parameters,
            error_handled_move_reason,
            reason.toStdString(),
            model_index,
            [this]() {
                ui->setErrorHandledPushButton->setEnabled(true);
            });
}

//assumes text_value has been converted to lower case already
bool ErrorsWindow::searchIndexedTextForValue(
        const std::unordered_multimap<char, int>& indexing,
        const std::string& text_value,
        const std::string& text_to_search
) {

    if (text_value.empty()) {
        return true;
    }

    if (text_value.size() > text_to_search.size()) {
        return false;
    }

    auto first_char_indexing = indexing.equal_range(text_value[0]);

    //no values associated with key
    if (first_char_indexing.first == first_char_indexing.second) {
        return false;
    }

    if (text_value.size() == 1) {
        return true;
    }

    for (auto& it = first_char_indexing.first; it != first_char_indexing.second; it++) {

        //if the remaining amount of text_to_search after idx is not
        // long enough to contain text_value
        if (text_value.size() > text_to_search.size() - it->second) {
            continue;
        }

        bool idx_matches = true;
        for (int i = 1; (size_t) i < text_value.size(); i++) {
            if (text_value[i] != (char) std::tolower(text_to_search[i + it->second])) {
                idx_matches = false;
                break;
            }
        }

        if (idx_matches) {
            return true;
        }
    }

    return false;
}

void ErrorsWindow::on_filterPushButton_clicked() {

    std::vector<int> extracted_values(extracted_errors->size());
    std::iota(extracted_values.begin(), extracted_values.end(), 0);

    //NOTE: order of checks will be important here for performance, want easiest most distinguishing checks first

    if (!ui->filterDeviceNameAllCheckBox->isChecked()) {

        //NOTE: there are several checks I could do here such as removing all
        // whitespace, removing extra spaces between chars,
        // checking for specific chars however JetBrains doesn't seem to have many
        // restrictions on file naming so only doing convenience checks.

        std::vector<std::string> device_names;
        const std::string device_names_str = ui->filterDeviceNameInputLineEdit->text().toStdString();

        if (stringIsEmptyOrAllWhiteSpace(device_names_str)) {
            ui->filterDeviceNameInputLineEdit->clear();
            ui->filterDeviceNameAllCheckBox->setChecked(true);
        } else {

            auto [return_str, success] = extractForCSVStringFields(
                    device_names_str,
                    [&](const std::string& stored_string) {
                        device_names.emplace_back(stored_string);
                        return true;
                    }
            );

            if (!return_str.isEmpty()) {
                slot_displayWarning(
                        "Error",
                        return_str.arg("'Device Names'")
                );
            }

            if (!success) {
                return;
            }

            if (!device_names.empty()) {

                auto it = extracted_values.begin();

                while (it != extracted_values.end()) {
                    bool equals_name = false;
                    for (const std::string& name: device_names) {
                        if ((*extracted_errors)[*it].message.error_message().device_name() == name) {
                            equals_name = true;
                            break;
                        }
                    }

                    if (equals_name) {
                        it++;
                    } else {
                        it = extracted_values.erase(it);
                    }
                }
            }
        }
    }

    if (!ui->filterAndroidApiAllCheckBox->isChecked()) {

        unsigned int min = ui->filterAndroidApiMinSpinBox->value();
        unsigned int max = ui->filterAndroidApiMaxSpinBox->value();

        if (min <= max) {

            auto it = extracted_values.begin();

            while (it != extracted_values.end()) {

                if (min <= (*extracted_errors)[*it].message.error_message().api_number()
                    && (*extracted_errors)[*it].message.error_message().api_number() <= max
                        ) {
                    it++;
                } else {
                    it = extracted_values.erase(it);
                }
            }
        } else {
            slot_displayWarning(
                    "Error",
                    "Min api number can not be larger than max api number."
            );
            return;
        }
    }

    if (!ui->filterDateAllCheckBox->isChecked()) {
        const std::string date_input = ui->filterDateInputLineEdit->text().toStdString();

        if (stringIsEmptyOrAllWhiteSpace(date_input)) {
            ui->filterDateInputLineEdit->clear();
            ui->filterDateAllCheckBox->setChecked(true);
        } else {

            auto [successful, timestamp] = convertDateStringToLong(date_input);

            if (!successful) {
                //error has already been sent
                return;
            }

            bool before = ui->filterDateBeforeRadioButton->isChecked();

            auto it = extracted_values.begin();

            while (it != extracted_values.end()) {
                if (before) {
                    if ((*extracted_errors)[*it].message.error_message().timestamp_stored() < timestamp.count()) {
                        it++;
                    } else {
                        it = extracted_values.erase(it);
                    }
                } else {
                    if ((*extracted_errors)[*it].message.error_message().timestamp_stored() > timestamp.count()) {
                        it++;
                    } else {
                        it = extracted_values.erase(it);
                    }
                }

            }
        }
    }

    if (!ui->filterTextInputLineEdit->text().isEmpty()) {

        std::string text = ui->filterTextInputLineEdit->text().toStdString();

        std::for_each(std::begin(text), std::end(text), [](char& c) {
            c = (char) std::tolower(c);
        });

        auto it = extracted_values.begin();

        while (it != extracted_values.end()) {

            auto device_name_it = std::search(
                    std::begin((*extracted_errors)[*it].message.error_message().device_name()),
                    std::end((*extracted_errors)[*it].message.error_message().device_name()),
                    std::begin(text),
                    std::end(text),
                    [](char lhs, char rhs) {
                        return lhs == (char) std::tolower(rhs);
                    });

            //NOTE: do not change order if these 'OR' statements
            if (
                    (*extracted_errors)[*it].message.error_message().device_name().end() != device_name_it
                    ||
                    searchIndexedTextForValue(
                            (*extracted_errors)[*it].stack_trace_indexing,
                            text,
                            (*extracted_errors)[*it].message.error_message().stack_trace()
                    )
                    ||
                    searchIndexedTextForValue(
                            (*extracted_errors)[*it].error_message_indexing,
                            text,
                            (*extracted_errors)[*it].message.error_message().error_message()
                    )) { //string found in, device name, stack trace OR error message
                it++;
            } else { //string not found
                it = extracted_values.erase(it);
            }
        }
    }

    ui->filterOptionsTreeWidget->clear();

    for (int idx: extracted_values) {
        displayFullErrorMessageOnFilterTree(idx);
    }

    ui->filterOptionsTreeWidget->viewport()->update();
}

std::tuple<bool, std::chrono::milliseconds> ErrorsWindow::convertDateStringToLong(const std::string& date_input) {
    DateStruct date_struct;

    try {

        for (int i = 0; (size_t) i < date_input.size(); i++) {
            std::string next_number_str;

            while ((size_t) i < date_input.size() && isdigit(date_input[i])) {
                next_number_str += date_input[i];
                i++;
            }

            if (!next_number_str.empty()) { //a number was found

                int next_number = std::stoi(next_number_str);

                if (date_struct.month == 0) {
                    if (next_number < 1 || 12 < next_number) {
                        slot_displayWarning(
                                "Error",
                                "Invalid Date Entered."
                        );
                        return {false, std::chrono::milliseconds{-1L}};
                    } else {
                        date_struct.month = next_number;
                    }
                } else if (date_struct.day == 0) {
                    if (next_number < 1 || 31 < next_number) {
                        slot_displayWarning(
                                "Error",
                                "Invalid Date Entered."
                        );
                        return {false, std::chrono::milliseconds{-1L}};
                    } else {
                        date_struct.day = next_number;
                    }
                } else if (date_struct.year == 0) {
                    if (next_number < 1900 || 2200 < next_number) {
                        slot_displayWarning(
                                "Error",
                                "Invalid Date Entered."
                        );
                        return {false, std::chrono::milliseconds{-1L}};
                    } else {
                        date_struct.year = next_number;
                    }
                } else if (date_struct.hours == 0) {
                    if (next_number < 0 || 23 < next_number) {
                        slot_displayWarning(
                                "Error",
                                "Invalid Date Entered."
                        );
                        return {false, std::chrono::milliseconds{-1L}};
                    } else {
                        date_struct.hours = next_number + 1;
                    }
                } else if (date_struct.minutes == 0) {
                    if (next_number < 0 || 59 < next_number) {
                        slot_displayWarning(
                                "Error",
                                "Invalid Date Entered."
                        );
                        return {false, std::chrono::milliseconds{-1L}};
                    } else {
                        date_struct.minutes = next_number + 1;
                    }
                } else {
                    slot_displayWarning(
                            "Error",
                            "Invalid Date Entered."
                    );
                    return {false, std::chrono::milliseconds{-1L}};
                }
            }
        }

    } catch (const std::out_of_range& e) {
        slot_displayWarning(
                "Error",
                "Invalid Date Entered."
        );
        return {false, std::chrono::milliseconds{-1L}};
    } catch (const std::invalid_argument& e) {
        slot_displayWarning(
                "Error",
                "Invalid Date Entered."
        );
        return {false, std::chrono::milliseconds{-1L}};
    }

    if (date_struct.year == 0 || date_struct.month == 0 || date_struct.day == 0) {
        slot_displayWarning(
                "Error",
                "Invalid Date Entered."
        );
        return {false, std::chrono::milliseconds{-1L}};
    }

    tm setupTime{};

    time_t lTimeEpoch = extractTimeTFromDate(
            date_struct.year,
            date_struct.month,
            date_struct.day,
            date_struct.hours ? date_struct.hours - 1 : 0,
            date_struct.minutes ? date_struct.minutes - 1 : 0,
            setupTime
    );

    return {true, std::chrono::milliseconds{lTimeEpoch * 1000}};
}

void ErrorsWindow::on_deleteSinglePushButton_clicked() {

    auto selected_items = ui->filterOptionsTreeWidget->selectedItems();
    std::string error_oid_string;
    QModelIndex model_index;
    QTreeWidgetItem* item = nullptr;

    //there should only be 1 item in this object because the tree selection mode is QAbstractItemView::SelectionMode::SingleSelection
    for (const auto& ele: selected_items) {
        item = ele;
    }

    if (item == nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Selected item not found."
        );
        return;
    }

    error_oid_string = item->data(0, Qt::UserRole).toString().toStdString();

    if (error_oid_string == ERROR_MESSAGE_DATA
        || error_oid_string == STACK_TRACE_DATA) {
        item = item->parent();

        if (item == nullptr) {
            QMessageBox::warning(this,
                                 "Error",
                                 "Selected item not found."
            );
            return;
        }

        error_oid_string = item->data(0, Qt::UserRole).toString().toStdString();
    }

    model_index = ui->filterOptionsTreeWidget->indexFromItem(item);

    QString oid_error_string = isValidOid(error_oid_string);

    if (!oid_error_string.isEmpty()) {
        QMessageBox::warning(this,
                             "Error",
                             oid_error_string
        );
        return;
    }

    AccessStatusInvalidMessageDialog dialog(
            AccessStatusInvalidMessageDialog::DELETE_INPUT_SINGLE);
    auto return_status = AccessStatusInvalidMessageDialog::ReturnStatusForDialog(dialog.exec());
    QString reason;

    switch (return_status) {
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_OK:
            reason = dialog.getReasonString();
            break;
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_CANCEL:
            return;
        case AccessStatusInvalidMessageDialog::ACCESS_STATUS_RETURN_ERROR:
            QMessageBox::warning(this,
                                 "Error",
                                 dialog.getErrorString()
            );
            return;
    }

    if (reason.size() < globals->minimum_number_allowed_bytes_modify_error_message_reason()) {
        QMessageBox::warning(this,
                             "Error",
                             "Reason too short, " +
                             QString::number(globals->minimum_number_allowed_bytes_modify_error_message_reason()) +
                             " characters required."
        );
        return;
    }


    if (extracted_errors->empty()) {
        QMessageBox::warning(this,
                             "Error",
                             "extracted_errors was empty when attempting to delete single message."
        );
        return;
    }

    ui->deleteSinglePushButton->setEnabled(false);

    delete_single_error_object.deleteSingleError(
            error_oid_string,
            reason.toStdString(),
            model_index,
            [this]() {
                ui->deleteSinglePushButton->setEnabled(true);
            });
}