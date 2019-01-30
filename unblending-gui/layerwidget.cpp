#include "layerwidget.h"
#include <array>
#include <QBoxLayout>
#include <QColorDialog>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <unblending/color_model.hpp>
#include <unblending/blend_mode.hpp>
#include "layerimagewidget.h"
#include "layercolormodelwidget.h"

// When FULL_CONTROL macro is defined, the system provides finer control of color distributions
// #define FULL_CONTROL

namespace
{
    inline double convert(const QSlider* slider)
    {
        return static_cast<double>(slider->value() - slider->minimum()) / static_cast<double>(slider->maximum() - slider->minimum());
    }
    
    inline void set(double value, QSlider* slider)
    {
        value = std::max(std::min(value, 1.0), 0.0);
        slider->setValue(static_cast<int>((slider->maximum() - slider->minimum()) * value + slider->minimum()));
    }
}

LayerWidget::LayerWidget(int index, unblending::BlendMode* mode, unblending::ColorModelPtr color_model, QWidget *parent) :
QWidget(parent),
index_(index),
mode_(mode),
color_model_(color_model)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    this->setLayout(layout);
}

void LayerWidget::set_layer_image_widget(LayerImageWidget* layer_image_widget)
{
    layer_image_widget_ = layer_image_widget;
    this->layout()->addWidget(layer_image_widget);
}

void LayerWidget::set_layer_color_model_widget(LayerColorModelWidget* layer_color_model_widget)
{
    layer_color_model_widget_ = layer_color_model_widget;
    this->layout()->addWidget(layer_color_model_widget);
    this->layout()->setAlignment(layer_color_model_widget, Qt::AlignHCenter);
}

void LayerWidget::initialize_blend_model_option()
{
    const auto& list = unblending::get_blend_mode_list();
    
    combo_box_ = new QComboBox(this);
    for (auto& mode : list)
    {
        combo_box_->addItem(QString::fromStdString(unblending::retrieve_name(mode)));
    }
    combo_box_->setCurrentIndex(static_cast<int>(*mode_));
    this->layout()->addWidget(combo_box_);
    
    connect(combo_box_, SIGNAL(activated(int)), this, SLOT(update_blend_mode()));
}

void LayerWidget::initialize_control_widget()
{
#ifdef FULL_CONTROL
    constexpr int num_sliders = 9;
    constexpr std::array<const char*, num_sliders> labels =
    {{
        "Red", "Green", "Blue", "s00", "s11", "s22", "s01", "s02", "s12"
    }};
#else
    constexpr int num_sliders = 4;
    constexpr std::array<const char*, num_sliders> labels = {{ "Red", "Green", "Blue", "Variance" }};
#endif
    
    for (int i = 0; i < num_sliders; ++ i)
    {
        // Add a label for a new slider
        this->layout()->addWidget(new QLabel(QString(labels[i]), this));
        
        // Add a new slider
        QSlider* slider = new QSlider(Qt::Horizontal, this);
        this->layout()->addWidget(slider);
        sliders_.push_back(slider);
        
        // When RGB sliders are added, put a horizontal line
        if (i == 2)
        {
            auto* line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            this->layout()->addWidget(line);
        }
    }
    update_sliders();
    for (int i = 0; i < num_sliders; ++ i)
    {
        connect(sliders_[i], SIGNAL(valueChanged(int)), this, SLOT(update_color_model()));
    }
}

void LayerWidget::insert_stretch()
{
    static_cast<QVBoxLayout*>(this->layout())->addStretch();
}

namespace
{
    constexpr double s = 3.2;
    constexpr double offset = 1e-03;
}

void LayerWidget::update_sliders()
{
    const auto* model = static_cast<const unblending::GaussianColorModel*>(color_model_.get());
    set(model->get_mu()(0), sliders_[0]);
    set(model->get_mu()(1), sliders_[1]);
    set(model->get_mu()(2), sliders_[2]);
    const auto sigma = model->get_sigma();
#ifdef FULL_CONTROL
    set(std::pow(sigma(0, 0) - offset, 1.0 / s), sliders_[3]);
    set(std::pow(sigma(1, 1) - offset, 1.0 / s), sliders_[4]);
    set(std::pow(sigma(2, 2) - offset, 1.0 / s), sliders_[5]);
    set(sigma(0, 1) * 10.0 + 0.5, sliders_[6]);
    set(sigma(1, 2) * 10.0 + 0.5, sliders_[7]);
    set(sigma(2, 0) * 10.0 + 0.5, sliders_[8]);
#else
    assert((sigma / sigma(0, 0)).isIdentity() && "Sigma is not homogeneous.");
    set(std::pow(sigma(0, 0) - offset, 1.0 / s), sliders_[3]);
#endif
}

void LayerWidget::update_color_model()
{
    const double r   = convert(sliders_[0]);
    const double g   = convert(sliders_[1]);
    const double b   = convert(sliders_[2]);
#ifdef FULL_CONTROL
    const double s11 = std::pow(convert(sliders_[3]), s) + offset;
    const double s22 = std::pow(convert(sliders_[4]), s) + offset;
    const double s33 = std::pow(convert(sliders_[5]), s) + offset;
    const double s12 = 0.1 * (convert(sliders_[6]) - 0.5);
    const double s23 = 0.1 * (convert(sliders_[7]) - 0.5);
    const double s31 = 0.1 * (convert(sliders_[8]) - 0.5);
#else
    const double s11 = std::pow(convert(sliders_[3]), s) + offset;
    const double s22 = std::pow(convert(sliders_[3]), s) + offset;
    const double s33 = std::pow(convert(sliders_[3]), s) + offset;
    const double s12 = 0.0;
    const double s23 = 0.0;
    const double s31 = 0.0;
#endif
    
    const Eigen::Vector3d mu(r, g, b);
    const Eigen::Matrix3d sigma = [&](){ Eigen::Matrix3d sigma; sigma << s11, s12, s31, s12, s22, s23, s31, s23, s33; return sigma; }();
    static_cast<unblending::GaussianColorModel*>(color_model_.get())->set_mu(mu);
    static_cast<unblending::GaussianColorModel*>(color_model_.get())->set_sigma(sigma);
    
    layer_color_model_widget_->copy_and_set_image(color_model_->generate_visualization());
}

void LayerWidget::update_blend_mode()
{
    const int index = combo_box_->currentIndex();
    *mode_ = static_cast<unblending::BlendMode>(index);
}
