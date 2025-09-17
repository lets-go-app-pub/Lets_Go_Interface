//
// Created by jeremiah on 9/26/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_button_with_meta_data.h" resolved

#include "button_with_meta_data.h"

#include <utility>
#include "ui_button_with_meta_data.h"

ButtonWithMetaData::ButtonWithMetaData(std::string _meta_data, QWidget* parent) :
QPushButton(parent), ui(new Ui::ButtonWithMetaData), meta_data(std::move(_meta_data)) {
    ui->setupUi(this);

    connect(this, &ButtonWithMetaData::clicked, this, &ButtonWithMetaData::slot_receiveClicked);
}

ButtonWithMetaData::~ButtonWithMetaData() {
    delete ui;
}

void ButtonWithMetaData::slot_receiveClicked(bool checked) {
    emit signal_clickedWithMetaData(meta_data, this, checked);
}

