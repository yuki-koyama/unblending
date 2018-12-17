#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <iostream>
#include <unblending/unblending.hpp>
#include <unblending/equations.hpp>
#include <cxxopts.hpp>

using namespace unblending;

int main(int argc, char** argv)
{
    cxxopts::Options options("unblending-cli", " - A command line interface (CLI) for using the ``unblending'' library.");
    
    options.add_options()("o,outdir", "Path to the output directory", cxxopts::value<std::string>()->default_value("./out"));
    options.add_options()("w,width", "Target width (pixels) of the output image (default: original resolution)", cxxopts::value<int>());
    options.add_options()("h,help", "Print help");
    options.add_options()("e,explicit-mode-names", "Append blend mode names to output image file names");
    options.add_options()("v,verbose-export", "Export intermediate files as well as final outcomes");
    options.add_options()("input-image-path", "Path to the input image (png or jpg)", cxxopts::value<std::string>());
    options.add_options()("layer-infos-path", "Path to the layer infos (json)", cxxopts::value<std::string>());
    
    options.parse_positional({ "input-image-path", "layer-infos-path" });
    options.positional_help("<input-image-path> <layer-infos-path>");
    options.show_positional_help();
    
    const auto parse_result = options.parse(argc, argv);
    
    if (parse_result.count("input-image-path") != 1 ||
        parse_result.count("layer-infos-path") != 1 ||
        parse_result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    
    const std::string image_file_path       = parse_result["input-image-path"].as<std::string>();
    const std::string layer_infos_path      = parse_result["layer-infos-path"].as<std::string>();
    const std::string output_directory_path = parse_result["outdir"].as<std::string>();
    const bool        use_explicit_name     = parse_result.count("explicit-mode-names");
    const bool        export_verbosely      = parse_result.count("verbose-export");
    
    if (std::system(("mkdir -p " + output_directory_path).c_str()) < 0) { exit(1); };
    
    // Import the target image and resize it if a target width is specified
    const ColorImage original_image = [&]()
    {
        const ColorImage image(image_file_path);
        const bool use_target_width = (parse_result["width"].count() == 1);
        return use_target_width ? image.get_scaled_image(parse_result["width"].as<int>()) : image;
    }();
    
    // Prepare layer infos
    std::vector<LayerInfo> layer_infos = import_layer_infos(layer_infos_path);
    
    // TODO: Allow users to specify these variables
    constexpr bool has_opaque_background   = true;
    constexpr bool force_smooth_background = true;
    
    // Compute color unmixing to obtain an initial result
    const std::vector<ColorImage> layers = compute_color_unmixing(original_image, layer_infos, has_opaque_background);
    
    // Perform post processing steps
    const std::vector<ColorImage> refined_layers = perform_matte_refinement(original_image, layers, layer_infos, has_opaque_background, force_smooth_background);
    
    // Export layers
    if (export_verbosely) { export_layers(layers, output_directory_path, "non-smoothed-layer", true, use_explicit_name, layer_infos); }
    export_layers(refined_layers, output_directory_path, "layer", export_verbosely, use_explicit_name, layer_infos);
    
    // Export the original image
    if (export_verbosely) { original_image.save(output_directory_path + "/input.png"); }
    
    // Export the composited image
    const auto modes    = extract_blend_modes(layer_infos);
    const auto comp_ops = extract_comp_ops(layer_infos);
    
    const ColorImage composited_image = composite_layers(layers, comp_ops, modes);
    if (export_verbosely) { composited_image.save(output_directory_path + "/non-smoothed-recomposited.png"); }
    
    const ColorImage refined_composited_image = composite_layers(refined_layers, comp_ops, modes);
    if (export_verbosely) { refined_composited_image.save(output_directory_path + "/recomposited.png"); }
    
    // Export visualizations of color models
    if (export_verbosely)
    {
        const std::vector<ColorModelPtr> models = extract_color_models(layer_infos);
        export_models(models, output_directory_path, "model");
    }
    
    // Export layer infos
    export_layer_infos(layer_infos, output_directory_path);
    
    return 0;
}
