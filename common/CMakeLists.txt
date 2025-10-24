add_library(common STATIC)

target_sources(common
  PRIVATE
    src/camera.cpp      # tu cámara
    src/config.cpp      # este stub que acabamos de hacer
    # src/vector.cpp    # <-- SOLO si tienes un .cpp separado. 
                        # Si TODO Vec3 está inline en vector.hpp, NO pongas esto.
)

target_include_directories(common
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(common
  PUBLIC
    Microsoft.GSL::GSL
)
