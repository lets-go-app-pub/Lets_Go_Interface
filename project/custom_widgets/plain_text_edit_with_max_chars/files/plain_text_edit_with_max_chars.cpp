//
// Created by jeremiah on 9/13/21.
//

#include <QMessageBox>
#include "plain_text_edit_with_max_chars.h"

PlainTextEditWithMaxChars::PlainTextEditWithMaxChars(QWidget* parent) : QPlainTextEdit(parent) {
    connect(this, &PlainTextEditWithMaxChars::textChanged, this, &PlainTextEditWithMaxChars::myTextChanged);
}

int PlainTextEditWithMaxChars::getMaxChar() const {
    return maxChar;
}

void PlainTextEditWithMaxChars::setMaxChar(int max_char) {
    maxChar = max_char;
}

void PlainTextEditWithMaxChars::myTextChanged() {

    if (maxChar < 0) {
        return;
    }

    QString plain_text = QPlainTextEdit::toPlainText();
    if (plain_text.length() > maxChar) {
        QPlainTextEdit::setPlainText(plain_text.left(plain_text.length() - 1));
        QPlainTextEdit::moveCursor(QTextCursor::End);
    }
}
