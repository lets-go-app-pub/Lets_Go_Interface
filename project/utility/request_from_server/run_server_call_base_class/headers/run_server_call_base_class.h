//
// Created by jeremiah on 9/6/21.
//

#pragma once

#include <QObject>

#include "thread"

class RunServerCallBaseClass : public QObject {
Q_OBJECT

public:
    explicit RunServerCallBaseClass(QObject* parent = nullptr);

    void cancelRequest();

signals:

    //will be set to completed if successful
    void signal_requestSuccessfullyCompleted(const std::function<void()>& completed_lambda = [](){});

    //this will be set to the error message if one occurs
    void signal_sendWarning(
            const QString& title,
            const QString& text,
            const std::function<void()>& completed_lambda = [](){}
    );

    //this will be called if the request was canceled
    void signal_requestCanceled(const std::function<void()>& completed_lambda = [](){});

    void signal_internalCompleted();

private slots:

    void slot_internalCompleted();

protected:

    //only set/get this variable in the UI Thread
    std::unique_ptr<std::jthread> function_thread;
};
