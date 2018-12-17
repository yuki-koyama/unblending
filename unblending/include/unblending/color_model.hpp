#ifndef COLOR_MODEL_HPP
#define COLOR_MODEL_HPP

#include <unblending/common.hpp>
#include <unblending/image_processing.hpp>

namespace unblending
{
    class ColorModel
    {
    public:
        virtual double calculate_distance(const Vec3& color)          const = 0;
        virtual Vec3   calculate_distance_gradient(const Vec3& color) const = 0;
        virtual Vec3   get_representative_color()                     const = 0;
        
        virtual ColorImage generate_visualization() const = 0;
    };
    
    class GaussianColorModel final : public ColorModel
    {
    public:
        GaussianColorModel(const Vec3& mu,
                           const Mat3& sigma_inv) :
        mu_(mu),
        sigma_inv_(sigma_inv)
        {
        }
        
        double calculate_distance(const Vec3& color) const override
        {
            return (color - mu_).transpose() * sigma_inv_ * (color - mu_);
        }
        
        Vec3 calculate_distance_gradient(const Vec3& color) const override
        {
            return 2.0 * sigma_inv_ * (color - mu_);
        }
        
        Vec3 get_representative_color() const override
        {
            return mu_;
        }
        
        ColorImage generate_visualization() const override;
        
        // Getters
        const Vec3& get_mu() const { return mu_; }
        const Mat3& get_sigma_inv() const { return sigma_inv_; }
        Mat3 get_sigma() const;
        
        // Setters
        void set_mu(const Vec3& mu) { mu_ = mu; }
        void set_sigma(const Mat3& sigma);
        void set_sigma_inv(const Mat3& sigma_inv) { sigma_inv_ = sigma_inv; }
        
    private:
        Vec3 mu_;
        Mat3 sigma_inv_;
    };
}

#endif // COLOR_MODEL_HPP
