#pragma once

#include <cstdint>
#include <random>

#include "common/ray.hpp"
#include "common/vector.hpp"

namespace common {

  class Camera {
  public:
    Camera(std::uint32_t image_width, std::uint32_t image_height, double vertical_fov_deg,
           Vec3 const & lookfrom, Vec3 const & lookat, Vec3 const & vup,
           std::uint32_t samples_per_pixel, std::uint64_t seed);

    [[nodiscard]] Ray get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t sample_id);

  private:
    // resolución destino
    std::uint32_t m_image_width;
    std::uint32_t m_image_height;

    // muestreo
    std::uint32_t m_samples_per_pixel;

    // geometría cámara
    Vec3 m_origin;
    Vec3 m_horizontal;
    Vec3 m_vertical;
    Vec3 m_lower_left_corner;

    // delta por píxel en mundo
    Vec3 m_pixel_delta_u;
    Vec3 m_pixel_delta_v;

    // RNG para jitter subpixel
    std::mt19937_64 m_rng;
    std::uniform_real_distribution<double> m_dist;
  };

}  // namespace common
