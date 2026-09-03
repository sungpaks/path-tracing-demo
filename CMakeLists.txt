cmake_minimum_required(VERSION 3.20)

project(path_tracing LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/examples/*/main.cc"
)

if(NOT EXAMPLE_SOURCES)
  message(FATAL_ERROR "No examples found. Expected examples/<name>/main.cc")
endif()

foreach(EXAMPLE_SOURCE IN LISTS EXAMPLE_SOURCES)
  get_filename_component(EXAMPLE_DIR "${EXAMPLE_SOURCE}" DIRECTORY)
  get_filename_component(EXAMPLE_NAME "${EXAMPLE_DIR}" NAME)

  if(NOT EXAMPLE_NAME MATCHES "^[A-Za-z0-9_]+$")
    message(FATAL_ERROR
      "Example directory '${EXAMPLE_NAME}' must contain only letters, numbers, and underscores"
    )
  endif()

  add_executable("${EXAMPLE_NAME}" "${EXAMPLE_SOURCE}")
  target_compile_features("${EXAMPLE_NAME}" PRIVATE cxx_std_11)
  target_include_directories("${EXAMPLE_NAME}" PRIVATE "${EXAMPLE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/common")

  if(CMAKE_CXX_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
    target_compile_options("${EXAMPLE_NAME}" PRIVATE -Wall -Wextra -Wpedantic)
  endif()

  set_target_properties("${EXAMPLE_NAME}" PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
endforeach()
