#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

#include "render/parser.hpp"
#include "render/scene.hpp"
#include "render/vector.hpp"

namespace render {

  // --------------------------- util comunes ---------------------------
  static inline void ltrim(std::string & s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
  }

  static inline void rtrim(std::string & s) {
    s.erase(
        std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base(),
        s.end());
  }

  static inline void trim(std::string & s) {
    ltrim(s);
    rtrim(s);
  }

  static inline bool strip_bom_if_first_line(std::string & line, int line_no) {
    if (line_no == 1 &&
        line.size() >= 3 &&
        (unsigned char) line[0] == 0xEF &&
        (unsigned char) line[1] == 0xBB &&
        (unsigned char) line[2] == 0xBF)
    {
      line.erase(0, 3);
      return true;
    }
    return false;
  }

  // devuelve false si la línea es vacía o comentario tras recortar y cortar el inline-#
  static inline bool clean_and_is_content_line(std::string & line) {
    trim(line);
    if (line.empty() || line[0] == '#') {
      return false;
    }
    if (auto pos = line.find('#'); pos != std::string::npos) {
      line.erase(pos);
      trim(line);
      if (line.empty()) {
        return false;
      }
    }
    return true;
  }

  static inline bool parse_double_sv(std::string_view sv, double & out) {
    char const * b = sv.data();
    char const * e = sv.data() + sv.size();
    auto res       = std::from_chars(b, e, out);
    return (res.ec == std::errc{}) && (res.ptr == e);
  }

  static inline bool parse_vec3_csv(std::string_view s, Vec3 & v) {
    // formato "x,y,z" sin espacios
    size_t p1 = s.find(',');
    if (p1 == std::string_view::npos) {
      return false;
    }
    size_t p2 = s.find(',', p1 + 1);
    if (p2 == std::string_view::npos) {
      return false;
    }
    double x, y, z;
    if (!parse_double_sv(s.substr(0, p1), x)) {
      return false;
    }
    if (!parse_double_sv(s.substr(p1 + 1, p2 - p1 - 1), y)) {
      return false;
    }
    if (!parse_double_sv(s.substr(p2 + 1), z)) {
      return false;
    }
    v.x = x;
    v.y = y;
    v.z = z;
    return true;
  }

  static inline bool normalize_safe(Vec3 & v) {
    double const n = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (n <= 1e-12) {
      return false;
    }
    v.x /= n;
    v.y /= n;
    v.z /= n;
    return true;
  }

  // ======================= TA2: CONFIG PARSER =======================
  std::optional<Config> try_parse_config(std::string const & filename, std::string * err) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      if (err) {
        *err = "Error: cannot open file '" + filename + "'";
      }
      return std::nullopt;
    }

    Config cfg{};  // defaults
    std::string line;
    int line_number = 0;

    while (std::getline(file, line)) {
      ++line_number;

      strip_bom_if_first_line(line, line_number);
      if (!clean_and_is_content_line(line)) {
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      // width / height
      if (key == "width" || key == "image_width" || key == "w") {
        if (!(iss >> cfg.width)) {  // <— parseo falla => invalid format
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

      } else if (key == "height" || key == "image_height" || key == "h") {
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

      } else if (key == "fov" || key == "vfov") {
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

      } else if (key == "samples" || key == "samples_per_pixel" || key == "spp") {
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

      } else if (key == "seed" || key == "rng_seed") {
        if (!(iss >> cfg.seed)) {
          if (err) {
            *err = "Error: invalid format for 'seed' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "lookfrom" || key == "camera_from" || key == "from") {
        if (!(iss >> cfg.lookfrom.x >> cfg.lookfrom.y >> cfg.lookfrom.z)) {
          if (err) {
            *err = "Error: invalid format for 'lookfrom' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "lookat" || key == "camera_at" || key == "at") {
        if (!(iss >> cfg.lookat.x >> cfg.lookat.y >> cfg.lookat.z)) {
          if (err) {
            *err = "Error: invalid format for 'lookat' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

      } else if (key == "vup" || key == "camera_up" || key == "up") {
        if (!(iss >> cfg.vup.x >> cfg.vup.y >> cfg.vup.z)) {
          if (err) {
            *err = "Error: invalid format for 'vup' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }

        // aperture / lens radius
      } else if (key == "aperture" ||
                 key == "lens_radius" ||
                 key == "aperture_radius" ||
                 key == "lens")
      {
        double v{};
        if (!(iss >> v)) {
          if (err) {
            *err = "Error: invalid format for 'aperture' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        if (v < 0.0) {
          if (err) {
            *err = "Error: invalid value for 'aperture' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        cfg.aperture = v;  // <<< antes: (void)v;

        // focus distance
      } else if (key == "focus_dist" ||
                 key == "focus_distance" ||
                 key == "focal_dist" ||
                 key == "focal_distance")
      {
        double v{};
        if (!(iss >> v)) {
          if (err) {
            *err = "Error: invalid format for 'focus_dist' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        if (v <= 0.0) {
          if (err) {
            *err = "Error: invalid value for 'focus_dist' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        cfg.focus_dist = v;  // <<< antes: (void)v;

        // max depth / bounces
      } else if (key == "max_depth" ||
                 key == "max_bounces" ||
                 key == "depth_limit" ||
                 key == "bounce_limit")
      {
        int v{};
        if (!(iss >> v)) {
          if (err) {
            *err = "Error: invalid format for 'max_depth' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        if (v <= 0) {
          if (err) {
            *err = "Error: invalid value for 'max_depth' in " +
                   filename +
                   ":" +
                   std::to_string(line_number);
          }
          return std::nullopt;
        }
        cfg.max_depth = v;  // <<< antes: (void)v;

      } else {
        if (err) {
          *err =
              "Error: invalid key '" + key + "' in " + filename + ":" + std::to_string(line_number);
        }
        return std::nullopt;
      }

      // (mantén justo después tu chequeo de trailing tokens)
      std::string trailing;
      if (iss >> trailing) {
        if (err) {
          *err = "Error: invalid format (trailing data) after '" +
                 key +
                 "' in " +
                 filename +
                 ":" +
                 std::to_string(line_number);
        }
        return std::nullopt;
      }
    }

    return cfg;
  }

  // ======================= TA3: SCENE PARSER =======================
  std::optional<Scene> try_parse_scene(std::string const & filename, std::string * err) {
    if (err) {
      err->clear();
    }

    std::ifstream in(filename);
    if (!in) {
      if (err) {
        *err = "Error: cannot open file '" + filename + "'";
      }
      return std::nullopt;
    }

    auto fail = [&](int line, std::string const & msg) -> std::optional<Scene> {
      if (err) {
        *err = "Error: " + msg + " in " + filename + ":" + std::to_string(line);
      }
      return std::nullopt;
    };

    Scene scn;
    std::unordered_set<std::string> mat_names, sph_names, cyl_names;
    enum class Phase { Materials, Objects };
    Phase phase = Phase::Materials;

    std::string raw;
    int line_no = 0;

    while (std::getline(in, raw)) {
      ++line_no;
      strip_bom_if_first_line(raw, line_no);
      if (!clean_and_is_content_line(raw)) {
        continue;
      }

      std::istringstream iss(raw);
      std::string head;
      if (!(iss >> head)) {
        continue;
      }

      auto is_kind = [](std::string const & s) {
        return s == "matte" || s == "metal" || s == "refractive";
      };

      // -------- Material --------
      if (head == "material" || is_kind(head)) {
        std::string kindStr, name;
        if (head == "material") {
          if (!(iss >> kindStr >> name)) {
            return fail(line_no, "invalid material header");
          }
        } else {
          kindStr = head;
          if (!(iss >> name)) {
            return fail(line_no, "invalid material header");
          }
        }
        if (phase == Phase::Objects) {
          return fail(line_no, "material declared after objects");
        }

        Material m{};
        m.name = name;
        if (kindStr == "matte") {
          m.kind = MaterialKind::Matte;
        } else if (kindStr == "metal") {
          m.kind = MaterialKind::Metal;
        } else if (kindStr == "refractive") {
          m.kind = MaterialKind::Refractive;
        } else {
          return fail(line_no, "unknown material kind '" + kindStr + "'");
        }

        if (!mat_names.insert(name).second) {
          return fail(line_no, "duplicated material '" + name + "'");
        }

        std::string tok;
        while (iss >> tok) {
          if ((tok == "color") && m.kind != MaterialKind::Refractive) {
            std::streampos pos = iss.tellg();
            double r, g, b;
            if (iss >> r >> g >> b) {
              m.color = Vec3{r, g, b};
            } else {
              iss.clear();
              iss.seekg(pos);
              std::string csv;
              if (!(iss >> csv)) {
                return fail(line_no, "invalid format for 'color'");
              }
              Vec3 c{};
              if (!parse_vec3_csv(csv, c)) {
                return fail(line_no, "invalid format for 'color'");
              }
              m.color = c;
            }
          } else if (tok.rfind("color=", 0) == 0 && m.kind != MaterialKind::Refractive) {
            Vec3 c{};
            if (!parse_vec3_csv(std::string_view(tok).substr(6), c)) {
              return fail(line_no, "invalid format for 'color'");
            }
            m.color = c;

          } else if ((tok == "fuzz") && m.kind == MaterialKind::Metal) {
            if (!(iss >> m.fuzz)) {
              return fail(line_no, "invalid value for 'fuzz'");
            }
          } else if (tok.rfind("fuzz=", 0) == 0 && m.kind == MaterialKind::Metal) {
            if (!parse_double_sv(std::string_view(tok).substr(5), m.fuzz)) {
              return fail(line_no, "invalid value for 'fuzz'");
            }

          } else if ((tok == "ior") && m.kind == MaterialKind::Refractive) {
            if (!(iss >> m.ior)) {
              return fail(line_no, "invalid value for 'ior'");
            }
          } else if (tok.rfind("ior=", 0) == 0 && m.kind == MaterialKind::Refractive) {
            if (!parse_double_sv(std::string_view(tok).substr(4), m.ior)) {
              return fail(line_no, "invalid value for 'ior'");
            }

          } else {
            std::string key = tok.substr(0, tok.find('='));
            return fail(line_no, "unknown key '" + key + "' for material '" + kindStr + "'");
          }
        }

        if (m.kind == MaterialKind::Metal && (m.fuzz < 0.0 || m.fuzz > 1.0)) {
          return fail(line_no, "invalid value for 'fuzz' (must be in [0,1])");
        }
        if (m.kind == MaterialKind::Refractive && m.ior <= 1.0) {
          return fail(line_no, "invalid value for 'ior' (must be > 1)");
        }

        scn.materials.push_back(std::move(m));
        continue;
      }

      // -------- Objetos --------
      if (head == "sphere" || head == "cylinder") {
        if (mat_names.empty()) {
          return fail(line_no, "object declared before materials");
        }
        phase = Phase::Objects;

        if (head == "sphere") {
          // Soporta: (1) sphere NAME center=.. radius=.. mat=..
          //          (2) sphere NAME cx cy cz r mat
          //          (3) sphere cx cy cz r   (legacy sin nombre/material)
          std::string first;
          if (!(iss >> first)) {
            return fail(line_no, "invalid sphere format");
          }

          // detectar si viene K=V
          std::streampos pos = iss.tellg();
          std::string nxt;
          iss >> nxt;
          bool is_kv = (!nxt.empty() and nxt.find('=') != std::string::npos);
          iss.clear();
          iss.seekg(pos);

          Sphere s{};
          if (is_kv) {
            s.name   = first;
            bool okc = false, okr = false, okm = false;
            std::string kv;
            while (iss >> kv) {
              if (kv.rfind("center=", 0) == 0) {
                Vec3 c{};
                if (!parse_vec3_csv(std::string_view(kv).substr(7), c)) {
                  return fail(line_no, "invalid format for 'center' (x,y,z)");
                }
                s.center = c;
                okc      = true;
              } else if (kv.rfind("radius=", 0) == 0) {
                if (!parse_double_sv(std::string_view(kv).substr(7), s.radius) || s.radius <= 0.0) {
                  return fail(line_no, "sphere radius must be > 0");
                }
                okr = true;
              } else if (kv.rfind("mat=", 0) == 0) {
                s.mat = kv.substr(4);
                okm   = true;
              } else {
                return fail(line_no,
                            "unknown key '" + kv.substr(0, kv.find('=')) + "' for 'sphere'");
              }
            }
            if (!(okc and okr and okm)) {
              return fail(line_no, "invalid sphere format");
            }
            if (!sph_names.insert(s.name).second) {
              return fail(line_no, "duplicated object '" + s.name + "'");
            }
            if (!mat_names.count(s.mat)) {
              return fail(line_no, "unknown material '" + s.mat + "'");
            }
            scn.spheres.push_back(std::move(s));

          } else {
            // ¿legacy o con nombre?
            double testnum{};
            if (parse_double_sv(first, testnum)) {
              Vec3 c{};
              c.x = testnum;
              if (!(iss >> c.y >> c.z)) {
                return fail(line_no, "invalid sphere format");
              }
              double r{};
              if (!(iss >> r)) {
                return fail(line_no, "invalid sphere format");
              }
              if (r <= 0.0) {
                return fail(line_no, "sphere radius must be > 0");
              }
              Sphere sl{};
              sl.name   = "__legacy" + std::to_string(line_no);
              sl.center = c;
              sl.radius = r;
              scn.spheres.push_back(std::move(sl));
            } else {
              s.name = first;
              if (!sph_names.insert(s.name).second) {
                return fail(line_no, "duplicated object '" + s.name + "'");
              }
              if (!(iss >> s.center.x >> s.center.y >> s.center.z >> s.radius)) {
                return fail(line_no, "invalid sphere format");
              }
              if (s.radius <= 0.0) {
                return fail(line_no, "sphere radius must be > 0");
              }
              if (!(iss >> s.mat)) {
                return fail(line_no, "invalid sphere format");
              }
              if (!mat_names.count(s.mat)) {
                return fail(line_no, "unknown material '" + s.mat + "'");
              }
              scn.spheres.push_back(std::move(s));
              std::string extra;
              if (iss >> extra) {
                return fail(line_no, "trailing data after sphere");
              }
            }
          }
          continue;
        }

        if (head == "cylinder") {
          Cylinder c{};
          std::string name;
          if (!(iss >> name)) {
            return fail(line_no, "invalid cylinder header");
          }
          if (!cyl_names.insert(name).second) {
            return fail(line_no, "duplicated object '" + name + "'");
          }
          c.name = name;

          if (!(iss >>
                c.base.x >>
                c.base.y >>
                c.base.z >>
                c.axis.x >>
                c.axis.y >>
                c.axis.z >>
                c.height >>
                c.radius))
          {
            return fail(line_no, "invalid cylinder format");
          }

          if (c.height <= 0.0 || c.radius <= 0.0) {
            return fail(line_no, "invalid value for 'height'/'radius' (must be > 0)");
          }

          if (!(iss >> c.mat)) {
            return fail(line_no, "invalid cylinder format");
          }
          if (!normalize_safe(c.axis)) {
            return fail(line_no, "invalid value for 'axis' (zero vector)");
          }
          if (!mat_names.count(c.mat)) {
            return fail(line_no, "unknown material '" + c.mat + "'");
          }

          scn.cylinders.push_back(std::move(c));
          std::string extra;
          if (iss >> extra) {
            return fail(line_no, "trailing data after cylinder");
          }
          continue;
        }
      }

      // directiva desconocida
      return fail(line_no, "unknown object '" + head + "'");
    }

    return scn;
  }

}  // namespace render
