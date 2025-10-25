#include "render/parser.hpp"

#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace render {

  // ============================================================
  // try_parse_config
  //
  // Formato esperado del archivo de configuración:
  //
  //   width 800
  //   height 450
  //   fov 60
  //   samples 16
  //   seed 1234
  //   lookfrom 0 0 1
  //   lookat   0 0 0
  //   vup      0 1 0
  //
  // Reglas (TA2):
  //  - Si una línea tiene una clave desconocida => error.
  //  - Si el valor tiene basura extra => error.
  //  - Si falta una propiedad, se deja el valor por defecto del struct.
  //  - Validaciones básicas: width>0, height>0, fov en (0,180), samples>0.
  // ============================================================
  std::optional<Config> try_parse_config(std::string const & filename, std::string * err) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      if (err) {
        *err = "Error: cannot open file '" + filename + "'";
      }
      return std::nullopt;
    }

    Config cfg{};  // defaults válidos

    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
      ++line_number;

      // saltar líneas vacías / solo espacios
      bool only_ws = true;
      for (char ch : line) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
          only_ws = false;
          break;
        }
      }
      if (only_ws) {
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      if (key == "width") {
        if (!(iss >> cfg.width)) {
          if (err) {
            *err = "Error: invalid format for 'width' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        if (cfg.width == 0U) {
          if (err) {
            *err = "Error: width must be > 0 in " + filename + ":" + std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "height") {
        if (!(iss >> cfg.height)) {
          if (err) {
            *err = "Error: invalid format for 'height' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        if (cfg.height == 0U) {
          if (err) {
            *err = "Error: height must be > 0 in " + filename + ":" + std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "fov") {
        if (!(iss >> cfg.vertical_fov_deg)) {
          if (err) {
            *err = "Error: invalid format for 'fov' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        if (cfg.vertical_fov_deg <= 0.0 || cfg.vertical_fov_deg >= 180.0) {
          if (err) {
            *err = "Error: fov out of range in " + filename + ":" + std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "samples") {
        if (!(iss >> cfg.samples_per_pixel)) {
          if (err) {
            *err = "Error: invalid format for 'samples' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        if (cfg.samples_per_pixel == 0U) {
          if (err) {
            *err = "Error: samples must be > 0 in " + filename + ":" + std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "seed") {
        if (!(iss >> cfg.seed)) {
          if (err) {
            *err = "Error: invalid format for 'seed' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "lookfrom") {
        if (!(iss >> cfg.lookfrom.x >> cfg.lookfrom.y >> cfg.lookfrom.z)) {
          if (err) {
            *err = "Error: invalid format for 'lookfrom' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "lookat") {
        if (!(iss >> cfg.lookat.x >> cfg.lookat.y >> cfg.lookat.z)) {
          if (err) {
            *err = "Error: invalid format for 'lookat' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "vup") {
        if (!(iss >> cfg.vup.x >> cfg.vup.y >> cfg.vup.z)) {
          if (err) {
            *err = "Error: invalid format for 'vup' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else {
        // clave desconocida
        if (err) {
          *err =
              "Error: invalid key '" + key + "' in " + filename + ":" + std::to_string(line_number);
        }
        return std::nullopt;
      }

      // comprobar basura extra en esa línea
      std::string trailing;
      if (iss >> trailing) {
        if (err) {
          *err = "Error: trailing data after '" +
                 key +
                 "' in " +
                 filename +
                 ":" +
                 std::to_string(line_number);
        }
        return std::nullopt;
      }
    }

    // todo OK
    return cfg;
  }

  // ============================================================
  // try_parse_scene
  //
  // Formato mínimo soportado para TA2:
  //
  //   sphere cx cy cz r
  //
  // Validaciones:
  //   - r > 0
  //   - palabra desconocida => error
  //   - basura extra => error
  // ============================================================
  std::optional<Scene> try_parse_scene(std::string const & filename, std::string * err) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      if (err) {
        *err = "Error: cannot open file '" + filename + "'";
      }
      return std::nullopt;
    }

    Scene scene{};

    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
      ++line_number;
      if (line.empty()) {
        continue;
      }

      std::istringstream iss(line);
      std::string kind;
      if (!(iss >> kind)) {
        continue;
      }

      if (kind == "sphere") {
        double cx, cy, cz, r;
        if (!(iss >> cx >> cy >> cz >> r)) {
          if (err) {
            *err =
                "Error: invalid sphere format in " + filename + ":" + std::to_string(line_number);
          }
          return std::nullopt;
        }

        if (r <= 0.0) {
          if (err) {
            *err = "Error: sphere radius must be > 0 in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

        scene.spheres.push_back(Sphere{
          .center = vector{cx, cy, cz},
          .radius = r,
        });

        // ¿basura extra tras la esfera?
        std::string trailing;
        if (iss >> trailing) {
          if (err) {
            *err = "Error: trailing data after sphere in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else {
        if (err) {
          *err = "Error: unknown object '" +
                 kind +
                 "' in " +
                 filename +
                 ":" +
                 std::to_string(line_number);
        }
        return std::nullopt;
      }
    }

    return scene;
  }

}  // namespace render
