# --- BLOQUE TEST PARA UTCOMMON (pegar en utcommon/CMakeLists.txt) ---


add_executable(utcommon
  test_camera.cpp
  test_config.cpp
  test_hits.cpp
  test_misc.cpp
  test_parser_scene.cpp
  test_vector.cpp
  test_scene_parse_min.cpp
  test_ppm_overloads.cpp
  test_config_extras.cpp
)
 target_link_libraries(utcommon PRIVATE gtest_main common)  # ajusta deps

# Registro del test en CTest usando el ejecutable del target (portable)
add_test(NAME utcommon COMMAND $<TARGET_FILE:utcommon>)


