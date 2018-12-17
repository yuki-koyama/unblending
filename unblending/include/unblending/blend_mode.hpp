#ifndef BLEND_MODE_HPP
#define BLEND_MODE_HPP

#include <unblending/common.hpp>
#include <string>

namespace unblending
{
    enum class BlendMode
    {
        Normal,
        Multiply,
        Screen,
        Overlay,
        Darken,
        Lighten,
        ColorDodge,
        ColorBurn,
        HardLight,
        SoftLight,
        Difference,
        Exclusion,
        LinearDodge,
    };
    
    inline std::string retrieve_name(BlendMode mode)
    {
        switch (mode)
        {
            case BlendMode::Normal:      return "Normal";
            case BlendMode::Multiply:    return "Multiply";
            case BlendMode::Screen:      return "Screen";
            case BlendMode::Overlay:     return "Overlay";
            case BlendMode::Darken:      return "Darken";
            case BlendMode::Lighten:     return "Lighten";
            case BlendMode::ColorDodge:  return "ColorDodge";
            case BlendMode::ColorBurn:   return "ColorBurn";
            case BlendMode::HardLight:   return "HardLight";
            case BlendMode::SoftLight:   return "SoftLight";
            case BlendMode::Difference:  return "Difference";
            case BlendMode::Exclusion:   return "Exclusion";
            case BlendMode::LinearDodge: return "LinearDodge";
            default:
                assert(false);
                return "";
        }
    }
    
    inline BlendMode retrieve_by_name(const std::string& name)
    {
        if (name == "Normal"     ) { return BlendMode::Normal; }
        if (name == "Multiply"   ) { return BlendMode::Multiply; }
        if (name == "Screen"     ) { return BlendMode::Screen; }
        if (name == "Overlay"    ) { return BlendMode::Overlay; }
        if (name == "Darken"     ) { return BlendMode::Darken; }
        if (name == "Lighten"    ) { return BlendMode::Lighten; }
        if (name == "ColorDodge" ) { return BlendMode::ColorDodge; }
        if (name == "ColorBurn"  ) { return BlendMode::ColorBurn; }
        if (name == "HardLight"  ) { return BlendMode::HardLight; }
        if (name == "SoftLight"  ) { return BlendMode::SoftLight; }
        if (name == "Difference" ) { return BlendMode::Difference; }
        if (name == "Exclusion"  ) { return BlendMode::Exclusion; }
        if (name == "LinearDodge") { return BlendMode::LinearDodge; }
        
        assert(false);
        return BlendMode::Normal;
    }
    
    inline std::vector<BlendMode> get_blend_mode_list()
    {
        return {
            BlendMode::Normal,
            BlendMode::Multiply,
            BlendMode::Screen,
            BlendMode::Overlay,
            BlendMode::Darken,
            BlendMode::Lighten,
            BlendMode::ColorDodge,
            BlendMode::ColorBurn,
            BlendMode::HardLight,
            BlendMode::SoftLight,
            BlendMode::Difference,
            BlendMode::Exclusion,
            BlendMode::LinearDodge
        };
    }
    
    constexpr double blend_function_internal_epsilon = 1e-05;
    
    inline double blend_grad_s(double s, double d, BlendMode mode, bool crop = false)
    {
        switch (mode)
        {
            case BlendMode::Normal:
                return 1.0;
            case BlendMode::Multiply:
                return d;
            case BlendMode::Screen:
                return 1.0 - d;
            case BlendMode::Overlay:
                return (d <= 0.5) ? 2.0 * d : 2.0 * (1.0 - d);
            case BlendMode::Darken:
                return (s < d) ? 1.0 : 0.0;
            case BlendMode::Lighten:
                return (s < d) ? 0.0 : 1.0;
            case BlendMode::ColorDodge:
                return (d < blend_function_internal_epsilon) ? 0.0 : ((1.0 - s < blend_function_internal_epsilon) ? 0.0 : ((1.0 < d / (1.0 - s)) ? 0.0 : d / ((1.0 - s) * (1.0 - s))));
            case BlendMode::ColorBurn:
                return (1.0 - d < blend_function_internal_epsilon) ? 0.0 : ((s < blend_function_internal_epsilon) ? 0.0 : - (1.0 < (1.0 - d) / s ? 0.0 : - (1.0 - d) / (s * s)));
            case BlendMode::HardLight:
                return (s <= 0.5) ? 2.0 * d : 2.0 * (1.0 - d);
            case BlendMode::SoftLight:
                return (s <= 0.5) ? 2.0 * d * (1.0 - d) : 2.0 * (((d <= 0.25) ? ((16.0 * d - 12.0) * d + 4.0) * d : std::sqrt(d)) - d);
            case BlendMode::Difference:
                return (s < d) ? - 1.0 : 1.0;
            case BlendMode::Exclusion:
                return 1.0 - 2.0 * d;
            case BlendMode::LinearDodge:
                return (crop && s + d > 1.0) ? 0.0 : 1.0;
            default:
                assert(false);
                return 0.0;
        }
    }
    
    inline double blend_grad_d(double s, double d, BlendMode mode, bool crop = false)
    {
        switch (mode)
        {
            case BlendMode::Normal:
                return 0.0;
            case BlendMode::Multiply:
                return s;
            case BlendMode::Screen:
                return 1.0 - s;
            case BlendMode::Overlay:
                return (d <= 0.5) ? 2.0 * s : 2.0 * (1.0 - s);
            case BlendMode::Darken:
                return (s < d) ? 0.0 : 1.0;
            case BlendMode::Lighten:
                return (s < d) ? 1.0 : 0.0;
            case BlendMode::ColorDodge:
                return (d < blend_function_internal_epsilon) ? 0.0 : ((1.0 - s < blend_function_internal_epsilon) ? 0.0 : ((1.0 < d / (1.0 - s)) ? 0.0 : 1.0 / (1.0 - s)));
            case BlendMode::ColorBurn:
                return (1.0 - d < blend_function_internal_epsilon) ? 0.0 : ((s < blend_function_internal_epsilon) ? 0.0 : - (1.0 < (1.0 - d) / s ? 0.0 : - 1.0 / s));
            case BlendMode::HardLight:
                return (s <= 0.5) ? 2.0 * s : 2.0 * (1.0 - s);
            case BlendMode::SoftLight:
                return (s <= 0.5) ? 2.0 * s + 2.0 * d - 4.0 * s * d : (1.0 + (2.0 * s - 1.0) * ((d <= 0.25) ? (48.0 * d * d - 24.0 * d + 4.0) : (1.0 / (2.0 * std::sqrt(d)))) - (2.0 * s - 1.0));
            case BlendMode::Difference:
                return (s < d) ? 1.0 : - 1.0;
            case BlendMode::Exclusion:
                return 1.0 - 2.0 * s;
            case BlendMode::LinearDodge:
                return (crop && s + d > 1.0) ? 0.0 : 1.0;
            default:
                assert(false);
                return 0.0;
        }
    }
    
    inline double blend(double s, double d, BlendMode mode, bool crop = false)
    {
        switch (mode)
        {
            case BlendMode::Normal:
                return s;
            case BlendMode::Multiply:
                return s * d;
            case BlendMode::Screen:
                return 1.0 - (1.0 - s) * (1.0 - d);
            case BlendMode::Overlay:
                return (d <= 0.5) ? 2.0 * s * d : 1.0 - 2.0 * (1.0 - s) * (1.0 - d);
            case BlendMode::Darken:
                return (s < d) ? s : d;
            case BlendMode::Lighten:
                return (s < d) ? d : s;
            case BlendMode::ColorDodge:
                return (d < blend_function_internal_epsilon) ? 0.0 : ((1.0 - s < blend_function_internal_epsilon) ? 1.0 : std::min(1.0, d / (1.0 - s)));
            case BlendMode::ColorBurn:
                return (1.0 - d < blend_function_internal_epsilon) ? 1.0 : ((s < blend_function_internal_epsilon) ? 0.0 : 1.0 - std::min(1.0, (1.0 - d) / s));
            case BlendMode::HardLight:
                return (s <= 0.5) ? 2.0 * s * d : 1.0 - 2.0 * (1.0 - s) * (1.0 - d);
            case BlendMode::SoftLight:
                return (s <= 0.5) ? d - (1.0 - 2.0 * s) * d * (1.0 - d) : (d + (2.0 * s - 1.0) * (((d <= 0.25) ? ((16.0 * d - 12.0) * d + 4.0) * d : std::sqrt(d)) - d));
            case BlendMode::Difference:
                return (s < d) ? d - s : s - d;
            case BlendMode::Exclusion:
                return s + d - 2.0 * s * d;
            case BlendMode::LinearDodge:
                return (crop && s + d > 1.0) ? 1.0 : s + d;
            default:
                assert(false);
                return 0.0;
        }
    }
    
    inline Vec3 blend(const Vec3& s, const Vec3& d, BlendMode mode, bool crop = false)
    {
        // Assuming separable blend functions
        return Vec3(blend(s(0), d(0), mode, crop), blend(s(1), d(1), mode, crop), blend(s(2), d(2), mode, crop));
    }
}

#endif // BLEND_MODE_HPP
