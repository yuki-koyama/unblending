#ifndef EQUATIONS_HPP
#define EQUATIONS_HPP

#include <unblending/common.hpp>
#include <unblending/blend_mode.hpp>
#include <unblending/comp_op.hpp>

namespace unblending
{
    Vec4 composite_two_layers(const Vec3&     c_s,
                              const Vec3&     c_d,
                              const double    a_s,
                              const double    a_d,
                              const CompOp&   comp_op,
                              const BlendMode mode,
                              const bool      crop = false);
    
    Vec4 composite_layers(const VecX&                   alphas,
                          const VecX&                   colors,
                          const std::vector<CompOp>&    comp_ops,
                          const std::vector<BlendMode>& modes,
                          const bool                    crop = false);
    
    double calculate_lagrange_term(const VecX& constraint_vector,
                                   const VecX& lambda);
    
    double calculate_penalty_term(const VecX&  constraint_vector,
                                  const double lo);
    
    double calculate_unmixing_energy_term(const VecX&                       alphas,
                                          const VecX&                       colors,
                                          const std::vector<ColorModelPtr>& models,
                                          const double                      sigma,
                                          const bool                        use_sparcity,
                                          const bool                        use_minimum_alpha);
    
    VecX calculate_constraint_vector(const VecX&                   alphas,
                                     const VecX&                   colors,
                                     const Vec3&                   target_color,
                                     const std::vector<CompOp>&    comp_ops,
                                     const std::vector<BlendMode>& modes,
                                     const bool                    use_target_alphas,
                                     const VecX&                   target_alphas = VecX(),
                                     const std::vector<int>&       gray_layers = std::vector<int>());
    
    VecX calculate_derivative_of_unmixing_energy(const VecX&                       alphas,
                                                 const VecX&                       colors,
                                                 const std::vector<ColorModelPtr>& models,
                                                 const double                      sigma,
                                                 const bool                        use_sparcity,
                                                 const bool                        use_minimum_alpha);
    
    MatX calculate_derivative_of_constraint_vector(const VecX&                   alphas,
                                                   const VecX&                   colors,
                                                   const Vec3&                   target_color,
                                                   const std::vector<CompOp>&    comp_ops,
                                                   const std::vector<BlendMode>& modes,
                                                   const bool                    use_target_alphas,
                                                   const VecX&                   target_alphas = VecX(),
                                                   const std::vector<int>&       gray_layers = std::vector<int>());
    
    //////////////////////////////////////////////////////////////////////////////////
    // Wrapper functions
    //////////////////////////////////////////////////////////////////////////////////
    
    Vec4 composite_two_layers(const Vec4&     x_s,
                              const Vec4&     x_d,
                              const CompOp&   comp_op,
                              const BlendMode mode,
                              const bool      crop = false);
    
    double calculate_unmixing_energy_term(const VecX&                       x,
                                          const std::vector<ColorModelPtr>& models,
                                          const double                      sigma,
                                          const bool                        use_sparcity,
                                          const bool                        use_minimum_alpha);
    
    VecX calculate_constraint_vector(const VecX&                   x,
                                     const Vec3&                   target_color,
                                     const std::vector<CompOp>&    comp_ops,
                                     const std::vector<BlendMode>& modes,
                                     const bool                    use_target_alphas,
                                     const VecX&                   target_alphas = VecX(),
                                     const std::vector<int>&       gray_layers = std::vector<int>());
}

#endif // EQUATIONS_HPP
