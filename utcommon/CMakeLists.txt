# utcommon/CMakeLists.txt
add_executable(utcommon
  test_vector.cpp
  test_camera.cpp
  test_config.cpp
  test_hits.cpp
  test_misc.cpp
  test_parser_scene.cpp
  
)

target_include_directories(utcommon PRIVATE
  ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(utcommon PRIVATE
  common
  GTest::gtest_main
)

add_test(NAME utcommon COMMAND utcommon)
