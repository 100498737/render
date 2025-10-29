cmake_minimum_required(VERSION 3.20)
project(render_project LANGUAGES CXX)

# ---- Opciones ----
option(ENABLE_CLANG_TIDY "Enable clang-tidy checks" OFF)

# ---- Dependencias (GSL + GoogleTest) ----
include(FetchContent)

FetchContent_Declare(
  GSL
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG        v4.1.0
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
)

set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK            OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(GSL googletest)

# ---- Testing ----
enable_testing()
include(GoogleTest)
include(cmake/TestUtils.cmake)

# ---- clang-tidy (Avignon) ----
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy-20 clang-tidy)
  if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY
        "${CLANG_TIDY_EXE};--extra-arg=--gcc-toolchain=/opt/gcc-14;--header-filter=.*")
  else()
    message(STATUS "clang-tidy not found; skipping CMAKE_CXX_CLANG_TIDY")
  endif()
endif()

# ---- Subproyectos (cada uno define sus targets) ----
add_subdirectory(common)
add_subdirectory(aos)
add_subdirectory(soa)
add_subdirectory(utcommon)
add_subdirectory(utsoa)
add_subdirectory(utaos)


#########################################################


# --- clang-tidy (Avignon) para NUESTRO código ---
if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy-20 clang-tidy)
  if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY
      "${CLANG_TIDY_EXE};-quiet;--extra-arg=--gcc-toolchain=/opt/gcc-14;--header-filter=^(common|aos|soa)/")
  endif()
endif()

# --- FetchContent: desactivar tidy en deps externas ---
set(_OLD_TIDY "${CMAKE_CXX_CLANG_TIDY}")
set(CMAKE_CXX_CLANG_TIDY "")  # << evita tidy en GSL/googletest

include(FetchContent)
FetchContent_Declare(
  GSL
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG        v4.1.0
  GIT_SHALLOW    TRUE
)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(GSL googletest)

# Restaurar tidy para NUESTROS targets
set(CMAKE_CXX_CLANG_TIDY "${_OLD_TIDY}")
