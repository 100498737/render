#pragma once

#include <cstdint>
#include <random>

#include "render/ray.hpp"
#include "render/vector.hpp"

namespace render {

  class camera {
  public:
    // Crea una cámara pinhole (sin DOF todavía).
    //
    // image_width, image_height: resolución en píxeles
    // vfov_deg: FOV vertical en grados
    // lookfrom: posición de la cámara en mundo
    // lookat:   punto al que mira la cámara
    // vup:      "up" aproximado del usuario (para construir el sistema u,v,w)
    // samples_per_pixel: número de rayos distintos por píxel
    // seed: semilla para RNG (reproducible)
    camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
           vector const & lookfrom, vector const & lookat, vector const & vup,
           std::uint32_t samples_per_pixel, std::uint64_t seed);

    // Genera el rayo primario (con jitter) para el píxel (px, py) y el índice de
    // muestra "sample_id" en [0, samples_per_pixel).
    //
    // Nota: el origen del rayo siempre es la cámara (pinhole). La dirección
    // apunta hacia el punto subpixel muestreado en el plano de imagen.
    [[nodiscard]] ray get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t sample_id);

    // Getters útiles para tests / depuración
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
  };

}  // namespace render
