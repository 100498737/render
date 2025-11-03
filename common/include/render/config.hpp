#pragma once

#include "render/vector.hpp"
#include <cstdint>

namespace render {

  // Configuración de la cámara/render que pide el enunciado
  struct Config {
    std::uint32_t width{400};
    std::uint32_t height{225};

    // campo: FOV vertical en grados
    double vertical_fov_deg{60.0};

    // Parámetros de cámara
    Vec3 lookfrom{
      Vec3{0.0, 0.0, 1.0}
    };
    Vec3 lookat{
      Vec3{0.0, 0.0, 0.0}
    };
    Vec3 vup{
      Vec3{0.0, 1.0, 0.0}
    };

    double aperture{0.0};    // diámetro efectivo del diafragma (0 => pinhole)
    double focus_dist{1.0};  // distancia de enfoque ( > 0 )
    int max_depth{5};        // rebotes máximos del path ( >= 1 )

    // Muestreo y RNG
    std::uint32_t samples_per_pixel{4};
    std::uint64_t seed{42ULL};

    // --- NUEVO: gamma configurable (default 2.2) ---
    // De momento no lo usamos; más adelante conectaremos parser -> writer PPM.
    double gamma{2.2};
  };

}  // namespace render
