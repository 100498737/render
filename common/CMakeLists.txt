add_library(common STATIC)

# fuentes que pertenecen a la librería common
target_sources(common
    PRIVATE
        src/vector.cpp
        src/camera.cpp
)

# directorios de cabeceras públicos
target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# dependencias públicas (la plantilla ya trae GSL)
target_link_libraries(common
    PUBLIC
        Microsoft.GSL::GSL
)
