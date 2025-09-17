
#pragma once

#include <QPlainTextEdit>
#include <QPixmap>
#include <QResizeEvent>

class PlainTextEditWithMaxChars : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit PlainTextEditWithMaxChars(QWidget* parent = 0);

    int getMaxChar() const;

    //values < 0 will be treated as no limit
    void setMaxChar(int max_char);

    void myTextChanged();

private:

    //values < 0 will be treated as no limit
    int maxChar = -1;
};
