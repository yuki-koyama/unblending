#ifndef LAYERWIDGET_H
#define LAYERWIDGET_H

#include <memory>
#include <QWidget>

class QSlider;
class QComboBox;
class LayerImageWidget;
class LayerColorModelWidget;

namespace unblending
{
    enum class BlendMode;
    class ColorModel;
    using ColorModelPtr = std::shared_ptr<ColorModel>;
}

class LayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LayerWidget(int index,
                         unblending::BlendMode* mode,
                         unblending::ColorModelPtr color_model,
                         QWidget *parent = nullptr);
    
    void set_layer_image_widget(LayerImageWidget* layer_image_widget);
    void set_layer_color_model_widget(LayerColorModelWidget* layer_color_model_widget);
    void initialize_blend_model_option();
    void initialize_control_widget();
    void insert_stretch();
    
    LayerImageWidget*      layer_image_widget_;
    LayerColorModelWidget* layer_color_model_widget_;
    
    void update_sliders();
    
    public slots:
    void update_color_model();
    void update_blend_mode();
    
private:
    const int index_;
    unblending::BlendMode*    mode_;
    unblending::ColorModelPtr color_model_;
    
    std::vector<QSlider*> sliders_;
    QComboBox*            combo_box_;
};

#endif // LAYERWIDGET_H
