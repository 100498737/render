#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>

#include "render/parser.hpp"
#include "render/scene.hpp"
#include "render/vector.hpp"

namespace render {

  // ======================= TA2: CONFIG PARSER (tu versión) =====================

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
        if (err) {
          *err =
              "Error: invalid key '" + key + "' in " + filename + ":" + std::to_string(line_number);
        }
        return std::nullopt;
      }

      // basura extra al final de línea
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

    return cfg;
  }

  // ======================= TA3: SCENE PARSER =====================

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

    auto parse_float = [](std::string_view s, double & out) -> bool {
      try {
        size_t p = 0;
        out      = std::stod(std::string(s), &p);
        return p == s.size();
      } catch (...) {
        return false;
      }
    };
    auto parse_vec3_csv = [&](std::string_view s, Vec3 & v) -> bool {
      // "x,y,z" sin espacios
      std::string a, b, c;
      std::stringstream ss{std::string(s)};
      if (!std::getline(ss, a, ',')) {
        return false;
      }
      if (!std::getline(ss, b, ',')) {
        return false;
      }
      if (!std::getline(ss, c, ',')) {
        return false;
      }
      return parse_float(a, v.x) && parse_float(b, v.y) && parse_float(c, v.z);
    };
    auto normalize = [](Vec3 & v) -> bool {
      double const n = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
      if (n <= 1e-12) {
        return false;
      }
      v.x /= n;
      v.y /= n;
      v.z /= n;
      return true;
    };

    std::string line_raw;
    int line_no = 0;
    while (std::getline(in, line_raw)) {
      ++line_no;
      // trim
      std::string t = line_raw;
      auto ltrim    = [](std::string & s) {
        s.erase(s.begin(),
                   std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
      };
      auto rtrim = [](std::string & s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); })
                    .base(),
                s.end());
      };
      ltrim(t);
      rtrim(t);
      if (t.empty() || t[0] == '#') {
        continue;
      }

      std::istringstream iss(t);
      std::string head;
      if (!(iss >> head)) {
        continue;
      }

      // -------- Materiales --------
      auto is_kind = [&](std::string const & s) {
        return s == "matte" || s == "metal" || s == "refractive";
      };
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

        Material m;
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
            // aceptar "color r g b" o "color r,g,b"
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
            if (!parse_vec3_csv(tok.substr(6), c)) {
              return fail(line_no, "invalid format for 'color'");
            }
            m.color = c;
          } else if ((tok == "fuzz") && m.kind == MaterialKind::Metal) {
            if (!(iss >> m.fuzz)) {
              return fail(line_no, "invalid value for 'fuzz'");
            }
          } else if (tok.rfind("fuzz=", 0) == 0 && m.kind == MaterialKind::Metal) {
            if (!parse_float(tok.substr(5), m.fuzz)) {
              return fail(line_no, "invalid value for 'fuzz'");
            }
          } else if ((tok == "ior") && m.kind == MaterialKind::Refractive) {
            if (!(iss >> m.ior)) {
              return fail(line_no, "invalid value for 'ior'");
            }
          } else if (tok.rfind("ior=", 0) == 0 && m.kind == MaterialKind::Refractive) {
            if (!parse_float(tok.substr(4), m.ior)) {
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
          // 1) sphere NAME center=... radius=... mat=...
          // 2) sphere NAME cx cy cz r mat
          // 3) sphere cx cy cz r   (legacy)
          std::string first;
          if (!(iss >> first)) {
            return fail(line_no, "invalid sphere format");
          }

          // ¿siguiente token K=V?
          std::streampos pos = iss.tellg();
          std::string nxt;
          iss >> nxt;
          bool is_kv = (!nxt.empty() && nxt.find('=') != std::string::npos);
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
                if (!parse_vec3_csv(kv.substr(7), c)) {
                  return fail(line_no, "invalid format for 'center' (x,y,z)");
                }
                s.center = c;
                okc      = true;
              } else if (kv.rfind("radius=", 0) == 0) {
                if (!parse_float(kv.substr(7), s.radius) || s.radius <= 0.0) {
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
            if (!(okc && okr && okm)) {
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
            double testnum;
            if (parse_float(first, testnum)) {
              // legacy: first era cx
              Vec3 c{};
              c.x = testnum;
              if (!(iss >> c.y >> c.z)) {
                return fail(line_no, "invalid sphere format");
              }
              double r;
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
          if (!normalize(c.axis)) {
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

      // Directiva desconocida
      return fail(line_no, "unknown object '" + head + "'");
    }

    return scn;
  }

}  // namespace render
