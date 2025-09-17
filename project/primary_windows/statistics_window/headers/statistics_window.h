//
// Created by jeremiah on 8/26/21.
//
#pragma once

#include <sys/types.h>
#include <thread>

#include <QWidget>
#include <QMainWindow>
#include <QBarSet>
#include <QBarSeries>
#include <QLineSeries>
#include <QChart>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChartView>

#include <RequestStatistics.grpc.pb.h>
#include <QHorizontalStackedBarSeries>
#include <QStackedBarSeries>

class HomeWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class StatisticsWindow; }
QT_END_NAMESPACE

        QT_BEGIN_NAMESPACE

class QChart;

QT_END_NAMESPACE

class StatisticsWindow : public QWidget {
    Q_OBJECT
protected:
    void closeEvent(QCloseEvent* event) override;

public:

    void setHomeWindowHandle(HomeWindow* _home_window_handle) {
        home_window_handle = _home_window_handle;
    }

    explicit StatisticsWindow(QWidget* parent = nullptr);

    ~StatisticsWindow() override;

private
    slots:

    void slot_requestActivityStatisticsThreadCompleted();

    void slot_requestAgeGenderStatisticsThreadCompleted();

    void on_getActivityStatisticsButton_clicked();

    void on_ageAndGenderStatisticsSearchButton_clicked();

    void display_bar_series_info(bool status, int index, QBarSet* bar_set);

    void display_line_series_info(const QPointF& point, bool state);

    void slot_displayWarning(const QString& title, const QString& text, const std::function<void()>& lambda = {});

    void slot_setupSingleBarChart(const QStringList& labels, const QList <qreal>& counts);

    void slot_setupBarChartWithLineGraph(const request_statistics::AgeGenderStatisticsResponse& response);

    signals:

    void signal_requestActivityStatisticsThreadCompleted();

    void signal_requestAgeGenderStatisticsThreadCompleted();

    void signal_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& lambda = {}
    );

    void signal_setupSingleBarChart(const QStringList& labels, const QList <qreal>& counts);

    void signal_setupBarChartWithLineGraph(const request_statistics::AgeGenderStatisticsResponse& response);

private:
    HomeWindow* home_window_handle = nullptr;
    Ui::StatisticsWindow* ui;


    QBarSeries* bar_series = nullptr;
    QLineSeries* line_series = nullptr;

    QChart* chart = nullptr;

    QBarCategoryAxis* axisX = nullptr;
    QValueAxis* axisY = nullptr;

    QChartView* chartView = nullptr;

    void setupClearChartLayout(QLayout* layout);

    //only set or get these values on the UI thread
    std::unique_ptr<std::jthread> request_activity_statistics_thread = nullptr;
    std::unique_ptr<std::jthread> request_age_gender_statistics_thread = nullptr;

};
