set(SC_RUNTIME_DIR ${CMAKE_CURRENT_BINARY_DIR}/run)

file(MAKE_DIRECTORY ${SC_RUNTIME_DIR})

add_executable(server server.c)
target_link_libraries(server PRIVATE SC::sc)
target_link_libraries(sc PRIVATE uv_a)
target_compile_features(server PRIVATE c_std_11)
target_compile_options(server PRIVATE ${WARNING_FLAGS})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_server DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME server COMMAND test_server)

add_executable(generate_record generate_record.c)
target_compile_features(generate_record PRIVATE c_std_11)
target_compile_options(generate_record PRIVATE ${WARNING_FLAGS})
