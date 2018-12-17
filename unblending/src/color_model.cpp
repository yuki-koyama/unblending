#include <unblending/color_model.hpp>
#include <iostream>
#include <cfloat>
#include <Eigen/LU>
#include <Eigen/Eigenvalues>

namespace unblending
{
    namespace
    {
        ColorImage generate_unsorted_visualization(const Mat3& V, const Vec3& A, const Vec3& mu, int size)
        {
            ColorImage image(size, size);
            image.fill(Vec4(0.0, 0.0, 0.0, 0.0));
            
            for (int x_screen = 0; x_screen < size; ++ x_screen)
            {
                for (int y_screen = 0; y_screen < size; ++ y_screen)
                {
                    const double x = (2.0 * x_screen) / (size - 1) - 1.0;
                    const double y = 1.0 - (2.0 * y_screen) / (size - 1);
                    const double t = (std::abs(x) > 1e-05) ? std::atan(y / x) : (y / std::abs(y)) * M_PI_2;
                    
                    Vec3   b_1;
                    Vec3   b_2;
                    double s_1;
                    double s_2;
                    Mat2   r;
                    
                    if (t > M_PI / 6.0)
                    {
                        b_1 = V.col(0);
                        b_2 = V.col(1);
                        s_1 = std::sqrt(std::max(A(0), 0.0));
                        s_2 = std::sqrt(std::max(A(1), 0.0));
                        r.col(0) = Vec2(std::cos(+ M_PI / 2.0), std::sin(+ M_PI / 2.0));
                        r.col(1) = Vec2(std::cos(+ M_PI / 6.0), std::sin(+ M_PI / 6.0));
                    }
                    else if (t > - M_PI / 6.0)
                    {
                        b_1 = V.col(1);
                        b_2 = V.col(2);
                        s_1 = std::sqrt(std::max(A(1), 0.0));
                        s_2 = std::sqrt(std::max(A(2), 0.0));
                        r.col(0) = Vec2(std::cos(+ M_PI / 6.0), std::sin(+ M_PI / 6.0));
                        r.col(1) = Vec2(std::cos(- M_PI / 6.0), std::sin(- M_PI / 6.0));
                    }
                    else
                    {
                        b_1 = V.col(2);
                        b_2 = V.col(0);
                        s_1 = std::sqrt(std::max(A(2), 0.0));
                        s_2 = std::sqrt(std::max(A(0), 0.0));
                        r.col(0) = Vec2(std::cos(- M_PI / 6.0), std::sin(- M_PI / 6.0));
                        r.col(1) = Vec2(std::cos(+ M_PI / 2.0), std::sin(+ M_PI / 2.0));
                    }
                    
                    const Vec2 w = r.inverse() * Vec2(x, y);
                    
                    // If the pixel is outside the hexagon, leave it transparent
                    if (std::abs(w(0)) + std::abs(w(1)) > 1.0) { continue; }
                    
                    const Vec3 color = crop_vec3(mu + w(0) * s_1 * b_1 + w(1) * s_2 * b_2);
                    image.set_rgba(x_screen, y_screen, color, 1.0);
                }
            }
            return image;
        }
        
        double evaluate_visual_clutter(const Mat3& V, const Vec3& A, const Vec3& mu)
        {
            const Vec3 v0 = crop_vec3(  V.col(0) * A(0) + mu);
            const Vec3 v1 = crop_vec3(  V.col(1) * A(1) + mu);
            const Vec3 v2 = crop_vec3(  V.col(2) * A(2) + mu);
            const Vec3 v3 = crop_vec3(- V.col(0) * A(0) + mu);
            const Vec3 v4 = crop_vec3(- V.col(1) * A(1) + mu);
            const Vec3 v5 = crop_vec3(- V.col(2) * A(2) + mu);
            
            return (v0 - v1).norm() + (v1 - v2).norm() + (v2 - v3).norm() + (v3 - v4).norm() + (v4 - v5).norm() + (v5 - v0).norm();
        }
    }
    
    ColorImage GaussianColorModel::generate_visualization() const
    {
        constexpr int size = 480;
        
        const Mat3 sigma = sigma_inv_.inverse();
        const Eigen::EigenSolver<Mat3> eigen_solver(sigma);
        Mat3 V = eigen_solver.eigenvectors().real();
        Vec3 A = eigen_solver.eigenvalues().real();
        
        auto swap = [&](int i, int j)
        {
            const Vec3 v_temp = V.col(i);
            V.col(i) = V.col(j);
            V.col(j) = v_temp;
            const double a_temp = A(i);
            A(i) = A(j);
            A(j) = a_temp;
        };
        
        // Generate many candidates
        std::vector<std::tuple<double, Mat3, Vec3>> candidates;
        candidates.push_back(std::make_tuple(evaluate_visual_clutter(V, A, mu_), V, A));
        for (int i : { 0, 1, 2 })
        {
            V.col(i) *= - 1.0;
            candidates.push_back(std::make_tuple(evaluate_visual_clutter(V, A, mu_), V, A));
            V.col(i) *= - 1.0;
        }
        swap(0, 1);
        candidates.push_back(std::make_tuple(evaluate_visual_clutter(V, A, mu_), V, A));
        for (int i : { 0, 1, 2 })
        {
            V.col(i) *= - 1.0;
            candidates.push_back(std::make_tuple(evaluate_visual_clutter(V, A, mu_), V, A));
            V.col(i) *= - 1.0;
        }
        
        // Pick up the least visually clutter one
        int    min_index = - 1;
        double min_error = DBL_MAX;
        for (int i = 0; i < candidates.size(); ++ i)
        {
            const auto&  candidate = candidates[i];
            const double error     = std::get<0>(candidate);
            
            if (error < min_error)
            {
                min_error = error;
                min_index = i;
            }
        }
        V = std::get<1>(candidates[min_index]);
        A = std::get<2>(candidates[min_index]);
        
        return generate_unsorted_visualization(V, A, mu_, size);
    }
    
    Mat3 GaussianColorModel::get_sigma() const
    {
        return sigma_inv_.inverse();
    }
    
    void GaussianColorModel::set_sigma(const Mat3 &sigma)
    {
        sigma_inv_ = sigma.inverse();
    }
}
