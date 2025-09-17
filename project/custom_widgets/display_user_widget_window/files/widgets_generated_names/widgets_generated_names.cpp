//
// Created by jeremiah on 10/10/21.
//

#include "widgets_generated_names.h"

QString generateLabel(const std::string& text) {
    std::string copy(text);
    std::transform(copy.begin(), copy.end(), copy.begin(), [](unsigned char c) -> unsigned char {
        return c == '_' ? ' ' : c;
    });

    return QString("%1: ").arg(copy.c_str());
}

QString generateQTreeWidgetItemName(const std::string& label_constant) {
    return QString("%1_QTreeWidgetItem").arg(label_constant.c_str());
}

QString generatePushButtonName(const std::string& label_constant) {
    return QString("%1_PushButton").arg(label_constant.c_str());
}

QString generateLineEditName(const std::string& label_constant) {
    return QString("%1_LineEdit").arg(label_constant.c_str());
}

QString generateComboBoxName(const std::string& label_constant) {
    return QString("%1_ComboBox").arg(label_constant.c_str());
}

QString generateIndexedLineEditName(const std::string& label_constant, int index) {
    return QString("%1_%2_LineEdit").arg(index).arg(label_constant.c_str());
}

QString generatePlainTextEditName(const std::string& label_constant) {
    return QString("%1_PlainTextEdit").arg(label_constant.c_str());
}

QString generateSpinBoxName(const std::string& label_constant, int number) {
    return QString("%1_%2_SpinBox").arg(label_constant.c_str()).arg(number);
}