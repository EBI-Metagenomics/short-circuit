find_program(SC_NC nc REQUIRED)
find_program(SC_LSOF lsof REQUIRED)

add_executable(genrec genrec.c)
target_compile_features(genrec PRIVATE c_std_11)
target_compile_options(genrec PRIVATE ${WARNING_FLAGS})

add_executable(disrec disrec.c)
target_compile_features(disrec PRIVATE c_std_11)
target_compile_options(disrec PRIVATE ${WARNING_FLAGS})

file(COPY runtime_dir DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY mktemp_dir DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY sendrec DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY recvrec DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY waitsock DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY waitport DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY waitproc DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY termproc DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
