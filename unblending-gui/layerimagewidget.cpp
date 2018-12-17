#include "layerimagewidget.h"
#include <cassert>
#include <QPainter>
#include <unblending/image_processing.hpp>

LayerImageWidget::LayerImageWidget(QWidget *parent) : ImageWidget(parent)
{
}

void LayerImageWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    
    // Draw checker board
    {
        constexpr int size = 8;
        const QColor gray_1(255, 255, 255);
        const QColor gray_2(220, 220, 220);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(gray_1));
        painter.drawRect(0, 0, this->width(), this->height());
        painter.setBrush(QBrush(gray_2));
        for (int x = 0; x < width(); x += size)
        {
            for (int y = 0; y < height(); y += size)
            {
                const bool cond_1 = (x / size) % 2 == 0;
                const bool cond_2 = (y / size) % 2 == 0;
                if ((cond_1 && !cond_2) || (!cond_1 && cond_2)) painter.drawRect(x, y, size, size);
            }
        }
    }
    painter.end();
    
    ImageWidget::paintEvent(event);
}
