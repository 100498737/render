#pragma once

#include <cstdint>
#include <random>

#include "render/ray.hpp"
#include "render/vector.hpp"

namespace render {

  class camera {
  public:
    // EXISTENTE: pinhole (compatibilidad).
    camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
           vector const & lookfrom, vector const & lookat, vector const & vup,
           std::uint32_t samples_per_pixel, std::uint64_t seed);

    // NUEVO: DOF (aperture + focus_dist). Mantén la misma firma pero añadiendo estos dos params.
    camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
           vector const & lookfrom, vector const & lookat, vector const & vup,
           std::uint32_t samples_per_pixel, std::uint64_t seed, double aperture,
           double focus_dist);  // <-- NUEVO

    [[nodiscard]] ray get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t sample_id);

    [[nodiscard]] std::uint32_t image_width() const { return m_image_width; }

    [[nodiscard]] std::uint32_t image_height() const { return m_image_height; }

    [[nodiscard]] std::uint32_t spp() const { return m_samples_per_pixel; }

    [[nodiscard]] vector origin() const { return m_origin; }

  private:
    // Resolución
    std::uint32_t m_image_width;
    std::uint32_t m_image_height;

    // Config de muestreo
    std::uint32_t m_samples_per_pixel;

    // Base geométrica de la cámara
    vector m_origin;             // O
    vector m_horizontal;         // ancho del plano de imagen en mundo
    vector m_vertical;           // alto del plano de imagen en mundo
    vector m_lower_left_corner;  // esquina inferior izquierda del plano de imagen

    // Paso por píxel (dx, dy) en mundo
    vector m_pixel_delta_u;
    vector m_pixel_delta_v;

    // RNG por cámara (reproducible)
    std::mt19937_64 m_rng;
    std::uniform_real_distribution<double> m_dist;

    // ===== NUEVO (para DOF) =====
    double m_lens_radius{0.0};  // = aperture/2, 0 => pinhole
    vector m_u, m_v, m_w;       // base de cámara para desplazar el origen en la lente
  };

}  // namespace render
