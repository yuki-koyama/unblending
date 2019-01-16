#include <unblending/equations.hpp>
#include <unblending/color_model.hpp>

//#define AKSOY_PERFORMANCE_TEST_OPTION

namespace unblending
{
    using std::vector;
    
    Vec4 composite_two_layers(const Vec3&     c_s,
                              const Vec3&     c_d,
                              const double    a_s,
                              const double    a_d,
                              const CompOp&   comp_op,
                              const BlendMode mode,
                              const bool      crop)
    {
        const double& X = comp_op.X;
        const double& Y = comp_op.Y;
        const double& Z = comp_op.Z;
        
        constexpr double epsilon = 1e-12;
        
        const double a     = X * a_s * a_d + Y * a_s * (1.0 - a_d) + Z * a_d * (1.0 - a_s);
        const Vec3   f     = blend(c_s, c_d, mode);
        const Vec3   c_pre = f * a_s * a_d + Y * a_s * (1.0 - a_d) * c_s + Z * a_d * (1.0 - a_s) * c_d;
        const Vec3   c     = (a > epsilon) ? c_pre / a : c_pre;
        
        assert(!std::isnan(c.sum()));
        
        return crop ? crop_vec4((Vec4() << c, a).finished()) : (Vec4() << c, a).finished();
    }
    
    Vec4 composite_layers(const VecX&              alphas,
                          const VecX&              colors,
                          const vector<CompOp>&    comp_ops,
                          const vector<BlendMode>& modes,
                          const bool               crop)
    {
#ifdef AKSOY_PERFORMANCE_TEST_OPTION
        const int num_layers = alphas.rows();
        Vec3   sum_color = Vec3::Zero();
        double sum_alpha = 0.0;
        for (int i = 0; i < num_layers; ++ i)
        {
            sum_color += alphas[i] * colors.segment<3>(i * 3);
            sum_alpha += alphas[i];
        }
        return (Vec4() << sum_color, sum_alpha).finished();
#else
        const int num_layers = static_cast<int>(alphas.rows());
        
        assert(num_layers == comp_ops.size());
        assert(num_layers == modes   .size());
        
        Vec3   color = colors.segment<3>(0);
        double alpha = alphas(0);
        
        for (int index = 1; index < num_layers; ++ index)
        {
            const Vec4 x = composite_two_layers(colors.segment<3>(index * 3),
                                                color,
                                                alphas(index),
                                                alpha,
                                                comp_ops[index],
                                                modes[index],
                                                crop);
            color = x.segment<3>(0);
            alpha = x(3);
        }
        
        return (Vec4() << color, alpha).finished();
#endif
    }
    
    double calculate_lagrange_term(const VecX& constraint_vector,
                                   const VecX& lambda)
    {
        return - lambda.transpose() * constraint_vector;
    }
    
    double calculate_penalty_term(const VecX&  constraint_vector,
                                  const double rho)
    {
        return 0.5 * rho * constraint_vector.squaredNorm();
    }
    
    double calculate_unmixing_energy_term(const VecX&                  alphas,
                                          const VecX&                  colors,
                                          const vector<ColorModelPtr>& models,
                                          const double                 sigma,
                                          const bool                   use_sparcity,
                                          const bool                   use_minimum_alpha)
    {
        const int number_of_layers = static_cast<int>(alphas.rows());
        
        // Main term
        double energy = 0.0;
        for (int index = 0; index < number_of_layers; ++ index)
        {
            energy += alphas[index] * models[index]->calculate_distance(colors.segment<3>(index * 3));
        }
        
        // Sparcity term
        if (use_sparcity) { energy += sigma * ((alphas.sum() / alphas.squaredNorm()) - 1.0); }
        
        // Minimum alpha term
        if (use_minimum_alpha) { constexpr double epsilon = 0.01; energy += epsilon * alphas.sum(); }
        
        return energy;
    }
    
    VecX calculate_constraint_vector(const VecX&              alphas,
                                     const VecX&              colors,
                                     const Vec3&              target_color,
                                     const vector<CompOp>&    comp_ops,
                                     const vector<BlendMode>& modes,
                                     const bool               use_target_alphas,
                                     const VecX&              target_alphas,
                                     const vector<int>&       gray_layers)
    {
        const Vec4 composited_color = composite_layers(alphas, colors, comp_ops, modes, false);
        const Vec3 g_color          = composited_color.segment<3>(0) - target_color;
        
        const int  num_layers            = static_cast<int>(alphas.rows());
        const int  num_gray_layers       = static_cast<int>(gray_layers.size());
        const int  num_alpha_constraints = use_target_alphas ? num_layers : 1;
        
        VecX constraints = VecX(3 + num_alpha_constraints + 3 * num_gray_layers);
        constraints.segment<3>(0) = g_color;
        
        // Alpha constraints
        if (use_target_alphas)
        {
            const VecX g_alpha = alphas - target_alphas;
            constraints.segment(3, num_alpha_constraints) = g_alpha;
        }
        else
        {
            const double g_alpha = composited_color(3) - 1.0;
            constraints(3) = g_alpha;
        }
        
        // Gray-scale constraints
        for (int i = 0; i < num_gray_layers; ++ i)
        {
            const int& gray_layer      = gray_layers[i];
            const Vec3 color           = colors.segment<3>(gray_layer * 3);
            const Vec3 gray_constraint = std::sqrt(3) * color - color.norm() * Vec3::Ones();
            
            constraints.segment(3 + num_alpha_constraints + 3 * i, 3) = gray_constraint;
        }
        
        return constraints;
    }
    
    VecX calculate_derivative_of_unmixing_energy(const VecX&                  alphas,
                                                 const VecX&                  colors,
                                                 const vector<ColorModelPtr>& models,
                                                 const double                 sigma,
                                                 const bool                   use_sparcity,
                                                 const bool                   use_minimum_alpha)
    {
        const int num_layers = static_cast<int>(alphas.rows());
        VecX grad = VecX(num_layers * 4);
        
        // Main term
        for (int index = 0; index < num_layers; ++ index)
        {
            const auto  m = models[index];
            const Vec3& u = colors.segment<3>(index * 3);
            
            grad(index)                             = m->calculate_distance(u);
            grad.segment<3>(num_layers + index * 3) = alphas(index) * m->calculate_distance_gradient(u);
        }
        
        // Sparcity term
        if (use_sparcity)
        {
            double alpha_sum         = alphas.sum();
            double alpha_squared_sum = alphas.squaredNorm();
            for (int index = 0; index < num_layers; ++ index)
            {
                grad(index) += sigma * (alpha_squared_sum - 2.0 * alphas(index) * alpha_sum) / (alpha_squared_sum * alpha_squared_sum);
            }
        }
        
        // Minimum alpha term
        if (use_minimum_alpha)
        {
            constexpr double epsilon = 0.01;
            grad.segment(0, num_layers) = epsilon * VecX::Ones(num_layers);
        }
        
        return grad;
    }
    
    inline double calculate_derivative_of_composite_alpha_by_source_alpha(double alpha_d,
                                                                          const CompOp& comp_op)
    {
        return comp_op.X * alpha_d + comp_op.Y * (1.0 - alpha_d) - comp_op.Z * alpha_d;
    }
    
    inline double calculate_derivative_of_composite_alpha_by_destination_alpha(double alpha_s,
                                                                               const CompOp& comp_op)
    {
        return comp_op.X * alpha_s - comp_op.Y * alpha_s + comp_op.Z * (1.0 - alpha_s);
    }
    
    // In general cases, the return value should be a dense 3-by-3 matrix; however, the use of separable blend functions allows it to be a diagonal matrix.
    inline Vec3 calculate_derivative_of_blend_function_by_source(const Vec3& c_s, const Vec3& c_d, const BlendMode mode)
    {
        return Vec3(blend_grad_s(c_s(0), c_d(0), mode), blend_grad_s(c_s(1), c_d(1), mode), blend_grad_s(c_s(2), c_d(2), mode));
    }
    
    // In general cases, the return value should be a dense 3-by-3 matrix; however, the use of separable blend functions allows it to be a diagonal matrix.
    inline Vec3 calculate_derivative_of_blend_function_by_destination(const Vec3& c_s, const Vec3& c_d, const BlendMode mode)
    {
        return Vec3(blend_grad_d(c_s(0), c_d(0), mode), blend_grad_d(c_s(1), c_d(1), mode), blend_grad_d(c_s(2), c_d(2), mode));
    }
    
    Mat4 calculate_derivative_of_composite_two_layers_by_source(const Vec4&     x_s,
                                                                const Vec4&     x_d,
                                                                const CompOp&   comp_op,
                                                                const BlendMode mode)
    {
        const Vec4   x_m = composite_two_layers(x_s, x_d, comp_op, mode);
        const double A   = x_m(3);
        const Vec3   B   = x_m.segment<3>(0);
        const Vec3   D   = blend(x_s.segment<3>(0), x_d.segment<3>(0), mode);
        
        const double partial_A_per_partial_a_s = calculate_derivative_of_composite_alpha_by_source_alpha(x_d(3), comp_op);
        
        // Diagonal matrices (In general cases, these should be dense 3-by-3 matrices; however, the use of separable blend functions allows them to be diagonal matrices.)
        const Vec3 partial_D_per_partial_c_s = calculate_derivative_of_blend_function_by_source(x_s.segment<3>(0), x_d.segment<3>(0), mode);
        const Vec3 partial_C_per_partial_c_s = x_s(3) * x_d(3) * partial_D_per_partial_c_s + Vec3::Constant(comp_op.Y * (1.0 - x_d(3)) * x_s(3));
        const Vec3 partial_B_per_partial_c_s = partial_C_per_partial_c_s / A;
        const Vec3 partial_C_per_partial_a_s = D * x_d(3) + comp_op.Y * (1.0 - x_d(3)) * x_s.segment<3>(0) - comp_op.Z * x_d(3) * x_d.segment<3>(0);
        
        // Row vector
        RowVec3 partial_B_per_partial_a_s = (partial_C_per_partial_a_s - B * partial_A_per_partial_a_s) / A;
        
        Mat4 derivative = Mat4::Zero();
        derivative(0, 0)             = partial_B_per_partial_c_s(0);
        derivative(1, 1)             = partial_B_per_partial_c_s(1);
        derivative(2, 2)             = partial_B_per_partial_c_s(2);
        derivative(3, 3)             = partial_A_per_partial_a_s;
        derivative.block<1, 3>(3, 0) = partial_B_per_partial_a_s;
        
        return derivative;
    }
    
    Mat4 calculate_derivative_of_composite_two_layers_by_destination(const Vec4&     x_s,
                                                                     const Vec4&     x_d,
                                                                     const CompOp&   comp_op,
                                                                     const BlendMode mode)
    {
        const Vec4   x_m = composite_two_layers(x_s, x_d, comp_op, mode);
        const double A   = x_m(3);
        const Vec3   B   = x_m.segment<3>(0);
        const Vec3   D   = blend(x_s.segment<3>(0), x_d.segment<3>(0), mode);
        
        const double partial_A_per_partial_a_d = calculate_derivative_of_composite_alpha_by_destination_alpha(x_s(3), comp_op);
        
        // Diagonal matrices (In general cases, these should be dense 3-by-3 matrices; however, the use of separable blend functions allows them to be diagonal matrices.)
        const Vec3 partial_D_per_partial_c_d = calculate_derivative_of_blend_function_by_destination(x_s.segment<3>(0), x_d.segment<3>(0), mode);
        const Vec3 partial_C_per_partial_c_d = x_s(3) * x_d(3) * partial_D_per_partial_c_d + Vec3::Constant(comp_op.Z * (1.0 - x_s(3)) * x_d(3));
        const Vec3 partial_B_per_partial_c_d = partial_C_per_partial_c_d / A;
        const Vec3 partial_C_per_partial_a_d = D * x_s(3) - comp_op.Y * x_s(3) * x_s.segment<3>(0) + comp_op.Z * (1.0 - x_s(3)) * x_d.segment<3>(0);
        
        // Row vector
        RowVec3 partial_B_per_partial_a_d = (partial_C_per_partial_a_d - B * partial_A_per_partial_a_d) / A;
        
        Mat4 derivative = Mat4::Zero();
        derivative(0, 0)             = partial_B_per_partial_c_d(0);
        derivative(1, 1)             = partial_B_per_partial_c_d(1);
        derivative(2, 2)             = partial_B_per_partial_c_d(2);
        derivative(3, 3)             = partial_A_per_partial_a_d;
        derivative.block<1, 3>(3, 0) = partial_B_per_partial_a_d;
        
        return derivative;
    }
    
    Mat4 calculate_derivative_of_k_th_composited_rgba_by_i_th_layer_rgba(const VecX&                   alphas,
                                                                         const VecX&                   colors,
                                                                         const Vec3&                   target_color,
                                                                         const std::vector<CompOp>&    comp_ops,
                                                                         const std::vector<BlendMode>& modes,
                                                                         const int                     i,
                                                                         const int                     k)
    {
        if (i == 0 && k == 0)
        {
            return Mat4::Identity();
        }
        
        auto comp_ops_k_minus_1 = comp_ops; comp_ops_k_minus_1.resize(k);
        auto modes_k_minus_1    = modes;    modes_k_minus_1.resize(k);
        
        const Vec4 x_k             = Vec4(colors(k * 3 + 0), colors(k * 3 + 1), colors(k * 3 + 2), alphas(k));
        const Vec4 x_hat_k_minus_1 = composite_layers(alphas.segment(0, k),
                                                      colors.segment(0, 3 * k),
                                                      comp_ops_k_minus_1,
                                                      modes_k_minus_1);
        
        if (i == k)
        {
            return calculate_derivative_of_composite_two_layers_by_source(x_k,
                                                                          x_hat_k_minus_1,
                                                                          comp_ops[k],
                                                                          modes[k]);
        }
        else
        {
            const Mat4 derivative_1 = calculate_derivative_of_k_th_composited_rgba_by_i_th_layer_rgba(alphas,
                                                                                                      colors,
                                                                                                      target_color,
                                                                                                      comp_ops,
                                                                                                      modes,
                                                                                                      i,
                                                                                                      k - 1);
            const Mat4 derivative_2 = calculate_derivative_of_composite_two_layers_by_destination(x_k,
                                                                                                  x_hat_k_minus_1,
                                                                                                  comp_ops[k],
                                                                                                  modes[k]);
            
            return derivative_1 * derivative_2;
        }
    }
    
    MatX calculate_derivative_of_constraint_vector(const VecX&              alphas,
                                                   const VecX&              colors,
                                                   const Vec3&              target_color,
                                                   const vector<CompOp>&    comp_ops,
                                                   const vector<BlendMode>& modes,
                                                   const bool               use_target_alphas,
                                                   const VecX&              target_alphas,
                                                   const vector<int>&       gray_layers)
    {
        const int num_gray_layers       = static_cast<int>(gray_layers.size());
        const int num_layers            = static_cast<int>(alphas.rows());
        const int num_alpha_constraints = use_target_alphas ? num_layers : 1;
        const int num_constraints       = 3 + num_alpha_constraints + 3 * num_gray_layers;
        
        MatX derivative = MatX::Zero(4 * num_layers, num_constraints);
        
        for (int i = 0; i < num_layers; ++ i)
        {
#ifdef AKSOY_PERFORMANCE_TEST_OPTION
            Mat4 i_th_derivative = Mat4::Zero();
            i_th_derivative(0, 0) = alphas[i];
            i_th_derivative(1, 1) = alphas[i];
            i_th_derivative(2, 2) = alphas[i];
            i_th_derivative(3, 0) = colors(3 * i + 0);
            i_th_derivative(3, 1) = colors(3 * i + 1);
            i_th_derivative(3, 2) = colors(3 * i + 2);
            i_th_derivative(3, 3) = 1.0;
#else
            const Mat4 i_th_derivative = calculate_derivative_of_k_th_composited_rgba_by_i_th_layer_rgba(alphas,
                                                                                                         colors,
                                                                                                         target_color,
                                                                                                         comp_ops,
                                                                                                         modes,
                                                                                                         i,
                                                                                                         num_layers - 1);
#endif
            
            if (use_target_alphas)
            {
                derivative.block<1, 3>(i, 0)                  = i_th_derivative.block<1, 3>(3, 0);
                derivative.block<3, 3>(num_layers + i * 3, 0) = i_th_derivative.block<3, 3>(0, 0);
                derivative(i, 3 + i)                          = 1.0;
            }
            else
            {
                derivative.block<1, 4>(i, 0)                  = i_th_derivative.block<1, 4>(3, 0);
                derivative.block<3, 4>(num_layers + i * 3, 0) = i_th_derivative.block<3, 4>(0, 0);
            }
        }
        
        // Constraints for gray-scale layers
        for (int i = 0; i < num_gray_layers; ++ i)
        {
            const int& gray_layer = gray_layers[i];
            const Vec3 color = colors.segment<3>(gray_layer * 3);
            const Mat3 ccc   = (Mat3() << color, color, color).finished();
            
            // Note: When color.norm() is sufficiently small, the derivative becomes nearly zeros
            constexpr double epsilon = 1e-03;
            if (color.norm() > epsilon)
            {
                derivative.block<3, 3>(num_layers + gray_layer * 3, 3 + num_alpha_constraints + i * 3) = std::sqrt(3) * Mat3::Identity() - (1.0 / color.norm()) * ccc;
            }
        }
        
        return derivative;
    }
    
    //////////////////////////////////////////////////////////////////////////////////
    // Wrapper functions
    //////////////////////////////////////////////////////////////////////////////////
    
    Vec4 composite_two_layers(const Vec4&     x_s,
                              const Vec4&     x_d,
                              const CompOp&   comp_op,
                              const BlendMode mode,
                              const bool      crop)
    {
        return composite_two_layers(x_s.segment<3>(0),
                                    x_d.segment<3>(0),
                                    x_s(3),
                                    x_d(3),
                                    comp_op,
                                    mode,
                                    crop);
    }
    
    double calculate_unmixing_energy_term(const VecX&                  x,
                                          const vector<ColorModelPtr>& models,
                                          const double                 sigma,
                                          const bool                   use_sparcity,
                                          const bool                   use_minimum_alpha)
    {
        const int  num_layers = static_cast<int>(x.size() / 4);
        const VecX alphas     = x.segment(0, num_layers);
        const VecX colors     = x.segment(num_layers, num_layers * 3);
        return calculate_unmixing_energy_term(alphas, colors, models, sigma, use_sparcity, use_minimum_alpha);
    }
    
    VecX calculate_constraint_vector(const VecX&              x,
                                     const Vec3&              target_color,
                                     const vector<CompOp>&    comp_ops,
                                     const vector<BlendMode>& modes,
                                     const bool               use_target_alphas,
                                     const VecX&              target_alphas,
                                     const vector<int>&       gray_layers)
    {
        const int  num_layers = static_cast<int>(x.size() / 4);
        const VecX alphas     = x.segment(0, num_layers);
        const VecX colors     = x.segment(num_layers, num_layers * 3);
        return calculate_constraint_vector(alphas,
                                           colors,
                                           target_color,
                                           comp_ops,
                                           modes,
                                           use_target_alphas,
                                           target_alphas,
                                           gray_layers);
    }
}
