//
// Created by jeremiah on 9/26/21.
//

#pragma once
#include <QWidget>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class ButtonWithMetaData; }
QT_END_NAMESPACE

class ButtonWithMetaData : public QPushButton {
Q_OBJECT

public:
    explicit ButtonWithMetaData(std::string  _meta_data, QWidget* parent = nullptr);

    ~ButtonWithMetaData() override;

signals:
    void signal_clickedWithMetaData(const std::string& meta_data, ButtonWithMetaData* button_handle, bool checked);

private slots:
    void slot_receiveClicked(bool checked);

private:
    Ui::ButtonWithMetaData* ui;

    std::string meta_data;
};

