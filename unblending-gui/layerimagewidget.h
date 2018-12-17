#ifndef LAYERIMAGEWIDGET_H
#define LAYERIMAGEWIDGET_H

#include <QWidget>
#include <memory>
#include "imagewidget.h"

class LayerImageWidget : public ImageWidget
{
    Q_OBJECT
public:
    explicit LayerImageWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;

signals:

public slots:
};

#endif // LAYERIMAGEWIDGET_H
