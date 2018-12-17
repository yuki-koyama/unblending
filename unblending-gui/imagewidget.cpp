#include "imagewidget.h"
#include <cassert>
#include <QPainter>
#include <unblending/image_processing.hpp>

ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent)
{
    this->setMinimumWidth (150);
    this->setMinimumHeight(150);
}

void ImageWidget::copy_and_set_image(const unblending::ColorImage &image)
{
    image_data_   = image.get_rgba_bits();
    image_width_  = image.width();
    image_height_ = image.height();
    
    update();
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    QPainter painter;
    painter.begin(this);
    
    // Draw image if exists
    if (!image_data_.empty())
    {
        assert(image_data_.size() == image_width_ * image_height_ * 4);
        const QImage image(image_data_.data(), image_width_, image_height_, QImage::Format_RGBA8888);
        const QImage scaled_image = image.scaled(this->width(),
                                                 this->height(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
        painter.drawPixmap(0, 0, QPixmap::fromImage(scaled_image, Qt::AutoColor));
    }
    
    painter.end();
}
