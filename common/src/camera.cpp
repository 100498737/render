#include "render/camera.hpp"

#include <cmath>    // tan, numbers::pi
#include <numbers>  // std::numbers::pi

namespace render {

  // muestreo uniforme en disco unidad para la lente
  static inline std::pair<double, double> random_in_unit_disk(
      std::mt19937_64 & rng, std::uniform_real_distribution<double> & dist) {
    for (;;) {
      double x = 2.0 * dist(rng) - 1.0;
      double y = 2.0 * dist(rng) - 1.0;
      if (x * x + y * y < 1.0) {
        return {x, y};
      }
    }
  }

  // Helper to compute camera geometry; extracted from constructor to reduce constructor complexity.
  namespace {

    struct CameraSetup {
      vector horizontal;
      vector vertical;
      vector lower_left_corner;
      vector pixel_delta_u;
      vector pixel_delta_v;
    };

    // Small helper to compute camera basis (keeps make_camera_setup smaller).
    struct CameraBasis {
      vector u;
      vector v;
      vector w;
      vector forward;
    };

    CameraBasis compute_camera_basis(vector const & lookfrom, vector const & lookat,
                                     vector const & vup) {
      CameraBasis b;
      b.w       = (lookfrom - lookat).normalized();  // mira desde lookat hacia lookfrom
      b.u       = (vup.cross(b.w)).normalized();     // derecha ortonormal
      b.v       = b.w.cross(b.u);                    // arriba ortonormal
      b.forward = (-1.0) * b.w;                      // dirección de visión
      return b;
    }

    // Helper: compute viewport width/height from image size and vertical FOV.
    std::pair<double, double> compute_viewport_size(std::uint32_t image_width,
                                                    std::uint32_t image_height, double vfov_deg) {
      double const theta           = vfov_deg * (std::numbers::pi / 180.0);  // rad
      double const half_height     = std::tan(theta * 0.5);                  // tan(vfov/2)
      double const viewport_height = 2.0 * half_height;
      double const aspect_ratio =
          static_cast<double>(image_width) / static_cast<double>(image_height);
      double const viewport_width = aspect_ratio * viewport_height;
      return {viewport_width, viewport_height};
    }

    // Compute axis vectors for the viewport (u, v) in world space.
    void compute_viewport_vectors(CameraBasis const & basis, double viewport_width,
                                  double viewport_height, vector & out_u, vector & out_v) {
      out_u = viewport_width * basis.u;
      out_v = viewport_height * basis.v;
    }

    [[maybe_unused]] CameraSetup make_camera_setup(std::uint32_t image_width,
                                                   std::uint32_t image_height, double vfov_deg,
                                                   vector const & lookfrom, vector const & lookat,
                                                   vector const & vup) {
      // Compute orthonormal basis for the camera.
      CameraBasis basis = compute_camera_basis(lookfrom, lookat, vup);

      // Compute viewport size and axis vectors (split into helpers to reduce complexity).
      auto [viewport_width, viewport_height] =
          compute_viewport_size(image_width, image_height, vfov_deg);

      vector viewport_u;
      vector viewport_v;
      compute_viewport_vectors(basis, viewport_width, viewport_height, viewport_u, viewport_v);

      vector center = lookfrom + basis.forward;

      CameraSetup out;
      out.horizontal        = viewport_u;
      out.vertical          = viewport_v;
      out.lower_left_corner = center - 0.5 * viewport_u - 0.5 * viewport_v;

      // Paso por píxel (dx, dy)
      out.pixel_delta_u = out.horizontal / static_cast<double>(image_width);
      out.pixel_delta_v = out.vertical / static_cast<double>(image_height);

      return out;
    }

  }  // namespace

  camera::camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
                 vector const & lookfrom, vector const & lookat, vector const & vup,
                 std::uint32_t samples_per_pixel, std::uint64_t seed)
      : camera(image_width, image_height, vfov_deg, lookfrom, lookat, vup, samples_per_pixel, seed,
               /*aperture*/ 0.0,
               /*focus_dist*/ 1.0) { }  // tu pinhole actual equivalía a focus_dist=1

  camera::camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
                 vector const & lookfrom, vector const & lookat, vector const & vup,
                 std::uint32_t samples_per_pixel, std::uint64_t seed, double aperture,
                 double focus_dist)
      : m_image_width(image_width), m_image_height(image_height),
        m_samples_per_pixel(samples_per_pixel), m_origin(lookfrom), m_rng(seed), m_dist(0.0, 1.0) {
    // === Base de cámara (igual que tus helpers) ===
    m_w            = (lookfrom - lookat).normalized();  // mira de lookat -> lookfrom
    m_u            = (vup.cross(m_w)).normalized();     // derecha
    m_v            = m_w.cross(m_u);                    // arriba
    vector forward = (-1.0) * m_w;

    // === Viewport desde vfov e imagen ===
    double const theta           = vfov_deg * (std::numbers::pi / 180.0);
    double const half_height     = std::tan(theta * 0.5);
    double const viewport_height = 2.0 * half_height;
    double const aspect_ratio =
        static_cast<double>(image_width) / static_cast<double>(image_height);
    double const viewport_width = aspect_ratio * viewport_height;

    // === Escalado por focus_dist ===
    m_horizontal        = (focus_dist * viewport_width) * m_u;
    m_vertical          = (focus_dist * viewport_height) * m_v;
    vector center       = lookfrom + (focus_dist * forward);
    m_lower_left_corner = center - 0.5 * m_horizontal - 0.5 * m_vertical;

    // === Paso por píxel ===
    m_pixel_delta_u = m_horizontal / static_cast<double>(image_width);
    m_pixel_delta_v = m_vertical / static_cast<double>(image_height);

    // === Lente ===
    m_lens_radius = 0.5 * aperture;
  }

  ray camera::get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t /*sample_id*/) {
    // 1) Jitter subpixel (igual que antes)
    double jitter_x = m_dist(m_rng);
    double jitter_y = m_dist(m_rng);

    double px_f       = static_cast<double>(px) + jitter_x;
    double py_flipped = (static_cast<double>(m_image_height - 1U - py)) + jitter_y;

    vector pixel_sample =
        m_lower_left_corner + px_f * m_pixel_delta_u + py_flipped * m_pixel_delta_v;

    if (m_lens_radius <= 0.0) {
      // Pinhole (comportamiento anterior)
      vector dir = (pixel_sample - m_origin).normalized();
      return {m_origin, dir};
    }

    // 2) DOF: desplaza el origen en el disco de la lente (plano u-v)
    auto [dx, dy] = random_in_unit_disk(m_rng, m_dist);
    vector offset = (dx * m_lens_radius) * m_u + (dy * m_lens_radius) * m_v;

    vector origin = m_origin + offset;
    vector dir    = (pixel_sample - origin).normalized();
    return {origin, dir};
  }

}  // namespace render
