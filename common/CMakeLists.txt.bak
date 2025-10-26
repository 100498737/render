add_library(common STATIC)

target_sources(common
  PRIVATE
    src/camera.cpp
    src/config.cpp
    src/parser.cpp
    src/scene.cpp
)

target_include_directories(common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common
  PUBLIC
    Microsoft.GSL::GSL
)

target_compile_features(common PUBLIC cxx_std_23)

target_compile_options(common
  PRIVATE
    -Wall -Wextra -Werror -pedantic -pedantic-errors
    -Wconversion -Wsign-conversion
)
