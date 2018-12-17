#ifndef COMMON_HPP
#define COMMON_HPP

#include <Eigen/Core>
#include <thread>
#include <vector>
#include <iomanip>
#include <memory>

namespace unblending
{
    using Vector3i = Eigen::Vector3i;
    using Vec2     = Eigen::Vector2d;
    using Vec3     = Eigen::Vector3d;
    using Vec4     = Eigen::Vector4d;
    using VecX     = Eigen::VectorXd;
    using Mat2     = Eigen::Matrix2d;
    using Mat3     = Eigen::Matrix3d;
    using Mat4     = Eigen::Matrix4d;
    using MatX     = Eigen::MatrixXd;
    using RowVec3  = Eigen::Matrix<double, 1, 3>;
    
    class ColorModel;
    using ColorModelPtr = std::shared_ptr<ColorModel>;
    
    inline Eigen::IOFormat get_format() { return Eigen::IOFormat(4, 0, ", ", "\n", "[", "]"); }
    
    // Crop a scalar value into [0, 1]
    template<typename Scalar>
    Scalar crop_value(Scalar value)
    {
        return std::min(std::max(value, 0.0), 1.0);
    }
    
    inline Vec3 crop_vec3(const Vec3& vector)
    {
        return vector.cwiseMax(Vec3::Zero()).cwiseMin(Vec3::Ones());
    }
    
    inline Vec4 crop_vec4(const Vec4& vector)
    {
        return vector.cwiseMax(Vec4::Zero()).cwiseMin(Vec4::Ones());
    }
    
    inline std::string get_current_time_in_string()
    {
        const std::time_t t = std::time(nullptr);
        std::stringstream s; s << std::put_time(std::localtime(&t), "%Y%m%d%H%M%S");
        return s.str();
    }
}

#endif // COMMON_HPP
