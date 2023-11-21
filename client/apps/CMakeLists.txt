add_executable(client client.c)
target_compile_features(client PRIVATE cxx_std_17)

target_include_directories(client PRIVATE
    ../includes
)

target_link_libraries(client PRIVATE client_library raylib)