cmake_minimum_required(VERSION 3.20)

project(web_dev LANGUAGES CXX)

option(WITH_SANITIZE_ADDRESS "Build with address sanitizer." OFF)
option(WITH_SANITIZE_UNDEFINED "Build with undefined sanitizer." OFF)

add_executable(test_lighten_number)
target_sources(
  test_lighten_number
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lighten-number.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/lighten.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/lighten.h"
          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")

if(WITH_SANITIZE_ADDRESS)
  foreach(target_i IN ITEMS test_lighten_number)
    target_compile_options(${target_i} PRIVATE "-fsanitize=address"
                                               "-fno-omit-frame-pointer")
    target_link_options(${target_i} PRIVATE "-fsanitize=address")
  endforeach()
endif()
if(WITH_SANITIZE_UNDEFINED)
  foreach(target_i IN ITEMS test_lighten_number)
    target_compile_options(
      ${target_i} PRIVATE "-fsanitize=undefined" "-fno-sanitize-recover=all"
                          "-fno-omit-frame-pointer")
    target_link_options(${target_i} PRIVATE "-fsanitize=undefined")
  endforeach()
endif()
