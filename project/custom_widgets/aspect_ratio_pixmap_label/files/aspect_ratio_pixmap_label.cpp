//
// Created by jeremiah on 9/10/21.
//

#include <QPainter>
#include <iostream>
#include "aspect_ratio_pixmap_label.h"

AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) :
QWidget(parent)
{
    label = new QLabel(this);
    label->setScaledContents(true);
    label->setFixedSize(0,0);
}

void AspectRatioPixmapLabel::resizeEvent(QResizeEvent *event) {
    std::cout << "resizeEvent\n";
    QWidget::resizeEvent(event);
    resizeImage();
}

QPixmap AspectRatioPixmapLabel::pixmap() const {
    return label->pixmap();
}

void AspectRatioPixmapLabel::setPixmap (const QPixmap &pixmap){
    label->setPixmap(pixmap);
    resizeImage();
}

void AspectRatioPixmapLabel::resizeImage() {
    QSize pixSize = label->pixmap().size();
    pixSize.scale(size(), Qt::KeepAspectRatio);
    label->setFixedSize(pixSize);
}

//AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) :
//QWidget(parent)
//{
//}
//
//void AspectRatioPixmapLabel::paintEvent(QPaintEvent *event) {
//    QWidget::paintEvent(event);
//
//    if (pix.isNull())
//        return;
//
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);
//
//    QSize pixSize = pix.size();
//    pixSize.scale(event->rect().size(), Qt::KeepAspectRatio);
//
//    QPixmap scaledPix = pix.scaled(pixSize,
//                                   Qt::KeepAspectRatio,
//                                   Qt::SmoothTransformation
//                                   );
//
//    painter.drawPixmap(QPoint(), scaledPix);
//
//}
//
//const QPixmap* AspectRatioPixmapLabel::pixmap() const {
//    return &pix;
//}
//
//void AspectRatioPixmapLabel::setPixmap (const QPixmap &pixmap){
//    pix = pixmap;
//}

//AspectRatioPixmapLabel::AspectRatioPixmapLabel(QWidget *parent) :
//QLabel(parent)
//{
//    this->setMinimumSize(1,1);
//    setScaledContents(false);
//}
//
//void AspectRatioPixmapLabel::setPixmap ( const QPixmap & p)
//{
//    pix = p;
//    QLabel::setPixmap(scaledPixmap());
//}
//
//int AspectRatioPixmapLabel::heightForWidth( int width ) const
//{
//    return pix.isNull() ? this->height() : ((qreal)pix.height()*width)/pix.width();
//}
//
//QSize AspectRatioPixmapLabel::sizeHint() const
//{
//    int w = this->width();
//    return { w, heightForWidth(w) };
//}
//
//QPixmap AspectRatioPixmapLabel::scaledPixmap() const
//{
//    return pix.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
//}
//
//void AspectRatioPixmapLabel::resizeEvent(QResizeEvent * e)
//{
//    if(!pix.isNull())
//        QLabel::setPixmap(scaledPixmap());
//}