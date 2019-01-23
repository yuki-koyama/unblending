#ifndef IMAGE_PROCESSING_HPP
#define IMAGE_PROCESSING_HPP

#include <vector>
#include <string>
#include <Eigen/Core>

namespace unblending
{
    class AbstractImage
    {
    public:
        AbstractImage(int width = 0, int height = 0) : width_(width), height_(height) {}
        
        using IntColor = Eigen::Vector4i;
        
        void save(const std::string& file_path) const;
        
        int width()  const { return width_;  }
        int height() const { return height_; }
        
    protected:
        int width_;
        int height_;
        
        virtual IntColor get_color(int x, int y) const = 0;
    };
    
    /// \brief Image class for handling a single-channel image.
    class Image final : public AbstractImage
    {
    public:
        Image(int width, int height, double value = 0.0) : AbstractImage(width, height)
        {
            pixels_ = std::vector<double>(width_ * height_, value);
        }
        
        void set_pixel(int x, int y, double value)
        {
            assert(x < width() && y < height());
            pixels_[y * width() + x] = value;
        }
        
        double get_pixel(int x, int y) const
        {
            assert(x < width() && y < height());
            return pixels_[y * width() + x];
        }
        
        void force_unity();
        void scale_to_unit();
        void fill(const double value);
        
        Image operator+(const Image& image) const
        {
            assert(width() == image.width());
            assert(height() == image.height());
            Image new_image(width(), height());
            for (int x = 0; x < width(); ++ x) for (int y = 0; y < height(); ++ y)
            {
                new_image.set_pixel(x, y, get_pixel(x, y) + image.get_pixel(x, y));
            }
            return new_image;
        }
        
        Image operator-(const Image& image) const
        {
            assert(width() == image.width());
            assert(height() == image.height());
            Image new_image(width(), height());
            for (int x = 0; x < width(); ++ x) for (int y = 0; y < height(); ++ y)
            {
                new_image.set_pixel(x, y, get_pixel(x, y) - image.get_pixel(x, y));
            }
            return new_image;
        }

        Image operator*(const Image& image) const
        {
            assert(width() == image.width());
            assert(height() == image.height());
            Image new_image(width(), height());
            for (int x = 0; x < width(); ++ x) for (int y = 0; y < height(); ++ y)
            {
                new_image.set_pixel(x, y, get_pixel(x, y) * image.get_pixel(x, y));
            }
            return new_image;
        }
        
    private:
        IntColor get_color(int x, int y) const override;
        
        std::vector<double> pixels_;
    };
    
    /// \brief Image class for handling a 4-channel (RGBA) image.
    class ColorImage final : public AbstractImage
    {
    public:
        ColorImage(int width, int height) : AbstractImage(width, height)
        {
            rgba_ = std::vector<Image>(4, Image(width, height, 1.0));
        }
        
        ColorImage(const std::string& file_path);
        
        void set_rgb(int x, int y, const Eigen::Vector3d& rgb)
        {
            assert(x < width() && y < height());
            for (int i : { 0, 1, 2 }) rgba_[i].set_pixel(x, y, rgb(i));
        }
        
        void set_rgba(int x, int y, const Eigen::Vector4d& rgba)
        {
            assert(x < width() && y < height());
            for (int i : { 0, 1, 2, 3 }) rgba_[i].set_pixel(x, y, rgba(i));
        }
        
        void set_rgba(int x, int y, const Eigen::Vector3d& rgb, double a)
        {
            assert(x < width() && y < height());
            for (int i : { 0, 1, 2 }) rgba_[i].set_pixel(x, y, rgb(i));
            rgba_[3].set_pixel(x, y, a);
        }
        
        void fill(const Eigen::Vector3d& rgb);
        void fill(const Eigen::Vector4d& rgba);
        
        Eigen::Vector3d get_rgb(int x, int y) const
        {
            assert(x < width() && y < height());
            Eigen::Vector3d rgb;
            for (int i : { 0, 1, 2 }) rgb(i) = rgba_[i].get_pixel(x, y);
            return rgb;
        }
        
        Eigen::Vector4d get_rgba(int x, int y) const
        {
            assert(x < width() && y < height());
            Eigen::Vector4d rgba;
            for (int i : { 0, 1, 2, 3 }) rgba(i) = rgba_[i].get_pixel(x, y);
            return rgba;
        }
        
        /// \brief Get the image data as a byte array.
        /// \details Each channel in a pixel is repsented using 8-bit unsigned integer (0 to 255).
        /// \return A vector array whose size is width * height * 4.
        std::vector<uint8_t> get_rgba_bits() const;
        
        void make_fully_opaque();
        
        Image get_luminance() const;
        
        Image& get_r() { return rgba_[0]; }
        Image& get_g() { return rgba_[1]; }
        Image& get_b() { return rgba_[2]; }
        Image& get_a() { return rgba_[3]; }
        const Image& get_r() const { return rgba_[0]; }
        const Image& get_g() const { return rgba_[1]; }
        const Image& get_b() const { return rgba_[2]; }
        const Image& get_a() const { return rgba_[3]; }
        void set_r(const Image& r) { rgba_[0] = r; }
        void set_g(const Image& g) { rgba_[1] = g; }
        void set_b(const Image& b) { rgba_[2] = b; }
        void set_a(const Image& a) { rgba_[3] = a; }
        
        ColorImage get_scaled_image(int target_width) const;
        
    private:
        IntColor get_color(int x, int y) const override;
        
        std::vector<Image> rgba_;
    };
    
    class ImageProcessingConcurrencyManager
    {
    public:
        static ImageProcessingConcurrencyManager& get_singleton()
        {
            static ImageProcessingConcurrencyManager singleton;
            return singleton;
        }
        
        int  get_target_concurrency() const { return target_concurrency_; }
        void set_target_concurrency(int target_concurrency) { target_concurrency_ = target_concurrency; }
        
    private:
        int target_concurrency_ = 0;
    };
    
    Image apply_convolution(const Image& image, const Eigen::MatrixXd& kernel);
    
    /// \brief Calculate a kernel of the guided image filter.
    /// \details This function is used inside apply_guided_filter.
    Image calculate_guided_filter_kernel(const Image& image,
                                         int center_x,
                                         int center_y,
                                         int radius,
                                         double epsilon = 0.01,
                                         bool force_positive = true);
    
    Image calculate_gradient_magnitude(const Image& image);
    
    /// \brief Calculate the result of applying the guided image filter to an image.
    Image apply_guided_filter(const Image& input_image,
                              const ColorImage& guidance_image,
                              int radius,
                              double epsilon);
    
    inline Image apply_sobel_filter_x(const Image& image)
    {
        Eigen::Matrix3d kernel;
        kernel << +1.0,  0.0, -1.0, +2.0,  0.0, -2.0, +1.0,  0.0, -1.0;
        return apply_convolution(image, kernel);
    }
    
    inline Image apply_sobel_filter_y(const Image& image)
    {
        Eigen::Matrix3d kernel;
        kernel << +1.0, +2.0, +1.0, 0.0,  0.0,  0.0, -1.0, -2.0, -1.0;
        return apply_convolution(image, kernel);
    }
    
    inline Image apply_box_filter(const Image& image, int radius)
    {
        assert(radius >= 0);
        if (radius == 0) return image;
        const int size = 2 * radius + 1;
        const Eigen::MatrixXd kernel = Eigen::MatrixXd::Constant(size, size, 1.0 / static_cast<double>(size * size));
        return apply_convolution(image, kernel);
    }
    
    inline Image calculate_difference(const ColorImage& left_image, const ColorImage& right_image)
    {
        assert(left_image.width() == right_image.width());
        assert(left_image.height() == right_image.height());
        
        const Image diff_r = left_image.get_r() - right_image.get_r();
        const Image diff_g = left_image.get_g() - right_image.get_g();
        const Image diff_b = left_image.get_b() - right_image.get_b();
        const Image diff_a = left_image.get_a() - right_image.get_a();
        
        Image new_image(left_image.width(), left_image.height());
        for (int x = 0; x < left_image.width(); ++ x) for (int y = 0; y < left_image.height(); ++ y)
        {
            const double diff_r_squared = diff_r.get_pixel(x, y) * diff_r.get_pixel(x, y);
            const double diff_g_squared = diff_g.get_pixel(x, y) * diff_g.get_pixel(x, y);
            const double diff_b_squared = diff_b.get_pixel(x, y) * diff_b.get_pixel(x, y);
            const double diff_a_squared = diff_a.get_pixel(x, y) * diff_a.get_pixel(x, y);
            
            new_image.set_pixel(x, y, std::sqrt(diff_r_squared + diff_g_squared + diff_b_squared + diff_a_squared));
        }
        
        return new_image;
    }
}

#endif // IMAGE_PROCESSING_HPP
