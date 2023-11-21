cmake_minimum_required(VERSION 3.16.3)

project(itech_naloga)

add_executable(server server.c)

target_link_libraries(server PRIVATE -fopenmp -lm)
