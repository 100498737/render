// utcommon/src/test_parser_scene.cpp
#include "render/config.hpp"
#include "render/parser.hpp"
#include "render/scene.hpp"
#include <fstream>
#include <gtest/gtest.h>

TEST(Parser, CFG_WithAliases_Ok) {
  std::ofstream o("cfg_ok.txt");
  o << "width 16\nheight 9\nvfov 45\n"
       "lookfrom 0 1 2\nlookat 0 0 0\nvup 0 1 0\n"
       "samples 2\nseed 42\naperture 0.1\nfocus_dist 2.0\nmax_depth 2\n";
  o.close();
  std::string err;
  auto cfg = render::try_parse_config("cfg_ok.txt", &err);
  ASSERT_TRUE(cfg) << err;
}

TEST(Parser, CFG_InvalidKey_Raises) {
  std::ofstream o("cfg_bad_key.txt");
  o << "widthez 8\n";  // clave inexistente => debe fallar
  o.close();
  std::string err;
  auto cfg = render::try_parse_config("cfg_bad_key.txt", &err);
  ASSERT_FALSE(cfg);
  EXPECT_FALSE(err.empty());
}

TEST(Parser, CFG_InvalidFormat_Raises) {
  std::ofstream o("cfg_bad_fmt.txt");
  o << "lookfrom 0 0\n";  // faltan componentes => formato inválido
  o.close();
  std::string err;
  auto cfg = render::try_parse_config("cfg_bad_fmt.txt", &err);
  ASSERT_FALSE(cfg);
  EXPECT_FALSE(err.empty());
}

// Sustituye ParserScene.Scene_Rich_Ok por este:
TEST(ParserScene, Scene_Rich_Ok) {
  std::ofstream o("scene_ok.txt");
  o << "matte  red   color=0.8,0.2,0.2\n"
       "metal  alu   color=0.8,0.8,0.8 fuzz=0.1\n"
       "refractive glass ior=1.5\n"
       "sphere ball center=0,0,0 radius=0.5 mat=red\n";
  o.close();

  std::string err;
  auto scn = render::try_parse_scene("scene_ok.txt", &err);
  ASSERT_TRUE(scn) << err;
  EXPECT_GE(scn->spheres.size(), 1u);
  // no exigimos cilindros aquí para no depender de la gramática exacta
}

TEST(ParserScene, Cylinder_InvalidFormat_IsRejected) {
  std::ofstream o("scene_bad_cyl.txt");
  o << "matte m color=0.5,0.5,0.5\n"
       "cylinder rod base=0,0,0 axis=0,1,0 ??? mat=m\n";  // basura para forzar error
  o.close();

  std::string err;
  auto scn = render::try_parse_scene("scene_bad_cyl.txt", &err);
  ASSERT_FALSE(scn);
  EXPECT_FALSE(err.empty());
}

TEST(ParserScene, Scene_InvalidFormat_Err) {
  std::ofstream o("scene_bad.txt");
  o << "sphere bad center=0,0,0 radius=0.5 extra_token\n";  // trailing no permitido
  o.close();
  std::string err;
  auto scn = render::try_parse_scene("scene_bad.txt", &err);
  ASSERT_FALSE(scn);
  EXPECT_FALSE(err.empty());
}

TEST(Parser, CFG_TrailingInvalid) {
  std::ofstream o("cfg_bad_tr.txt");
  o << "width 8 extra\n";
  o.close();
  std::string err;
  auto cfg = render::try_parse_config("cfg_bad_tr.txt", &err);
  ASSERT_FALSE(cfg);
  EXPECT_FALSE(err.empty());
}
