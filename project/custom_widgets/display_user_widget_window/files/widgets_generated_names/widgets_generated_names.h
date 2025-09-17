//
// Created by jeremiah on 10/10/21.
//

#pragma once

#include <QString>

QString generateLabel(const std::string& text);

QString generateQTreeWidgetItemName(const std::string& label_constant);

QString generatePushButtonName(const std::string& label_constant);

QString generateLineEditName(const std::string& label_constant);

QString generateComboBoxName(const std::string& label_constant);

QString generateIndexedLineEditName(const std::string& label_constant, int index);

QString generatePlainTextEditName(const std::string& label_constant);

QString generateSpinBoxName(const std::string& label_constant, int number);