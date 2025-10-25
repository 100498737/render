#include "render/camera.hpp"

#include <cmath>    // tan, numbers::pi
#include <numbers>  // std::numbers::pi

namespace render {

  camera::camera(std::uint32_t image_width, std::uint32_t image_height, double vfov_deg,
                 vector const & lookfrom, vector const & lookat, vector const & vup,
                 std::uint32_t samples_per_pixel, std::uint64_t seed)
      : m_image_width(image_width), m_image_height(image_height),
        m_samples_per_pixel(samples_per_pixel), m_origin(lookfrom), m_rng(seed), m_dist(0.0, 1.0) {
    // --------------------------
    // 1. Sistema de referencia
    // --------------------------
    //
    // w = dirección "hacia atrás" (de la escena hacia la cámara)
    // u = "derecha" cámara
    // v = "arriba" cámara
    //
    // Esto coincide con lo que se suele enseñar en ray tracing básico:
    //
    //   w = normalize(lookfrom - lookat)
    //   u = normalize(vup × w)
    //   v = w × u
    //
    vector w = (lookfrom - lookat).normalized();  // mira desde lookat hacia lookfrom
    vector u = (vup.cross(w)).normalized();       // derecha ortonormal
    vector v = w.cross(u);                        // arriba ortonormal

    // --------------------------
    // 2. Plano de imagen
    // --------------------------
    //
    // vfov_deg es el FOV vertical en grados.
    // Convertimos a radianes y calculamos la altura del plano de imagen a distancia focal 1.
    //
    double const theta           = vfov_deg * (std::numbers::pi / 180.0);  // rad
    double const half_height     = std::tan(theta * 0.5);                  // tan(vfov/2)
    double const viewport_height = 2.0 * half_height;

    double const aspect_ratio =
        static_cast<double>(image_width) / static_cast<double>(image_height);
    double const viewport_width = aspect_ratio * viewport_height;

    // En un pinhole clásico, el plano de imagen está a df = 1 delante de la cámara
    // en la dirección "adelante". Nuestro "adelante" en mundo es -w (porque w mira hacia atrás).
    //
    vector forward = (-1.0) * w;  // dirección de visión

    // Los ejes del plano de imagen en coordenadas de mundo:
    //
    // horizontal (u) y vertical (v), escalados al tamaño real del viewport
    //
    vector viewport_u = viewport_width * u;
    vector viewport_v = viewport_height * v;

    // Guardamos estos anchos/altos para luego construir el rayo de cada píxel:
    m_horizontal = viewport_u;
    m_vertical   = viewport_v;

    // El centro del plano de imagen está a 1 unidad delante de la cámara:
    //
    //   C = O + forward
    //
    vector center = m_origin + forward;

    // La esquina inferior izquierda del plano:
    //
    //   lower_left = C
    //              - 0.5 * viewport_u
    //              - 0.5 * viewport_v
    //
    m_lower_left_corner = center - 0.5 * viewport_u - 0.5 * viewport_v;

    // --------------------------
    // 3. Paso por píxel (dx, dy)
    // --------------------------
    //
    // Cada píxel es (1 / image_width) del ancho total del viewport_u.
    // Igual para vertical.
    //
    m_pixel_delta_u = m_horizontal / static_cast<double>(m_image_width);
    m_pixel_delta_v = m_vertical / static_cast<double>(m_image_height);
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

    return ray(m_origin, dir);
  }

}  // namespace render
