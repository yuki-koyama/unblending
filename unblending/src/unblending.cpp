#include <unblending/unblending.hpp>
#include <unblending/image_processing.hpp>
#include <unblending/color_model.hpp>
#include <unblending/equations.hpp>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <thread>
#include <nlopt-util.hpp>
#include <timer.hpp>
#include <parallel-util.hpp>

//#define SPARCITY
//#define VERBOSE
//#define MATTE_ALPHA_FORCE_FIXED
//#define AKSOY_INITIAL_SOLUTION
//#define SECOND_LAYER_GRAY
//#define USE_ORIGINAL_HYPERPARAMETERS

namespace unblending
{
    using std::vector;
    
    struct OptimizationParameterSet
    {
        vector<ColorModelPtr> models;
        vector<CompOp>        comp_ops;
        vector<BlendMode>     modes;
        
        Vec3   target_color;
        VecX   lambda;
        double lo;
        double sigma;               // Weight for the sparcity term
        bool   use_sparcity;
        bool   use_minimum_alpha;
        bool   use_target_alphas;   // If true, the alternative constraint (Eq. 6) will be used instead of the unity constraint (Eq. 2).
        VecX   target_alphas;       // This will be used when "use_target_alphas" is true.
        
        vector<int> gray_layers;    // A list of gray layer indices. For example, if the second and fourth layers are to be gray, it looks like { 1, 3 }.
    };
    
    double objective_function(const vector<double> &x, vector<double>& grad, void* data)
    {
        const int num_layers = static_cast<int>(x.size() / 4);
        
        const OptimizationParameterSet& set = *static_cast<const OptimizationParameterSet*>(data);
        
        const VecX alphas = Eigen::Map<const VecX>(&x[0], num_layers);
        const VecX colors = Eigen::Map<const VecX>(&x[num_layers], num_layers * 3);
        
        const VecX constraint_vector = calculate_constraint_vector(alphas,
                                                                   colors,
                                                                   set.target_color,
                                                                   set.comp_ops,
                                                                   set.modes,
                                                                   set.use_target_alphas,
                                                                   set.target_alphas,
                                                                   set.gray_layers);
        
        const VecX derivative_of_unmixing_energy   = calculate_derivative_of_unmixing_energy(alphas,
                                                                                             colors,
                                                                                             set.models,
                                                                                             set.sigma,
                                                                                             set.use_sparcity,
                                                                                             set.use_minimum_alpha);
        const MatX derivative_of_constraint_vector = calculate_derivative_of_constraint_vector(alphas,
                                                                                               colors,
                                                                                               set.target_color,
                                                                                               set.comp_ops,
                                                                                               set.modes,
                                                                                               set.use_target_alphas,
                                                                                               set.target_alphas,
                                                                                               set.gray_layers);
        
        const VecX gradient = derivative_of_unmixing_energy + derivative_of_constraint_vector * (set.lo * constraint_vector - set.lambda);
        
        Eigen::Map<VecX>(&grad[0], num_layers * 4) = gradient;
        
        const double unmixing_energy = calculate_unmixing_energy_term(alphas,
                                                                      colors,
                                                                      set.models,
                                                                      set.sigma,
                                                                      set.use_sparcity,
                                                                      set.use_minimum_alpha);
        const double lagrange        = calculate_lagrange_term(constraint_vector, set.lambda);
        const double penalty         = calculate_penalty_term(constraint_vector, set.lo);
        
        return unmixing_energy + lagrange + penalty;
    }
    
    VecX find_initial_solution(const Vec3&                  target_color,
                               const vector<ColorModelPtr>& models)
    {
        const int num_layers = static_cast<int>(models.size());
        
        VecX x_initial = VecX::Zero(num_layers * 4);
        
#ifdef AKSOY_INITIAL_SOLUTION
        double min_distance  = DBL_MAX;
        int    closest_index = - 1;
        for (int index = 0; index < num_layers; ++ index)
        {
            double distance = models[index]->calculate_distance(target_color);
            if (min_distance > distance)
            {
                min_distance  = distance;
                closest_index = index;
            }
        }
        
        // Set the alpha value of the closest layer as 1.0 (and keep those of the other layers as 0.0)
        x_initial(closest_index) = 1.0;
        
        for (int index = 0; index < num_layers; ++ index)
        {
            x_initial.segment<3>(num_layers + index * 3) = (index == closest_index) ? target_color : models[index]->get_representative_color();
            for (int i : { 0, 1, 2}) { x_initial(num_layers + index * 3 + i) = crop_value(x_initial(num_layers + index * 3 + i)); }
        }
#else
        x_initial.segment(0, num_layers) = VecX::Constant(num_layers, 0.50);
        for (int index = 0; index < num_layers; ++ index)
        {
            x_initial.segment<3>(num_layers + index * 3) = models[index]->get_representative_color();
        }
#endif
        
        return x_initial;
    }
    
    VecX solve_per_pixel_optimization(const Vec3&                  target_color,
                                      const vector<ColorModelPtr>& models,
                                      const vector<CompOp>&        comp_ops,
                                      const vector<BlendMode>&     modes,
                                      const bool                   is_for_refinement       = false,
                                      const bool                   has_opaque_background   = true,
                                      const VecX&                  initial_colors          = VecX(),
                                      const VecX&                  target_alphas           = VecX(),
                                      const bool                   force_smooth_background = false,
                                      const Vec3&                  target_background_color = Vec3())
    {
        const int num_layers = static_cast<int>(models.size());
        
#ifdef SECOND_LAYER_GRAY
        const vector<int> gray_layers = { 1 };
#else
        const vector<int> gray_layers;
#endif
        
        VecX upper = VecX::Constant(num_layers * 4, 1.0);
        VecX lower = VecX::Constant(num_layers * 4, 0.0);
        
#ifdef MATTE_ALPHA_FORCE_FIXED
        if (is_for_refinement)
        {
            upper.segment(0, num_layers) = target_alphas;
            lower.segment(0, num_layers) = target_alphas;
        }
#endif
        
        // Find an initial solution
        VecX x = find_initial_solution(target_color, models);
        if (is_for_refinement)
        {
            x.segment(0, num_layers)              = target_alphas;
            x.segment(num_layers, num_layers * 3) = initial_colors;
        }
        
        // Enforce background opacity
        if (has_opaque_background)
        {
            lower(0) = 1.0;
            x(0)     = 1.0;
        }
        
        // Enforce background smoothness
        if (force_smooth_background)
        {
            assert(has_opaque_background);
            
            upper.segment<3>(num_layers) = target_background_color;
            lower.segment<3>(num_layers) = target_background_color;
            x.segment<3>(num_layers)     = target_background_color;
        }
        
        const int num_alpha_constraints = is_for_refinement ? num_layers : 1;
        const int num_constraints       = 3 + num_alpha_constraints + 3 * gray_layers.size();
        
#ifdef USE_ORIGINAL_HYPERPARAMETERS
        constexpr double gamma         = 0.25;
        constexpr double epsilon       = 1e-05;
        constexpr double local_epsilon = 1e-05;
        constexpr double beta          = 10.0;
        constexpr double initial_lo    = 0.1;
#else
        constexpr double gamma         = 0.25;
        constexpr double epsilon       = 5e-03;
        constexpr double local_epsilon = 5e-03;
        constexpr double beta          = 10.0;
        constexpr double initial_lo    = 100.0;
#endif
        
        OptimizationParameterSet set;
        set.models            = models;
        set.comp_ops          = comp_ops;
        set.modes             = modes;
        set.lambda            = VecX::Constant(num_constraints, 0.0);
        set.lo                = initial_lo;
        set.target_color      = target_color;
        set.sigma             = 10.0;
        set.target_alphas     = target_alphas;
#ifdef SPARCITY
        set.use_sparcity      = !is_for_refinement;
#else
        set.use_sparcity      = false;
#endif
        set.use_minimum_alpha = !is_for_refinement;
        set.use_target_alphas = is_for_refinement;
        set.gray_layers       = gray_layers;
        
        int count = 0;
        constexpr int max_count = 20;
        while (true)
        {
            const VecX x_new = nloptutil::solve(x, upper, lower, objective_function, nlopt::LD_LBFGS, &set, false, 1000, local_epsilon, local_epsilon);
            const VecX g     = calculate_constraint_vector(x,
                                                           set.target_color,
                                                           comp_ops,
                                                           modes,
                                                           set.use_target_alphas,
                                                           target_alphas,
                                                           gray_layers);
            const VecX g_new = calculate_constraint_vector(x_new,
                                                           set.target_color,
                                                           comp_ops,
                                                           modes,
                                                           set.use_target_alphas,
                                                           target_alphas,
                                                           gray_layers);
            
            set.lambda -= set.lo * g_new;
            if (g_new.norm() > gamma * g.norm()) set.lo *= beta;
            
            const bool is_unchanged = (x_new - x).norm() < epsilon;
            const bool is_satisfied = g_new.norm() < epsilon;
            
            x = x_new;
            
            if ((is_unchanged && is_satisfied) || count > max_count) break;
            
            ++ count;
        }
        
#ifdef VERBOSE
        const VecX g = calculate_constraint_vector(x,
                                                   set.target_color,
                                                   comp_ops,
                                                   modes,
                                                   set.use_target_alphas,
                                                   target_alphas,
                                                   gray_layers);
        if (g.norm() > 0.01)
        {
            const auto comp = composite_layers(x.segment(0, num_layers), x.segment(num_layers, num_layers * 3), comp_ops, modes);
            
            std::cout << "==== Failed to satisfy hard constraints ====" << std::endl;
            std::cout << "count  : " << count << std::endl;
            std::cout << "target : " << target_color.transpose().format(get_format()) << std::endl;
            std::cout << "comp.  : " << comp.transpose().format(get_format()) << std::endl;
            for (int i = 0; i < num_layers; ++ i)
            {
                std::cout << "layer " << i << ": " << x.segment<3>(num_layers + i * 3).transpose().format(get_format()) << ", " << x(i);
                if (is_for_refinement)
                {
                    std::cout << " \t (" << target_alphas(i) << ")";
                }
                std::cout << std::endl;
            }
            std::cout << "g.norm : " << g.norm() << std::endl;
            std::cout << "g      : " << g.transpose().format(get_format()) << std::endl;
        }
#endif
        
        return x;
    }
    
    VecX normalize_alphas(const VecX&           alphas,
                          const vector<CompOp>& comp_ops)
    {
        bool is_all_plus        = true;
        bool is_all_source_over = true;
        for (auto& comp_op : comp_ops)
        {
            is_all_plus        = is_all_plus        && comp_op.is_plus();
            is_all_source_over = is_all_source_over && comp_op.is_source_over();
        }
        
        assert(is_all_plus || is_all_source_over);
        
        constexpr double epsilon = 1e-05;
        
        const bool has_opaque_background = std::abs(alphas(0) - 1.0) < epsilon;
        
        if (is_all_source_over && has_opaque_background) { return alphas; }
        if (is_all_plus) { return alphas / alphas.sum(); }
        
        // This line is never performed
        return VecX::Zero(alphas.rows());
    }
    
    vector<ColorImage> perform_matte_refinement(const ColorImage&         image,
                                                const vector<ColorImage>& layers,
                                                const vector<LayerInfo>&  layer_infos,
                                                const bool                has_opaque_background,
                                                const bool                force_smooth_background,
                                                const int                 target_concurrency)
    {
        timer::Timer timer("perform_matte_refinement");
        
        const vector<ColorModelPtr> models                  = extract_color_models           (layer_infos);
        const vector<CompOp>        comp_ops                = extract_comp_ops               (layer_infos);
        const vector<BlendMode>     modes                   = extract_blend_modes            (layer_infos);
        
        assert(layers.size() == models.size());
        
        const int number = static_cast<int>(layers.size());
        const int width  = image.width();
        const int height = image.height();
        const int radius = 60 * std::min(width, height) / 1000;
        
        constexpr double epsilon = 1e-04;
        
        // Apply guided filter
        vector<Image> refined_alphas;
        for (const ColorImage& layer : layers)
        {
            const Image& alpha         = layer.get_a();
            const Image  refined_alpha = apply_guided_filter(alpha, image, radius, epsilon);
            
            refined_alphas.push_back(refined_alpha);
        }
        
        // Crop alphas into [0, 1]
        for (int x = 0; x < width; ++ x) for (int y = 0; y < height; ++ y)
        {
            for (int i = 0; i < number; ++ i)
            {
                refined_alphas[i].set_pixel(x, y, crop_value(refined_alphas[i].get_pixel(x, y)));
            }
        }
        
        // Normalize alphas such that the composited alpha becomes one for each pixel
        for (int x = 0; x < width; ++ x) for (int y = 0; y < height; ++ y)
        {
            VecX alphas = VecX(number);
            for (int i = 0; i < number; ++ i)
            {
                alphas(i) = refined_alphas[i].get_pixel(x, y);
            }
            
            alphas = normalize_alphas(alphas, comp_ops);
            
            for (int i = 0; i < number; ++ i)
            {
                refined_alphas[i].set_pixel(x, y, alphas(i));
            }
        }
        
        // Smooth background
        ColorImage smoothed_background(width, height);
        if (force_smooth_background)
        {
            assert(has_opaque_background);
            smoothed_background.set_r(apply_guided_filter(layers[0].get_r(), image, radius, epsilon));
            smoothed_background.set_g(apply_guided_filter(layers[0].get_g(), image, radius, epsilon));
            smoothed_background.set_b(apply_guided_filter(layers[0].get_b(), image, radius, epsilon));
        }
        
        // Perform optimization
        vector<ColorImage> refined_layers(number, ColorImage(width, height));
        auto per_pixel_process = [&](int x, int y)
        {
            VecX initial_colors(number * 3);
            VecX target_alphas(number);
            for (int i = 0; i < number; ++ i)
            {
                initial_colors.segment<3>(i * 3) = layers[i].get_rgb(x, y);
                target_alphas(i)                 = refined_alphas[i].get_pixel(x, y);
            }
            
            if (force_smooth_background)
            {
                initial_colors.segment<3>(0) = crop_vec3(smoothed_background.get_rgb(x, y));
            }
            
            const Vec3 pixel_color = image.get_rgb(x, y);
            const VecX solution = solve_per_pixel_optimization(pixel_color,
                                                               models,
                                                               comp_ops,
                                                               modes,
                                                               true,
                                                               has_opaque_background,
                                                               initial_colors,
                                                               target_alphas,
                                                               force_smooth_background,
                                                               crop_vec3(smoothed_background.get_rgb(x, y)));
            
            for (int index = 0; index < number; ++ index)
            {
                refined_layers[index].set_rgba(x, y, solution.segment<3>(number + index * 3), solution(index));
            }
        };
        
        parallelutil::parallel_for_2d(width, height, per_pixel_process, target_concurrency);
        
        return refined_layers;
    }
    
    vector<ColorImage> compute_color_unmixing(const ColorImage&        image,
                                              const vector<LayerInfo>& layer_infos,
                                              const bool               has_opaque_background,
                                              const int                target_concurrency)
    {
        timer::Timer timer("compute_color_unmixing");
        
        const vector<ColorModelPtr> models                  = extract_color_models           (layer_infos);
        const vector<CompOp>        comp_ops                = extract_comp_ops               (layer_infos);
        const vector<BlendMode>     modes                   = extract_blend_modes            (layer_infos);
        
        const int width      = image.width();
        const int height     = image.height();
        const int num_layers = static_cast<int>(models.size());
        
        vector<ColorImage> layers(num_layers, ColorImage(width, height));
        
        auto per_pixel_process = [&](int x, int y)
        {
            const Vec3 pixel_color = image.get_rgb(x, y);
            const VecX solution = solve_per_pixel_optimization(pixel_color,
                                                               models,
                                                               comp_ops,
                                                               modes,
                                                               false,
                                                               has_opaque_background);
            
            for (int index = 0; index < num_layers; ++ index)
            {
                layers[index].set_rgba(x, y, solution.segment<3>(num_layers + index * 3), solution(index));
            }
        };
        
        parallelutil::parallel_for_2d(width, height, per_pixel_process, target_concurrency);
        
        return layers;
    }
    
    ColorImage composite_layers(const vector<ColorImage>& layers,
                                const vector<CompOp>&     comp_ops,
                                const vector<BlendMode>&  modes)
    {
        const int number = static_cast<int>(layers.size());
        const int width  = layers.front().width();
        const int height = layers.front().height();
        
        ColorImage composited_image(width, height);
        
        for (int x = 0; x < width; ++ x) for (int y = 0; y < height; ++ y)
        {
            VecX alphas(number);
            VecX colors(number * 3);
            
            for (int index = 0; index < number; ++ index)
            {
                alphas(index)                = layers[index].get_rgba(x, y)(3);
                colors.segment<3>(index * 3) = layers[index].get_rgb(x, y);
            }
            
            const Vec4 composited_color = composite_layers(alphas, colors, comp_ops, modes);
            composited_image.set_rgba(x, y, composited_color);
        }
        
        return composited_image;
    }
}
