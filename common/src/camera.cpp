#include "render/camera.hpp"

#include <cmath>    // tan, numbers::pi
#include <numbers>  // std::numbers::pi

namespace render {

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

    CameraSetup make_camera_setup(std::uint32_t image_width, std::uint32_t image_height,
                                  double vfov_deg, vector const & lookfrom, vector const & lookat,
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

  // Suppress linter warning about function size/complexity for this constructor.
  camera::camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
                 vector const & lookfrom, vector const & lookat, vector const & vup,
                 std::uint32_t samples_per_pixel, std::uint64_t seed)
      : m_image_width(image_width), m_image_height(image_height),
        m_samples_per_pixel(samples_per_pixel), m_origin(lookfrom), m_rng(seed), m_dist(0.0, 1.0) {
    auto params = make_camera_setup(m_image_width, m_image_height, vfov_deg, lookfrom, lookat, vup);
    m_horizontal        = params.horizontal;
    m_vertical          = params.vertical;
    m_lower_left_corner = params.lower_left_corner;
    m_pixel_delta_u     = params.pixel_delta_u;
    m_pixel_delta_v     = params.pixel_delta_v;
  }

  // Genera un rayo con jitter subpixel.
  // px, py: coordenadas de pixel (0,0) = esquina superior izquierda de la imagen?
  //
  // OJO IMPORTANTE: Convención del plano de imagen:
  // - En los tests hemos supuesto py=0 arriba y crece hacia abajo.
  // - Pero en el plano que hemos construido, v apunta "arriba".
  //   Eso significa que para ir a un píxel más abajo tenemos que RESTAR en vertical.
  //   (Esto cuadra con lo que ya probaste en test_camera.cpp.)
  //
  // sample_id: índice de muestra para el AA. Lo aceptamos en la firma por coherencia,
  // aunque ahora mismo no cambia el RNG (misma dist para todas las muestras).
  //
  ray camera::get_ray(std::uint32_t px, std::uint32_t py, std::uint32_t /*sample_id*/) {
    // --------------------------
    // 1. Jitter subpixel
    // --------------------------
    //
    // dx, dy uniformes en [0,1)
    double jitter_x = m_dist(m_rng);
    double jitter_y = m_dist(m_rng);

    // Coordenadas del subpixel dentro del viewport:
    //
    // pixel_sample =
    //   lower_left_corner
    // + (px + jitter_x) * pixel_delta_u
    // + (py + jitter_y) * pixel_delta_v
    //
    // Peeero, cuidado con el eje y: en imagen py crece hacia abajo,
    // pero m_pixel_delta_v apunta hacia ARRIBA (v).
    // Para alinear con la convención típica de framebuffer,
    // usamos (image_height - 1 - py) en vez de py.
    //
    double px_f       = static_cast<double>(px) + jitter_x;
    double py_flipped = (static_cast<double>(m_image_height - 1U - py)) + jitter_y;

    vector pixel_sample =
        m_lower_left_corner + px_f * m_pixel_delta_u + py_flipped * m_pixel_delta_v;

    // Dirección del rayo = pixel_sample - origen
    vector dir = (pixel_sample - m_origin).normalized();

    return {m_origin, dir};
  }

}  // namespace render
