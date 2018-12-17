#ifndef LAYER_INFO_HPP
#define LAYER_INFO_HPP

#include <unblending/common.hpp>
#include <unblending/blend_mode.hpp>
#include <unblending/comp_op.hpp>

namespace unblending
{
    struct LayerInfo
    {
        const CompOp        comp_op;
        const BlendMode     blend_mode;
        const ColorModelPtr color_model;
    };
    
    //////////////////////////////////////////////////////////////
    // Utility functions
    //////////////////////////////////////////////////////////////
    
    inline std::vector<CompOp> extract_comp_ops(const std::vector<LayerInfo>& layer_infos)
    {
        std::vector<CompOp> comp_ops;
        for (auto& layer_info : layer_infos) { comp_ops.push_back(layer_info.comp_op); }
        return comp_ops;
    }
    
    inline std::vector<BlendMode> extract_blend_modes(const std::vector<LayerInfo>& layer_infos)
    {
        std::vector<BlendMode> modes;
        for (auto& layer_info : layer_infos) { modes.push_back(layer_info.blend_mode); }
        return modes;
    }
    
    inline std::vector<ColorModelPtr> extract_color_models(const std::vector<LayerInfo>& layer_infos)
    {
        std::vector<ColorModelPtr> models;
        for (auto& layer_info : layer_infos) { models.push_back(layer_info.color_model); }
        return models;
    }
}

#endif // LAYER_INFO_HPP
