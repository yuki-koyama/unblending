#include "core.h"
#include <cassert>
#include <unblending/unblending.hpp>

#define REFINE

using unblending::GaussianColorModel;
using unblending::LayerInfo;
using unblending::CompOp;
using unblending::BlendMode;
using unblending::ColorModelPtr;
using unblending::Vec3;
using unblending::Mat3;
using unblending::ColorImage;

namespace
{
    inline std::shared_ptr<GaussianColorModel> instantiate_default_color_model()
    {
        return std::make_shared<GaussianColorModel>(Eigen::Vector3d::Constant(0.5), 10.0 * Mat3::Identity());
    }
    
    constexpr int  image_display_width     = 100;
    constexpr bool has_opaque_background   = true;
    constexpr bool force_smooth_background = true;
}

Core::Core()
{
    // Set random seed
    std::srand(static_cast<unsigned>(time(0)));
    
    // Set target image (hard coding)
    const std::string image_file_path = "/Users/koyama/Desktop/input.png";
    import_image(image_file_path);
    
    // Set default color models
    models_ = std::vector<ColorModelPtr>();
    models_.push_back(instantiate_default_color_model());
    models_.push_back(instantiate_default_color_model());
    
    modes_.clear(); modes_.resize(models_.size(), default_mode_);
}

void Core::create_color_model()
{
    models_.push_back(instantiate_default_color_model());
    
    modes_.resize(models_.size(), default_mode_);
}

void Core::delete_color_model()
{
    assert(!models_.empty());
    models_.pop_back();
    
    modes_.resize(models_.size(), default_mode_);
}

void Core::decompose_image()
{
    assert(image_ != nullptr);
    assert(!models_.empty());
    assert(models_.size() == modes_.size());
    
    std::vector<LayerInfo> layer_infos;
    for (int i = 0; i < models_.size(); ++ i)
    {
        layer_infos.push_back(LayerInfo{ dominant_comp_op_, modes_[i], models_[i] });
    }
    
    layers_ = unblending::compute_color_unmixing(*image_, layer_infos, has_opaque_background);
#ifdef REFINE
    layers_ = unblending::perform_matte_refinement(*image_, layers_, layer_infos, has_opaque_background, force_smooth_background);
#endif
}

std::vector<ColorImage> Core::generate_color_model_visualizations() const
{
    std::vector<ColorImage> visualizations;
    for (const auto model : models_)
    {
        visualizations.push_back(model->generate_visualization());
    }
    return visualizations;
}

void Core::import_image(const std::string& image_file_path)
{
    original_image_ = std::make_shared<ColorImage>(image_file_path);
    image_          = std::make_shared<ColorImage>(original_image_->get_scaled_image(image_display_width));
    
    original_image_->make_fully_opaque();
    image_         ->make_fully_opaque();
}

void Core::export_files(const std::string& output_dir_path) const
{
    assert(image_ != nullptr);
    assert(!models_.empty());
    assert(models_.size() == modes_.size());
    
    std::vector<LayerInfo> layer_infos;
    for (int i = 0; i < models_.size(); ++ i)
    {
        layer_infos.push_back(LayerInfo{ dominant_comp_op_, modes_[i], models_[i] });
    }
    
    const auto layers         = compute_color_unmixing(*original_image_, layer_infos, has_opaque_background);
    const auto refined_layers = unblending::perform_matte_refinement(*original_image_, layers, layer_infos, has_opaque_background, force_smooth_background);
    
    unblending::export_layers(layers, output_dir_path, "layer");
    unblending::export_layers(refined_layers, output_dir_path, "refined");
    unblending::export_models(models_, output_dir_path, "model");
    unblending::export_layer_infos(layer_infos, output_dir_path);
    
    original_image_->save(output_dir_path + "/original.png");
}
