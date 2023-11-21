cmake_minimum_required(VERSION 3.16.3)

project(itech_naloga)

set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)

include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
  GIT_TAG "5.0"
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/raylib"
)
FetchContent_MakeAvailable(raylib)


add_subdirectory(src)

add_subdirectory(apps)

add_subdirectory(tests)

