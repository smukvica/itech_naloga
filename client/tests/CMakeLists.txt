cmake_minimum_required(VERSION 3.14)
project(my_project)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  hello_test
  tests.cc
  #../src/c_file.c
  #../src/c_gui.c
  #../src/c_includes.c
  #../src/client.c
  #../src/c_output.c
  #../src/c_queue.c
  #../src/c_receiver.c
)

target_include_directories(hello_test PRIVATE
    ../includes
)

target_link_libraries(
  hello_test
  GTest::gtest_main
  -lraylib
  -lpthread 
  -lm 
  -fopenmp
)

include(GoogleTest)
gtest_discover_tests(hello_test)