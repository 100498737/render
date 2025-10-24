#pragma once

#include <cstdint>
#include <random>

#include "render/ray.hpp"
#include "render/vector.hpp"

namespace render {

  class camera {
  public:
    camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
           vector const & lookfrom, vector const & lookat, vector const & vup,
           std::uint32_t samples_per_pixel, std::uint64_t seed);

    // genera el rayo primario para el píxel (px,py) y el sample_id
    [[nodiscard]] ray get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t sample_id);

    [[nodiscard]] std::uint32_t spp() const { return m_samples_per_pixel; }

  private:
    std::uint32_t m_image_width{};
    std::uint32_t m_image_height{};
    std::uint32_t m_samples_per_pixel{};

    vector m_origin;
    vector m_horizontal;
    vector m_vertical;
    vector m_lower_left_corner;

    vector m_pixel_delta_u;
    vector m_pixel_delta_v;

    // RNG para jitter
    std::mt19937_64 m_rng;
    std::uniform_real_distribution<double> m_dist;
  };

}  // namespace render
