#ifndef CORE_H
#define CORE_H

#include <memory>
#include <unblending/image_processing.hpp>
#include <unblending/layer_info.hpp>

namespace unblending
{
    class ColorModel;
    using ColorModelPtr = std::shared_ptr<ColorModel>;
}

class Core
{
public:
    Core();
    
    static Core& GetInstance()
    {
        static Core core;
        return core;
    }
    
    const std::shared_ptr<unblending::ColorImage> get_image() const
    {
        return image_;
    }
    
    const std::vector<unblending::ColorImage>& get_layers() const
    {
        return layers_;
    }
    
    const std::vector<unblending::ColorModelPtr>& get_models() const
    {
        return models_;
    }
    
    std::vector<unblending::ColorModelPtr>& get_models()
    {
        return models_;
    }
    
    const std::vector<unblending::BlendMode>& get_modes() const
    {
        return modes_;
    }
    
    std::vector<unblending::BlendMode>& get_modes()
    {
        return modes_;
    }
    
    void import_image(const std::string& image_file_path);
    void export_files(const std::string& output_dir_path) const;
    void create_color_model();
    void delete_color_model();
    
    std::vector<unblending::ColorImage> generate_color_model_visualizations() const;
    
    void decompose_image();
    
private:
    unblending::CompOp    dominant_comp_op_ = unblending::CompOp::SourceOver();
    unblending::BlendMode default_mode_     = unblending::BlendMode::Normal;
    
    std::vector<unblending::BlendMode>      modes_;
    std::vector<unblending::ColorModelPtr>  models_;
    std::shared_ptr<unblending::ColorImage> image_;
    std::shared_ptr<unblending::ColorImage> original_image_;
    std::vector<unblending::ColorImage>     layers_;
};

#endif // CORE_H
