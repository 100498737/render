add_executable(utsoa test_image_soa.cpp)
target_include_directories(utsoa PRIVATE
  ${CMAKE_SOURCE_DIR}/soa/include
  ${CMAKE_SOURCE_DIR}/common/include
)
target_link_libraries(utsoa PRIVATE GTest::gtest_main)
add_test(NAME utsoa COMMAND $<TARGET_FILE:utsoa>)
