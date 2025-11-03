cmake_minimum_required(VERSION 3.24)

# Ejecutable de tests SOA
add_executable(utsoa
  test_image_soa.cpp
)

# Includes (SOA + common + gtest)
target_include_directories(utsoa PRIVATE
  ${CMAKE_SOURCE_DIR}/soa/include
  ${CMAKE_SOURCE_DIR}/common/include
  ${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include
)

# Librerías (gtest_main + common)
target_link_libraries(utsoa PRIVATE
  gtest_main
  common
)

# Registro del test en CTest (no ponemos WORKING_DIRECTORY para evitar /Release)
add_test(NAME utsoa COMMAND $<TARGET_FILE:utsoa>)
