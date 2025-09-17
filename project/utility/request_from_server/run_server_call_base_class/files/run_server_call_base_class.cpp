//
// Created by jeremiah on 9/6/21.
//

#include "run_server_call_base_class.h"

RunServerCallBaseClass::RunServerCallBaseClass(QObject* parent) : QObject(parent) {
    connect(this, &RunServerCallBaseClass::signal_internalCompleted, this, &RunServerCallBaseClass::slot_internalCompleted);
}

void RunServerCallBaseClass::slot_internalCompleted() {

    if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
        function_thread = nullptr;
    }
}

void RunServerCallBaseClass::cancelRequest() {
    if(function_thread != nullptr) {
        function_thread->request_stop();
        function_thread->join();
        function_thread = nullptr;
    }
}