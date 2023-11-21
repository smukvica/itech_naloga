set(HEADER_LIST "${itech_naloga_SOURCE_DIR}/includes/c_file.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_gui.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_includes.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_output.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_queue.h"
                "${itech_naloga_SOURCE_DIR}/includes/c_receiver.h"
                "${itech_naloga_SOURCE_DIR}/includes/raygui.h")

add_library(client_library c_file.c 
                           c_gui.c 
                           c_includes.c 
                           c_output.c 
                           c_queue.c 
                           c_receiver.c 
                           ${HEADER_LIST})

target_include_directories(client_library PRIVATE ../includes)

target_link_libraries(client_library PRIVATE raylib -lpthread -lm -fopenmp)

target_compile_features(client_library PUBLIC cxx_std_11)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})

