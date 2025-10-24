#include "common/camera.hpp"

#include <cmath>    // std::tan
#include <cstdint>  // std::uint32_t
#include <random>   // std::mt19937_64, std::uniform_real_distribution

namespace common {

  Camera::Camera(std::uint32_t image_width, std::uint32_t image_height, double vertical_fov_deg,
                 Vec3 const & lookfrom, Vec3 const & lookat, Vec3 const & vup,
                 std::uint32_t samples_per_pixel, std::uint64_t seed)
      : m_image_width(image_width), m_image_height(image_height),
        m_samples_per_pixel(samples_per_pixel), m_origin(lookfrom), m_rng(seed), m_dist(0.0, 1.0) {
    // relación de aspecto
    double const aspect_ratio =
        (image_height == 0U) ? 1.0
                             : static_cast<double>(image_width) / static_cast<double>(image_height);

    // fov vertical en radianes
    double const theta = vertical_fov_deg * (M_PI / 180.0);
    double const h     = std::tan(theta / 2.0);

    // alto del viewport en unidades de mundo (cercano a z=-1 mirando -w)
    double const viewport_height = 2.0 * h;
    double const viewport_width  = viewport_height * aspect_ratio;
    (void) viewport_width;  // silenciar warning si no lo usamos explícitamente

    // sistema de ejes de la cámara
    Vec3 const w = (lookfrom - lookat).normalized();
    Vec3 const u = (vup.cross(w)).normalized();
    Vec3 const v = w.cross(u);

    // dirección "frente" de la cámara (hacia -w)
    Vec3 const focal_dir = -w;

    // spans en mundo
    Vec3 const viewport_u = viewport_width * u;
    Vec3 const viewport_v = viewport_height * v;

    m_horizontal        = viewport_u;
    m_vertical          = viewport_v;
    m_lower_left_corner = m_origin + focal_dir - 0.5 * m_horizontal - 0.5 * m_vertical;

    m_pixel_delta_u = m_horizontal / static_cast<double>(m_image_width);
    m_pixel_delta_v = m_vertical / static_cast<double>(m_image_height);
    (void) lookat;
    (void) vup;
  }

  Ray Camera::get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t sample_id) {
    (void) sample_id;  // más adelante usaremos sample_id / jitter para AA

    // jitter subpixel aleatorio en [0,1)
    double const jitter_x = m_dist(m_rng);
    double const jitter_y = m_dist(m_rng);

    // posición dentro del píxel en coordenadas [0,1)
    double const pixel_offset_u = (static_cast<double>(px) + jitter_x);
    double const pixel_offset_v = (static_cast<double>(py) + jitter_y);

    // punto en el plano de imagen en espacio mundo
    Vec3 pixel_world =
        m_lower_left_corner + pixel_offset_u * m_pixel_delta_u + pixel_offset_v * m_pixel_delta_v;

    // dirección del rayo desde el origen de la cámara hasta ese punto
    Vec3 dir = pixel_world - m_origin;

    return Ray(m_origin, dir);
  }

}  // namespace common
