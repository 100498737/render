add_library(common STATIC)

target_sources(common
    PRIVATE
        src/vector.cpp
        src/camera.cpp
        # src/config.cpp      # <- si tu compi ya tiene parser config.cpp, descomenta
        # src/scene.cpp       # <- si ya existe scene.cpp, descomenta
)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common
    PUBLIC
        Microsoft.GSL::GSL
)
