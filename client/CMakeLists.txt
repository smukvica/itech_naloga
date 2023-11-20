cmake_minimum_required(VERSION 3.16.3)

project(itech_naloga)

add_executable (
    client
    src/c_file.c
    src/c_gui.c
    src/c_includes.c
    src/client.c
    src/c_output.c
    src/c_queue.c
    src/c_receiver.c
)

target_include_directories(client PRIVATE
    includes
)

target_link_libraries(client -lraylib -lpthread -lm -fopenmp)
