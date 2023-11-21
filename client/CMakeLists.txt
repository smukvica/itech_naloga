cmake_minimum_required(VERSION 3.16.3)

project(itech_naloga)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)
endif()

include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
  GIT_TAG "5.0"   # it's much better to use a specific Git revision or Git tag for reproducibility
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/raylib"
)
FetchContent_MakeAvailable(raylib)


add_subdirectory(src)

add_subdirectory(apps)

add_subdirectory(tests)

