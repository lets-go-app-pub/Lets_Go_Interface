//
// Created by jeremiah on 9/10/21.
//

#pragma once

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>


class AspectRatioPixmapLabel : public QWidget {
Q_OBJECT

public:
    explicit AspectRatioPixmapLabel(QWidget* parent = 0);

    [[nodiscard]] QPixmap pixmap() const;

public slots:

    void setPixmap(const QPixmap&);

protected:
    void resizeEvent(QResizeEvent*);

private slots:

    void resizeImage();

private:
    QLabel* label;
};

//class AspectRatioPixmapLabel : public QWidget {
//Q_OBJECT
//
//public:
//    explicit AspectRatioPixmapLabel(QWidget* parent = 0);
//
//    const QPixmap* pixmap() const;
//
//public slots:
//
//    void setPixmap(const QPixmap&);
//
//protected:
//    void paintEvent(QPaintEvent*);
//
//private:
//    QPixmap pix;
//};

//class AspectRatioPixmapLabel : public QLabel {
//Q_OBJECT
//public:
//    explicit AspectRatioPixmapLabel(QWidget* parent = 0);
//
//    virtual int heightForWidth(int width) const;
//
//    virtual QSize sizeHint() const;
//
//    QPixmap scaledPixmap() const;
//
//public slots:
//
//    void setPixmap(const QPixmap&);
//
//    void resizeEvent(QResizeEvent*);
//
//private:
//    QPixmap pix;
//};

