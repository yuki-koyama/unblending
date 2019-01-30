#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <memory>
#include <vector>

namespace unblending
{
    class ColorImage;
}

/// \brief Widget class for displaying a static image
class ImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget *parent = nullptr);
    
    void copy_and_set_image(const unblending::ColorImage& image);
    
protected:
    std::vector<uint8_t> image_data_;
    int image_width_;
    int image_height_;
    
protected:
    void paintEvent(QPaintEvent*) override;
};

#endif // IMAGEWIDGET_H
