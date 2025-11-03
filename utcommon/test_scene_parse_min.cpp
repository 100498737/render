#include "render/scene.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <optional>
#include <string>

// Declaración adelantada (no está en el .hpp)
namespace render {

  std::optional<Scene> try_parse_scene__shadow_do_not_link(std::string const & path,
                                                           std::string * err);

}

using namespace render;

[[maybe_unused]] static std::string write_tmp(char const * name, std::string const & text) {
  std::string p = std::string("/tmp/") + name;
  std::ofstream f(p);
  f << text;
  return p;
}
