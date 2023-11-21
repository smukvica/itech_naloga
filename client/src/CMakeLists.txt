# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
# file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")
set(HEADER_LIST "${itech_naloga_SOURCE_DIR}/includes/c_file.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_gui.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_includes.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_output.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_queue.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_receiver.h"
                "${itech_naloga_SOURCE_DIR}/includes/raygui.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(client_library c_file.c 
                           c_gui.c 
                           c_includes.c 
                           c_output.c 
                           c_queue.c 
                           c_receiver.c 
                           ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(client_library PRIVATE ../includes)

# This depends on (header only) boost
target_link_libraries(client_library PRIVATE -lraylib -lpthread -lm -fopenmp)

# All users of this library will need at least C++11
target_compile_features(client_library PUBLIC cxx_std_11)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})

