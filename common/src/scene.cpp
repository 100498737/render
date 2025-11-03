#include "render/scene.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace render {

  // ───────────────────────── Helpers ─────────────────────────
  static inline std::string trim(std::string s) {
    auto issp = [](int ch) { return std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char c) { return !issp(c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char c) { return !issp(c); }).base(),
            s.end());
    return s;
  }

  static bool parse_float(std::string const & s, double & out) {
    try {
      size_t p = 0;
      out      = std::stod(s, &p);
      return p == s.size();
    } catch (...) {
      return false;
    }
  }

  static bool parse_vec3(std::string const & s, Vec3 & out) {
    // formato: x,y,z (sin espacios)
    std::string a, b, c;
    std::stringstream ss(s);
    if (!std::getline(ss, a, ',')) {
      return false;
    }
    if (!std::getline(ss, b, ',')) {
      return false;
    }
    if (!std::getline(ss, c, ',')) {
      return false;
    }
    double x, y, z;
    if (!parse_float(a, x) || !parse_float(b, y) || !parse_float(c, z)) {
      return false;
    }
    out = Vec3{x, y, z};
    return true;
  }

  static std::string kv_extract(std::string const & kv, char const * keyPrefix) {
    // asume kv="key=value" y keyPrefix="key="
    return kv.substr(std::char_traits<char>::length(keyPrefix));
  }

  static bool normalize(Vec3 & v) {
    double const n = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (n <= 1e-12) {
      return false;
    }
    v.x /= n;
    v.y /= n;
    v.z /= n;
    return true;
  }

  // ───────────────────── Parser principal ────────────────────
  std::optional<Scene> try_parse_scene__shadow_do_not_link(std::string const & path,
                                                           std::string * err) {
    if (err) {
      err->clear();
    }

    std::ifstream in(path);
    if (!in) {
      if (err) {
        *err = "Error: cannot open file '" + path + "'";
      }
      return std::nullopt;
    }

    enum class Phase { Materials, Objects };
    Phase phase = Phase::Materials;

    Scene scn;
    std::unordered_set<std::string> mat_names, sph_names, cyl_names;

    std::string line;
    int line_no = 0;

    auto fail = [&](std::string const & msg) -> std::optional<Scene> {
      if (err) {
        *err = "Error: " + msg + " in " + path + ":" + std::to_string(line_no);
      }
      return std::nullopt;
    };

    while (std::getline(in, line)) {
      ++line_no;
      std::string t = trim(line);
      if (t.empty() || t[0] == '#') {
        continue;
      }

      std::istringstream iss(t);
      std::string head;
      iss >> head;

      // ────────────────────────────────────────────────────────────────────────
      // NUEVO: Sintaxis corta de materiales
      //   matte: <name> R G B
      //   metal: <name> R G B [fuzz]
      //   refractive: <name> IOR
      // ────────────────────────────────────────────────────────────────────────
      if (head == "matte:" || head == "metal:" || head == "refractive:") {
        if (phase == Phase::Objects) {
          return fail("material declared after objects");
        }
        std::string name;
        if (!(iss >> name) || name.empty()) {
          return fail("invalid material header");
        }
        if (!mat_names.insert(name).second) {
          return fail("duplicated material '" + name + "'");
        }

        Material m;
        m.name = name;

        if (head == "matte:") {
          m.kind = MaterialKind::Matte;
          double r{}, g{}, b{};
          if (!(iss >> r >> g >> b)) {
            return fail("invalid format for 'color'");
          }
          m.color = Vec3{r, g, b};
        } else if (head == "metal:") {
          m.kind = MaterialKind::Metal;
          double r{}, g{}, b{};
          if (!(iss >> r >> g >> b)) {
            return fail("invalid format for 'color'");
          }
          m.color = Vec3{r, g, b};
          // fuzz opcional (por defecto 0)
          if (iss.peek() != std::char_traits<char>::eof()) {
            double f{};
            if (!(iss >> f)) {
              return fail("invalid value for 'fuzz'");
            }
            m.fuzz = f;
          }
          if (m.fuzz < 0.0 || m.fuzz > 1.0) {
            return fail("invalid value for 'fuzz' (must be in [0,1])");
          }
        } else {  // "refractive:"
          m.kind = MaterialKind::Refractive;
          double ior{};
          if (!(iss >> ior)) {
            return fail("invalid value for 'ior'");
          }
          m.ior = ior;
          if (m.ior <= 1.0) {
            return fail("invalid value for 'ior' (must be > 1)");
          }
        }

        // no tokens extra
        std::string trailing;
        if (iss >> trailing) {
          return fail("invalid format (trailing data) after material");
        }

        scn.materials.push_back(std::move(m));
        continue;
      }

      // ────────────────────────────────────────────────────────────────────────
      // Sintaxis larga de materiales (la tuya original)
      //   material <kind> <name> color=... | fuzz=... | ior=...
      // ────────────────────────────────────────────────────────────────────────
      if (head == "material") {
        if (phase == Phase::Objects) {
          return fail("material declared after objects");
        }

        std::string kindStr, name;
        iss >> kindStr >> name;
        if (kindStr.empty() || name.empty()) {
          return fail("invalid material header");
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
          return fail("unknown material kind '" + kindStr + "'");
        }

        if (!mat_names.insert(name).second) {
          return fail("duplicated material '" + name + "'");
        }

        std::string kv;
        while (iss >> kv) {
          if (m.kind != MaterialKind::Refractive && kv.rfind("color=", 0) == 0) {
            std::string v = kv_extract(kv, "color=");
            if (!parse_vec3(v, m.color)) {
              return fail("invalid format for 'color'");
            }
          } else if (m.kind == MaterialKind::Metal && kv.rfind("fuzz=", 0) == 0) {
            std::string v = kv_extract(kv, "fuzz=");
            if (!parse_float(v, m.fuzz)) {
              return fail("invalid value for 'fuzz'");
            }
          } else if (m.kind == MaterialKind::Refractive && kv.rfind("ior=", 0) == 0) {
            std::string v = kv_extract(kv, "ior=");
            if (!parse_float(v, m.ior)) {
              return fail("invalid value for 'ior'");
            }
          } else {
            std::string key = kv.substr(0, kv.find('='));
            return fail("unknown key '" + key + "' for material '" + kindStr + "'");
          }
        }

        if (m.kind == MaterialKind::Metal) {
          if (m.fuzz < 0.0 || m.fuzz > 1.0) {
            return fail("invalid value for 'fuzz' (must be in [0,1])");
          }
        } else if (m.kind == MaterialKind::Refractive) {
          if (m.ior <= 1.0) {
            return fail("invalid value for 'ior' (must be > 1)");
          }
        }

        scn.materials.push_back(std::move(m));
        continue;
      }

      // ────────────────────────────────────────────────────────────────────────
      // NUEVO: Sintaxis corta de objetos
      //   sphere: cx cy cz r <mat_name>
      //   cylinder: bx by bz ax ay az h r <mat_name>
      // ────────────────────────────────────────────────────────────────────────
      if (head == "sphere:" || head == "cylinder:") {
        if (mat_names.empty()) {
          return fail("object declared before materials");
        }
        phase = Phase::Objects;

        if (head == "sphere:") {
          std::string mat;
          Sphere s;
          if (!(iss >> s.center.x >> s.center.y >> s.center.z >> s.radius >> mat)) {
            return fail("invalid 'sphere:' format (cx cy cz r mat)");
          }
          if (s.radius <= 0.0) {
            return fail("invalid value for 'radius' (must be > 0)");
          }
          if (!mat_names.count(mat)) {
            return fail("unknown material '" + mat + "'");
          }
          // nombre autogenerado (único)
          std::string base = "sphere";
          std::string name =
              base + "_" + std::to_string(static_cast<unsigned>(scn.spheres.size() + 1));
          while (!sph_names.insert(name).second) {
            name += "_";
          }
          s.name = std::move(name);
          s.mat  = std::move(mat);
          scn.spheres.push_back(std::move(s));
          continue;
        } else {  // "cylinder:"
          Cylinder c;
          std::string mat;
          if (!(iss >>
                c.base.x >>
                c.base.y >>
                c.base.z >>
                c.axis.x >>
                c.axis.y >>
                c.axis.z >>
                c.height >>
                c.radius >>
                mat))
          {
            return fail("invalid 'cylinder:' format (bx by bz ax ay az h r mat)");
          }
          if (!normalize(c.axis)) {
            return fail("invalid value for 'axis' (zero vector)");
          }
          if (c.height <= 0.0) {
            return fail("invalid value for 'height' (must be > 0)");
          }
          if (c.radius <= 0.0) {
            return fail("invalid value for 'radius' (must be > 0)");
          }
          if (!mat_names.count(mat)) {
            return fail("unknown material '" + mat + "'");
          }

          std::string base = "cylinder";
          std::string name =
              base + "_" + std::to_string(static_cast<unsigned>(scn.cylinders.size() + 1));
          while (!cyl_names.insert(name).second) {
            name += "_";
          }
          c.name = std::move(name);
          c.mat  = std::move(mat);
          scn.cylinders.push_back(std::move(c));
          continue;
        }
      }

      // ────────────────────────────────────────────────────────────────────────
      // Sintaxis larga de objetos (la tuya original)
      // ────────────────────────────────────────────────────────────────────────
      if (head == "sphere" || head == "cylinder") {
        if (mat_names.empty()) {
          return fail("object declared before materials");
        }
        phase = Phase::Objects;

        if (head == "sphere") {
          std::string name;
          iss >> name;
          if (name.empty()) {
            return fail("invalid sphere header");
          }
          if (!sph_names.insert(name).second) {
            return fail("duplicated object '" + name + "'");
          }

          Sphere s;
          s.name         = name;
          bool ok_center = false, ok_radius = false, ok_mat = false;

          std::string kv;
          while (iss >> kv) {
            if (kv.rfind("center=", 0) == 0) {
              ok_center = true;
              if (!parse_vec3(kv_extract(kv, "center="), s.center)) {
                return fail("invalid format for 'center' (x,y,z)");
              }
            } else if (kv.rfind("radius=", 0) == 0) {
              ok_radius = true;
              if (!parse_float(kv_extract(kv, "radius="), s.radius) || s.radius <= 0.0) {
                return fail("invalid value for 'radius' (must be > 0)");
              }
            } else if (kv.rfind("mat=", 0) == 0) {
              ok_mat = true;
              s.mat  = kv_extract(kv, "mat=");
            } else {
              std::string key = kv.substr(0, kv.find('='));
              return fail("unknown key '" + key + "' for 'sphere'");
            }
          }

          if (!ok_center || !ok_radius || !ok_mat) {
            return fail("missing required keys for 'sphere'");
          }
          if (!mat_names.count(s.mat)) {
            return fail("unknown material '" + s.mat + "'");
          }
          scn.spheres.push_back(std::move(s));
          continue;
        }

        // cylinder (largo)
        {
          std::string name;
          iss >> name;
          if (name.empty()) {
            return fail("invalid cylinder header");
          }
          if (!cyl_names.insert(name).second) {
            return fail("duplicated object '" + name + "'");
          }

          Cylinder c;
          c.name       = name;
          bool ok_base = false, ok_axis = false, ok_height = false, ok_radius = false,
               ok_mat = false;

          std::string kv;
          while (iss >> kv) {
            if (kv.rfind("base=", 0) == 0) {
              ok_base = true;
              if (!parse_vec3(kv_extract(kv, "base="), c.base)) {
                return fail("invalid format for 'base' (x,y,z)");
              }
            } else if (kv.rfind("axis=", 0) == 0) {
              ok_axis = true;
              if (!parse_vec3(kv_extract(kv, "axis="), c.axis)) {
                return fail("invalid format for 'axis' (x,y,z)");
              }
            } else if (kv.rfind("height=", 0) == 0) {
              ok_height = true;
              if (!parse_float(kv_extract(kv, "height="), c.height) || c.height <= 0.0) {
                return fail("invalid value for 'height' (must be > 0)");
              }
            } else if (kv.rfind("radius=", 0) == 0) {
              ok_radius = true;
              if (!parse_float(kv_extract(kv, "radius="), c.radius) || c.radius <= 0.0) {
                return fail("invalid value for 'radius' (must be > 0)");
              }
            } else if (kv.rfind("mat=", 0) == 0) {
              ok_mat = true;
              c.mat  = kv_extract(kv, "mat=");
            } else {
              std::string key = kv.substr(0, kv.find('='));
              return fail("unknown key '" + key + "' for 'cylinder'");
            }
          }

          if (!ok_base || !ok_axis || !ok_height || !ok_radius || !ok_mat) {
            return fail("missing required keys for 'cylinder'");
          }
          if (!normalize(c.axis)) {
            return fail("invalid value for 'axis' (zero vector)");
          }
          if (!mat_names.count(c.mat)) {
            return fail("unknown material '" + c.mat + "'");
          }
          scn.cylinders.push_back(std::move(c));
          continue;
        }
      }

      // Directiva desconocida
      return fail("unknown directive '" + head + "'");
    }

    return scn;
  }

  SceneStats scene_stats(Scene const & scn) {
    return SceneStats{scn.spheres.size(), scn.cylinders.size()};  // ajusta nombres si difieren
  }

}  // namespace render
