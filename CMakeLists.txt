cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

project(
  sc
  VERSION 0.0.3
  LANGUAGES C)

# Generate compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/compiler-options.cmake)
include(cmake/sanitizers.cmake)
include(cmake/CPM.cmake)

cpmaddpackage(NAME c_toolbelt VERSION 0.9.0 GITHUB_REPOSITORY horta/c-toolbelt)
cpmaddpackage(NAME libuv VERSION 1.44.2 GITHUB_REPOSITORY libuv/libuv)

add_library(
  sc
  src/backend.c
  src/backend_uv.c
  src/msg.c
  src/msg_reader.c
  src/sc.c
  src/socket.c
  src/uri.c
  src/warn.c
  src/watcher.c)
add_library(SC::sc ALIAS sc)

target_link_libraries(sc PUBLIC uv_a)
target_link_libraries(sc PUBLIC C_TOOLBELT::ctb)

set(EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/sc/export.h)
include(GenerateExportHeader)
generate_export_header(
  sc
  BASE_NAME
  SC
  INCLUDE_GUARD_NAME
  SC_EXPORT_H
  EXPORT_MACRO_NAME
  SC_API
  EXPORT_FILE_NAME
  ${EXPORT_FILE})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported()
  set_target_properties(sc PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

target_compile_features(sc PUBLIC c_std_11)
target_compile_options(sc PUBLIC ${WARNING_FLAGS})
set_target_properties(sc PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(
  sc
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

install(TARGETS sc EXPORT sc-targets)

install(DIRECTORY include/sc/ DESTINATION include/sc)

install(
  EXPORT sc-targets
  FILE sc-targets.cmake
  NAMESPACE SC::
  DESTINATION lib/cmake/sc)

install(FILES ${EXPORT_FILE} DESTINATION include/sc)

enable_testing()
add_subdirectory(test)

set(CPACK_PACKAGE_NAME sc)
set(CPACK_PACKAGE_VENDOR "Danilo Horta")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Short-circuit is a socket library for loop-based applications")
set(CPACK_PACKAGE_CONTACT "Danilo Horta")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/package)
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_PACKAGE_RELOCATABLE YES)
set(CPACK_MONOLITHIC_INSTALL YES)
include(CPack)
