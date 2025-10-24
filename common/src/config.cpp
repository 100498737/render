#include "render/config.hpp"

#include <cctype>
#include <charconv>
#include <fstream>
#include <string>
#include <string_view>

namespace render {

namespace {
inline std::string trim(std::string s) {
  const char* ws = " \t\r\n";
  auto a = s.find_first_not_of(ws);
  if (a == std::string::npos) return {};
  auto b = s.find_last_not_of(ws);
  return s.substr(a, b - a + 1);
}

bool parse_int(std::string_view sv, int& out) {
  sv = std::string_view(trim(std::string(sv)));
  if (sv.empty()) return false;
  auto begin = sv.data();
  auto end   = sv.data() + sv.size();
  int v = 0;
  auto [p, ec] = std::from_chars(begin, end, v);
  if (ec != std::errc() || p != end) return false;
  out = v;
  return true;
}

bool parse_double(std::string_view sv, double& out) {
  sv = std::string_view(trim(std::string(sv)));
  // std::from_chars para double no está implementado en todos los libstdc++; usamos stod
  try {
    size_t pos = 0;
    out = std::stod(std::string(sv), &pos);
    if (pos != sv.size()) return false;
    return true;
  } catch (...) { return false; }
}
} // namespace

std::optional<Config> try_parse_config(const std::string& path, std::string* err) {
  std::ifstream in(path);
  if (!in) {
    if (err) *err = "cannot open file: " + path;
    return std::nullopt;
  }

  Config cfg;
  std::string line;
  int lineno = 0;

  while (std::getline(in, line)) {
    ++lineno;
    auto raw = trim(line);
    if (raw.empty() || raw[0] == '#') continue;

    auto eq = raw.find('=');
    if (eq == std::string::npos) {
      if (err) *err = "line " + std::to_string(lineno) + ": missing '='";
      return std::nullopt;
    }

    auto key = trim(raw.substr(0, eq));
    auto val = trim(raw.substr(eq + 1));

    if (key == "image_width") {
      int v;
      if (!parse_int(val, v) || v < 1) { if (err) *err = "line " + std::to_string(lineno) + ": invalid image_width"; return std::nullopt; }
      cfg.image_width = v;
    } else if (key == "aspect_ratio") {
      double v;
      if (!parse_double(val, v) || v <= 0.0) { if (err) *err = "line " + std::to_string(lineno) + ": invalid aspect_ratio"; return std::nullopt; }
      cfg.aspect_ratio = v;
    } else if (key == "samples_per_pixel") {
      int v;
      if (!parse_int(val, v) || v < 1) { if (err) *err = "line " + std::to_string(lineno) + ": invalid samples_per_pixel"; return std::nullopt; }
      cfg.samples_per_pixel = v;
    } else if (key == "max_depth") {
      int v;
      if (!parse_int(val, v) || v < 1) { if (err) *err = "line " + std::to_string(lineno) + ": invalid max_depth"; return std::nullopt; }
      cfg.max_depth = v;
    } else if (key == "gamma") {
      double v;
      if (!parse_double(val, v) || v <= 0.0) { if (err) *err = "line " + std::to_string(lineno) + ": invalid gamma"; return std::nullopt; }
      cfg.gamma = v;
    } else {
      if (err) *err = "line " + std::to_string(lineno) + ": unknown key '" + key + "'";
      return std::nullopt;
    }
    // Regla "última ocurrencia prevalece": ya la cumplimos al sobrescribir.
  }

  return cfg;
}

} // namespace render
