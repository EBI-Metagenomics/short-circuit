add_executable(server server.c)
target_link_libraries(server PRIVATE SC::sc)
target_link_libraries(sc PRIVATE uv_a)
target_compile_features(server PRIVATE c_std_11)
target_compile_options(server PRIVATE ${WARNING_FLAGS})

file(COPY test_server_pipe DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME server_pipe COMMAND test_server_pipe)

file(COPY test_server_tcp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME server_tcp COMMAND test_server_tcp)

add_executable(client client.c)
target_link_libraries(client PRIVATE SC::sc)
target_link_libraries(sc PRIVATE uv_a)
target_compile_features(client PRIVATE c_std_11)
target_compile_options(client PRIVATE ${WARNING_FLAGS})

file(COPY test_client_tcp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME client_tcp COMMAND test_client_tcp)

add_subdirectory(tools)
