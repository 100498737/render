cmake_minimum_required(VERSION 3.24)

# Ejecutable de tests AOS
add_executable(utaos
  test_image_aos.cpp
)

# Includes (AOS + common + gtest)
target_include_directories(utaos PRIVATE
  ${CMAKE_SOURCE_DIR}/aos/include
  ${CMAKE_SOURCE_DIR}/common/include
  ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
)

# Librerías (gtest_main + common)
target_link_libraries(utaos PRIVATE
  gtest_main
  common
)

# Registro del test en CTest
add_test(NAME utaos COMMAND $<TARGET_FILE:utaos>)
