//
// Created by jeremiah on 8/26/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_statistics_window.h" resolved

#include <iostream>

#include <thread>
#include <grpcpp/impl/codegen/client_context.h>

#include <QMessageBox>
#include <user_login_info.h>
#include <grpc_channel.h>

#include "setup_login_info.h"
#include "statistics_window.h"
#include "ui_statistics_window.h"
#include "homewindow.h"


StatisticsWindow::StatisticsWindow(QWidget* parent) :
        QWidget(parent), ui(new Ui::StatisticsWindow) {
    ui->setupUi(this);

    connect(this, &StatisticsWindow::signal_sendWarning, this, &StatisticsWindow::slot_displayWarning);
    connect(this, &StatisticsWindow::signal_requestActivityStatisticsThreadCompleted,
            this, &StatisticsWindow::slot_requestActivityStatisticsThreadCompleted);
    connect(this, &StatisticsWindow::signal_setupSingleBarChart,
            this, &StatisticsWindow::slot_setupSingleBarChart);
    connect(this, &StatisticsWindow::signal_requestAgeGenderStatisticsThreadCompleted,
            this, &StatisticsWindow::slot_requestAgeGenderStatisticsThreadCompleted);
    connect(this, &StatisticsWindow::signal_setupBarChartWithLineGraph,
            this, &StatisticsWindow::slot_setupBarChartWithLineGraph);

    QStringList names;
    names << "Dummy" << "Bar" << "Chart" << "Data" << ".";
    QList<qreal> counts;
    counts << 1 << 3 << 2 << 5 << 2;

    slot_setupSingleBarChart(names, counts);

    if (user_admin_privileges.view_matching_activity_statistics()) {
        ui->getActivityStatisticsVerticalWidget->setVisible(true);
    } else {
        ui->getActivityStatisticsVerticalWidget->setVisible(false);
    }

    if (user_admin_privileges.view_age_gender_statistics()) {
        ui->getAgeStatisticsVerticalWidget->setVisible(true);
    } else {
        ui->getAgeStatisticsVerticalWidget->setVisible(false);
    }

    this->setMouseTracking(true);
}

StatisticsWindow::~StatisticsWindow() {
    delete ui;
    //Separate
}

void StatisticsWindow::closeEvent(QCloseEvent* event) {
    home_window_handle->show();
    QWidget::closeEvent(event);
}

void StatisticsWindow::slot_displayWarning(
        const QString& title,
        const QString& text,
        const std::function<void()>& lambda
) {

    lambda();
    std::cout << "title: " << title.toStdString() << '\n';
    std::cout << "text: " << text.toStdString() << "\n\n";

    QMessageBox::warning(
            this,
            title,
            text
    );
}

void StatisticsWindow::slot_requestAgeGenderStatisticsThreadCompleted() {
    ui->ageAndGenderStatisticsSearchButton->setEnabled(true);
    request_age_gender_statistics_thread = nullptr;
}

void StatisticsWindow::slot_requestActivityStatisticsThreadCompleted() {
    ui->getActivityStatisticsButton->setEnabled(true);
    request_activity_statistics_thread = nullptr;
}

void StatisticsWindow::slot_setupBarChartWithLineGraph(
        const request_statistics::AgeGenderStatisticsResponse& response
) {
    auto* male_bar_set = new QBarSet("Male");
    auto* female_bar_set = new QBarSet("Female");
    auto* other_bar_set = new QBarSet("Other");

    bar_series = new QBarSeries();
    line_series = new QLineSeries();
    line_series->setName("Total number users");

    QStringList chart_categories;

    unsigned long min_age = 1000;
    unsigned long max_age = 1;

    unsigned long max_number_users = 1;
    int index = 0;
    unsigned long total_number_users = 0;
    for (const auto& element: response.gender_selected_at_age_list()) {

        *male_bar_set << (double) element.gender_male();
        *female_bar_set << (double) element.gender_female();
        *other_bar_set << (double) element.gender_other();

        chart_categories << QString::number(element.age());

        if (element.age() < min_age) {
            min_age = element.age();
        }

        if (element.age() > max_age) {
            max_age = element.age();
        }

        unsigned long total_number_users_at_age =
                element.gender_male() + element.gender_female() + element.gender_other();

        total_number_users += total_number_users_at_age;

        if (max_number_users < total_number_users_at_age) {
            max_number_users = total_number_users_at_age;
        }

        line_series->append(QPoint(index, (int) total_number_users_at_age));
        index++;
    }

    if (min_age >= max_age) {
        min_age = 13;
        max_age = 120;
    }

    const QString text = QString("Total Number Users: ").append(QString::number(total_number_users));
    ui->displayChartInfoLabel->setText(text);

    bar_series->append(male_bar_set);
    bar_series->append(female_bar_set);
    bar_series->append(other_bar_set);

    chart = new QChart();
    chart->addSeries(bar_series);
    chart->addSeries(line_series);

    axisX = new QBarCategoryAxis();
    axisX->append(chart_categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    line_series->attachAxis(axisX);
    bar_series->attachAxis(axisX);
    axisX->setRange(QString::number(min_age), QString::number(max_age));

    axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    line_series->attachAxis(axisY);
    bar_series->attachAxis(axisY);
    axisY->setRange(0, qreal(max_number_users));

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    connect(bar_series, &QBarSeries::hovered, this, &StatisticsWindow::display_bar_series_info);
    connect(line_series, &QLineSeries::hovered, this, &StatisticsWindow::display_line_series_info);

    delete chartView;
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    ui->statisticsChartVerticalLayout->layout()->addWidget(chartView);
}

void StatisticsWindow::slot_setupSingleBarChart(const QStringList& labels, const QList<qreal>& counts) {
    //QChart will take ownership of this below, no need to deallocate.
    auto* stacked_bar_series = new QStackedBarSeries();

    QList<qreal> dummyCount;
    for (int i = 0; i < labels.count(); ++i) {
        auto barSet = new QBarSet(labels.at(i));
        // to "trick" the bar chart into different colors, each new group
        // must be in a different value position so replace the previous
        // value with 0 and insert the next one
        if (i > 0) dummyCount.replace(i - 1, 0);
        dummyCount << counts.at(i);
        barSet->append(dummyCount);
        stacked_bar_series->append(barSet);
    }

    chart = new QChart();
    chart->addSeries(stacked_bar_series);
    chart->legend()->hide();

    axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    axisY->applyNiceNumbers();
    stacked_bar_series->attachAxis(axisY);
    axisX = new QBarCategoryAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    axisX->append(labels);
    stacked_bar_series->attachAxis(axisX);

    connect(stacked_bar_series, &QStackedBarSeries::hovered, this, &StatisticsWindow::display_bar_series_info);

    delete chartView;
    chartView = new QChartView(chart);

    setupClearChartLayout(ui->statisticsChartVerticalLayout->layout());
    ui->statisticsChartVerticalLayout->layout()->addWidget(chartView);
}

void StatisticsWindow::setupClearChartLayout(QLayout* layout) {
    QLayoutItem* item;
    while ((item = layout->takeAt(0))) {
        if (item->layout()) {
            setupClearChartLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
}

void StatisticsWindow::on_getActivityStatisticsButton_clicked() {

    int number_days = ui->numberOfDaysToSearchActivityStatisticsSpinBox->value();

    if (
            !ui->usersCheckBox->isChecked()
            && !ui->userEventsCheckBox->isChecked()
            && !ui->letsGoEventsCheckBox->isChecked()
            ) {
        QMessageBox::warning(
                this,
                "Error",
                "Please check at least 1 box under 'Account Types'."
        );
    } else if (
            !ui->swipedYesYesCheckBox->isChecked()
            && !ui->swipedYesNoCheckBox->isChecked()
            && !ui->swipedYesBlockAndReportCheckBox->isChecked()
            && !ui->swipedYesIncompleteCheckBox->isChecked()
            && !ui->swipedNoCheckBox->isChecked()
            && !ui->swipedBlockAndReportCheckBox->isChecked()
            && !ui->swipedIncompleteCheckBox->isChecked()
            ) {
        QMessageBox::warning(
                this,
                "Error",
                "Please check at least 1 box under 'Swipe Options'."
        );
    } else if (number_days < 1 ||
               MAX_NUMBER_DAYS_TO_SEARCH_FOR_STATISTICS < number_days) { //at least 1 check box is checked
        QMessageBox::warning(
                this,
                "Error",
                ("Please set 'Number Days' to a value between 1 and " +
                 std::to_string(MAX_NUMBER_DAYS_TO_SEARCH_FOR_STATISTICS) + ".").c_str()
        );
    } else if (request_activity_statistics_thread != nullptr) { //valid info entered
        QMessageBox::warning(this,
                             "Error",
                             "Request activity statistics is already running."
        );
    } else { //valid info entered

        ui->getActivityStatisticsButton->setEnabled(false);
        request_statistics::MatchingActivityStatisticsRequest request;
        setup_login_info(request.mutable_login_info());

        request.set_include_categories(ui->includeCategoriesCheckBox->isChecked());
        request.set_number_days_to_search(number_days);
        request.set_yes_yes(ui->swipedYesYesCheckBox->isChecked());
        request.set_yes_no(ui->swipedYesNoCheckBox->isChecked());
        request.set_yes_block_and_report(ui->swipedYesBlockAndReportCheckBox->isChecked());
        request.set_yes_incomplete(ui->swipedYesIncompleteCheckBox->isChecked());
        request.set_no(ui->swipedNoCheckBox->isChecked());
        request.set_block_and_report(ui->swipedBlockAndReportCheckBox->isChecked());
        request.set_incomplete(ui->swipedIncompleteCheckBox->isChecked());

        if (ui->usersCheckBox->isChecked()) {
            request.add_user_account_types(UserAccountType::USER_ACCOUNT_TYPE);
        }

        if (ui->letsGoEventsCheckBox->isChecked()) {
            request.add_user_account_types(UserAccountType::ADMIN_GENERATED_EVENT_TYPE);
        }

        if (ui->userEventsCheckBox->isChecked()) {
            request.add_user_account_types(UserAccountType::USER_GENERATED_EVENT_TYPE);
        }

        if (request.user_account_types().empty()) {
            QMessageBox::warning(
                    this,
                    "Error",
                    "Please check at least 1 box under 'Account Types'."
            );
            return;
        }

        request_activity_statistics_thread = std::make_unique<std::jthread>(
                [request, this](const std::stop_token& stop_token) {
                    grpc::ClientContext unary_context;

                    std::stop_callback stopCallback = std::stop_callback(stop_token, [&] {
                        unary_context.TryCancel();
                    });

                    request_statistics::MatchingActivityStatisticsResponse response;

                    std::unique_ptr<request_statistics::RequestStatisticsService::Stub> request_statistics_stub = request_statistics::RequestStatisticsService::NewStub(
                            channel);

                    //NOTE: long deadline here to give it time to calculate
                    unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE_STATISTICS);

                    grpc::Status status = request_statistics_stub->MatchingActivityStatisticsRPC(&unary_context,
                                                                                                 request,
                                                                                                 &response);

                    if (stop_token.stop_requested()) {
                        emit signal_requestActivityStatisticsThreadCompleted();
                        return;
                    } else if (!status.ok()) { //if grpc call failed
                        const std::string errorMessage = "Grpc returned status.ok() == false; code: " +
                                                         std::to_string(status.error_code()) +
                                                         " message: " + status.error_message();

                        emit signal_sendWarning(
                                "Error",
                                errorMessage.c_str(),
                                [&]() {
                                    slot_requestActivityStatisticsThreadCompleted();
                                });
                        return;
                    } else if (!response.success()) { //if failed to log in

                        emit signal_sendWarning(
                                "Error",
                                response.error_msg().c_str(),
                                [&]() {
                                    slot_requestActivityStatisticsThreadCompleted();
                                });

                        return;
                    }

                    if (response.activity_statistics_size() == 0) {

                        emit signal_sendWarning(
                                "Error",
                                "No data to show in the selected time frame.",
                                [&]() {
                                    slot_requestActivityStatisticsThreadCompleted();
                                });
                        return;
                    }

                    QStringList labels;
                    QList<qreal> counts;

                    for (auto& activity: response.activity_statistics()) {
                        if (activity.account_category_type() == AccountCategoryType::ACTIVITY_TYPE) {
                            std::string activity_name;
                            if (request.include_categories()) {
                                activity_name += "A ";
                            }

                            activity_name += activities[activity.activity_or_category_index()].display_name();

                            labels << activity_name.c_str();
                            counts << double(activity.number_times_swiped());
                        }
                    }

                    for (auto& activity: response.activity_statistics()) {
                        if (activity.account_category_type() == AccountCategoryType::CATEGORY_TYPE) {
                            std::string category_name;
                            if (request.include_categories()) {
                                category_name += "C ";
                            }

                            category_name += categories[activity.activity_or_category_index()].display_name();

                            labels << category_name.c_str();
                            counts << double(activity.number_times_swiped());
                        }
                    }

                    emit signal_setupSingleBarChart(labels, counts);
                    emit signal_requestActivityStatisticsThreadCompleted();
                });

    }
}

void StatisticsWindow::on_ageAndGenderStatisticsSearchButton_clicked() {

    ui->ageAndGenderStatisticsSearchButton->setEnabled(false);

    if (request_age_gender_statistics_thread != nullptr) {
        QMessageBox::warning(this,
                             "Error",
                             "Request age gender statistics is already running."
        );
        return;
    }

    request_age_gender_statistics_thread = std::make_unique<std::jthread>(
            [this](const std::stop_token& stop_token) {
                grpc::ClientContext unary_context;

                std::stop_callback stop_callback = std::stop_callback(stop_token, [&]() {
                    unary_context.TryCancel();
                });

                request_statistics::AgeGenderStatisticsRequest request;
                request_statistics::AgeGenderStatisticsResponse response;
                setup_login_info(request.mutable_login_info());

                std::unique_ptr<request_statistics::RequestStatisticsService::Stub> request_statistics_stub = request_statistics::RequestStatisticsService::NewStub(
                        channel);

                //NOTE: long deadline here to give it time to calculate
                unary_context.set_deadline(std::chrono::system_clock::now() + GRPC_DEADLINE_STATISTICS);

                grpc::Status status = request_statistics_stub->MatchingAgeGenderRPC(&unary_context, request,
                                                                                    &response);

                if (stop_token.stop_requested()) {
                    emit signal_requestAgeGenderStatisticsThreadCompleted();
                    return;
                } else if (!status.ok()) { //if grpc call failed
                    const std::string errorMessage = "Grpc returned status.ok() == false; code: " +
                                                     std::to_string(status.error_code()) +
                                                     " message: " + status.error_message();

                    emit signal_sendWarning(
                            "Error",
                            errorMessage.c_str(),
                            [&]() {
                                signal_requestAgeGenderStatisticsThreadCompleted();
                            });
                    return;
                } else if (!response.success()) { //if failed to log in

                    emit signal_sendWarning(
                            "Error",
                            response.error_msg().c_str(),
                            [&]() {
                                signal_requestAgeGenderStatisticsThreadCompleted();
                            });
                    return;
                }

                if (response.gender_selected_at_age_list().empty()) {
                    QMessageBox::warning(this,
                                         "No Data",
                                         "No data to show in the selected time frame."
                    );

                    emit signal_sendWarning(
                            "No Data",
                            "No data to show in the selected time frame.",
                            [&]() {
                                signal_requestAgeGenderStatisticsThreadCompleted();
                            });
                    return;
                }

                std::sort(response.mutable_gender_selected_at_age_list()->begin(),
                          response.mutable_gender_selected_at_age_list()->end(),
                          [](const request_statistics::NumberOfTimesGenderSelectedAtAge& lhs,
                             const request_statistics::NumberOfTimesGenderSelectedAtAge& rhs) {
                              return lhs.age() < rhs.age();
                          });

                emit signal_setupBarChartWithLineGraph(response);
                emit signal_requestAgeGenderStatisticsThreadCompleted();
            });
}

void StatisticsWindow::display_bar_series_info(bool status, int index, QBarSet* bar_set) {

    if (status) {
        std::string text = bar_set->label().toStdString() + "\nNumber: " + std::to_string(long(bar_set->at(index)));
        ui->displayChartInfoLabel->setText(text.c_str());
    }
}

void StatisticsWindow::display_line_series_info(const QPointF& point, bool state) {

    if (state) {
        std::string text =
                "Age: " + axisX->at(point.x()).toStdString() + "\nNumber: " + std::to_string(long(point.y()));
        ui->displayChartInfoLabel->setText(text.c_str());
    }
}

