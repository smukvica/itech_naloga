cmake_minimum_required(VERSION 3.14)
project(itech_naloga)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  hello_test
  tests.cc
)

target_include_directories(hello_test PRIVATE
    ../includes
)

target_link_libraries(
  hello_test
  GTest::gtest_main
  client_library
)

include(GoogleTest)
gtest_discover_tests(hello_test)