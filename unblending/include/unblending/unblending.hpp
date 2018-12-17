#ifndef UNBLENDING_HPP
#define UNBLENDING_HPP

#include <string>
#include <unblending/common.hpp>
#include <unblending/layer_info.hpp>
#include <unblending/color_model.hpp>
#include <unblending/image_processing.hpp>

namespace unblending
{
    std::vector<ColorImage> compute_color_unmixing(const ColorImage& image,
                                                   const std::vector<LayerInfo>& layer_infos,
                                                   const bool has_opaque_background,
                                                   const int target_concurrency = 0);
    
    std::vector<ColorImage> perform_matte_refinement(const ColorImage&              image,
                                                     const std::vector<ColorImage>& layers,
                                                     const std::vector<LayerInfo>&  layer_infos,
                                                     const bool                     has_opaque_background,
                                                     const bool                     force_smooth_background,
                                                     const int                      target_concurrency = 0);
    
    ColorImage composite_layers(const std::vector<ColorImage>& layers,
                                const std::vector<CompOp>&     comp_ops,
                                const std::vector<BlendMode>&  modes);
    
    void export_layers(const std::vector<ColorImage>& layers,
                       const std::string&             output_directory_path,
                       const std::string&             file_name_prefix,
                       const bool                     with_alpha_channel     = false,
                       const bool                     with_blend_mode_suffix = false,
                       const std::vector<LayerInfo>&  layer_infos            = {});
    
    void export_models(const std::vector<ColorModelPtr>& models,
                       const std::string& output_directory_path,
                       const std::string& file_name_prefix);
    
    void export_layer_infos(const std::vector<LayerInfo>& layer_infos,
                            const std::string& output_directory_path);
    
    std::vector<LayerInfo> import_layer_infos(const std::string& input_file_path);
}

#endif // UNBLENDING_HPP
