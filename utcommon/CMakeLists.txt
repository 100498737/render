set(CURRENT_DIR_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/test_vector.cpp"
)

add_unit_test_target(
  TARGET_NAME utcommon
  SOURCE_FILES
    test_vector.cpp
    test_camera.cpp
  LIBRARY_FILTER common
  COVERAGE_DIR coverage-common
  LIBRARY_TO_LINK common
)
