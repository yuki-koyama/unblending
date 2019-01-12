#ifndef UNBLENDING_HPP
#define UNBLENDING_HPP

#include <string>
#include <unblending/common.hpp>
#include <unblending/layer_info.hpp>
#include <unblending/color_model.hpp>
#include <unblending/image_processing.hpp>

namespace unblending
{
    /// \brief Compute the main unblending optimization
    std::vector<ColorImage> compute_color_unmixing(const ColorImage& image,
                                                   const std::vector<LayerInfo>& layer_infos,
                                                   const bool has_opaque_background,
                                                   const int target_concurrency = 0);
    
    /// \brief Compute the sub unblending optimization for refinement
    std::vector<ColorImage> perform_matte_refinement(const ColorImage&              image,
                                                     const std::vector<ColorImage>& layers,
                                                     const std::vector<LayerInfo>&  layer_infos,
                                                     const bool                     has_opaque_background,
                                                     const bool                     force_smooth_background,
                                                     const int                      target_concurrency = 0);
    
    /// \brief Calculate a blended image from multiple layers by color blending
    ColorImage composite_layers(const std::vector<ColorImage>& layers,
                                const std::vector<CompOp>&     comp_ops,
                                const std::vector<BlendMode>&  modes);
    
    /// \brief Export layers as image files
    void export_layers(const std::vector<ColorImage>& layers,
                       const std::string&             output_directory_path,
                       const std::string&             file_name_prefix,
                       const bool                     with_alpha_channel     = false,
                       const bool                     with_blend_mode_suffix = false,
                       const std::vector<LayerInfo>&  layer_infos            = {});
    
    /// \brief Export color models as image files
    void export_models(const std::vector<ColorModelPtr>& models,
                       const std::string& output_directory_path,
                       const std::string& file_name_prefix);
    
    /// \brief Export layer infos as a JSON file
    void export_layer_infos(const std::vector<LayerInfo>& layer_infos,
                            const std::string& output_directory_path);
    
    /// \brief Import layer infos from a JSON file
    std::vector<LayerInfo> import_layer_infos(const std::string& input_file_path);
}

#endif // UNBLENDING_HPP
