#include "render/camera.hpp"

#include <cmath>

namespace render {

  camera::camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
                 vector const & lookfrom, vector const & lookat, vector const & vup,
                 std::uint32_t samples_per_pixel, std::uint64_t seed)
      : m_image_width(image_width), m_image_height(image_height),
        m_samples_per_pixel(samples_per_pixel), m_origin(lookfrom), m_rng(seed), m_dist(0.0, 1.0) {
    // relación de aspecto
    double const aspect_ratio =
        static_cast<double>(image_width) / static_cast<double>(image_height);

    // alto del viewport a partir del FOV vertical
    double const theta           = vfov_deg * M_PI / 180.0;
    double const h               = std::tan(theta / 2.0);
    double const viewport_height = 2.0 * h;
    double const viewport_width  = viewport_height * aspect_ratio;

    // sistema de referencia de cámara
    vector const w = (lookfrom - lookat).normalized();  // mira hacia atrás
    vector const u = (vup.cross(w)).normalized();       // derecha
    vector const v = w.cross(u);                        // arriba "real" ortonormal

    // dirección hacia delante (desde la cámara hacia la escena)
    vector const forward = (-1.0) * w;

    // tamaño del viewport en el espacio mundial
    vector const viewport_u = viewport_width * u;
    vector const viewport_v = viewport_height * v;

    m_horizontal        = viewport_u;
    m_vertical          = viewport_v;
    m_lower_left_corner = m_origin + forward - 0.5 * m_horizontal - 0.5 * m_vertical;

    // cuánto avanza un píxel en u y v
    m_pixel_delta_u = m_horizontal / static_cast<double>(m_image_width);
    m_pixel_delta_v = m_vertical / static_cast<double>(m_image_height);
  }

  ray camera::get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t /*sample_id*/) {
    // centro del píxel (px,py) en coordenadas de mundo
    vector pixel_center = m_lower_left_corner +
                          (static_cast<double>(px) + 0.5) * m_pixel_delta_u +
                          (static_cast<double>(py) + 0.5) * m_pixel_delta_v;

    vector dir = pixel_center - m_origin;
    return ray(m_origin, dir.normalized());
  }

}  // namespace render
