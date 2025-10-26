add_executable(utaos test_image_aos.cpp)
target_include_directories(utaos PRIVATE
  ${CMAKE_SOURCE_DIR}/aos/include
  ${CMAKE_SOURCE_DIR}/common/include
)
target_link_libraries(utaos PRIVATE GTest::gtest_main)
add_test(NAME utaos COMMAND $<TARGET_FILE:utaos>)
