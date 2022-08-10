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

add_executable(send_hello send_hello.c)
target_link_libraries(send_hello PRIVATE SC::sc)
target_link_libraries(sc PRIVATE uv_a)
target_compile_features(send_hello PRIVATE c_std_11)
target_compile_options(send_hello PRIVATE ${WARNING_FLAGS})
add_test(NAME send_hello COMMAND send_hello)

add_executable(echo_server echo_server.c)
target_link_libraries(echo_server PRIVATE SC::sc)
target_link_libraries(sc PRIVATE uv_a)
target_compile_features(echo_server PRIVATE c_std_11)
target_compile_options(echo_server PRIVATE ${WARNING_FLAGS})

file(COPY test_echo_server DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_test(NAME echo_server COMMAND test_echo_server)

add_subdirectory(tools)
