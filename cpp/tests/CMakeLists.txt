cmake_minimum_required(VERSION 3.20)

project(web_dev LANGUAGES CXX)

include(CTest)
enable_testing()

option(WITH_SANITIZE_ADDRESS "Build with address sanitizer." OFF)
option(WITH_SANITIZE_UNDEFINED "Build with undefined sanitizer." OFF)

set(LLVM_TARGETS_TO_BUILD
    ""
    CACHE STRING "Semicolon-separated list of targets to build, or \"all\".")
option(LLVM_INCLUDE_TESTS "Generate build targets for the LLVM unit tests." OFF)
option(LLVM_INCLUDE_EXAMPLES "Generate build targets for the LLVM examples" OFF)
option(LLVM_INCLUDE_BENCHMARKS
       "Generate benchmark targets. If OFF, benchmarks can't be built." OFF)
option(LLVM_INCLUDE_UTILS "Generate build targets for the LLVM utils." OFF)
option(LLVM_BUILD_UTILS
       "Build LLVM utility binaries. If OFF, just generate build targets." OFF)
# For clang::format
set(LLVM_ENABLE_PROJECTS
    "clang"
    CACHE
      STRING
      "Semicolon-separated list of projects to build (${LLVM_KNOWN_PROJECTS}), or \"all\"."
)

set(LLVM_ENABLE_RTTI ON)
set(LIST_SANITIZER "")
if(WITH_SANITIZE_ADDRESS)
  set(LIST_SANITIZER ${LIST_SANITIZER} "Address")
endif()
if(WITH_SANITIZE_UNDEFINED)
  set(LIST_SANITIZER ${LIST_SANITIZER} "Undefined")
endif()
set(LLVM_USE_SANITIZER
    "${LIST_SANITIZER}"
    CACHE STRING "Define the sanitizer used to build binaries and tests.")

add_subdirectory(../third_party/llvm/llvm llvm_build EXCLUDE_FROM_ALL)
add_subdirectory(../third_party/catch2 catch2_build EXCLUDE_FROM_ALL)

include(Catch)

add_executable(test_lighten_number)
target_sources(
  test_lighten_number
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lighten-number.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/lighten.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/lighten.h"
          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")

add_executable(test_clang_format_config_migrate)
target_sources(
  test_clang_format_config_migrate
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/clang-format-config-migrate.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.3.0/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.3.0/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.4.2/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.4.2/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.5.2/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.5.2/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.6.2/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.6.2/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.7.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.7.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.8.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.8.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.9.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.9.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/4.0.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/4.0.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/5.0.2/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/5.0.2/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/6.0.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/6.0.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/7.1.0/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/7.1.0/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/7.1.0/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/7.1.0/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/8.0.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/8.0.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/8.0.1/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/8.0.1/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/9.0.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/9.0.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/9.0.1/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/9.0.1/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/10.0.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/10.0.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/10.0.1/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/10.0.1/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/11.1.0/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/11.1.0/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/11.1.0/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/11.1.0/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/12.0.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/12.0.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/12.0.1/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/12.0.1/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/13.0.1/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/13.0.1/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/13.0.1/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/13.0.1/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/14.0.6/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/14.0.6/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/14.0.6/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/14.0.6/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/15.0.7/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/15.0.7/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/15.0.7/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/15.0.7/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/16.0.6/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/16.0.6/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/16.0.6/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/16.0.6/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/17.0.6/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/17.0.6/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/17.0.6/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/17.0.6/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/18.1.8/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/18.1.8/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/18.1.8/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/18.1.8/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/19.1.6/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/19.1.6/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/19.1.6/IncludeStyle.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/19.1.6/IncludeStyle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/Format.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/Format.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/update.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/update.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(test_clang_format_config_migrate
                      PRIVATE clangBasic Catch2::Catch2WithMain)
target_include_directories(
  test_clang_format_config_migrate
  PRIVATE SYSTEM
          ${LLVM_SOURCE_DIR}/include
          ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/include
          ${LLVM_BINARY_DIR}/tools/clang/include
          "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/pfr/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/frozen/include"
          "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/magic_enum/include"
          ${LLVM_BINARY_DIR}/include)
add_custom_command(
  TARGET test_clang_format_config_migrate
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/data/config-file
    $<TARGET_FILE_DIR:test_clang_format_config_migrate>)

catch_discover_tests(test_clang_format_config_migrate)

if(WITH_SANITIZE_ADDRESS)
  foreach(target_i IN ITEMS test_lighten_number
                            test_clang_format_config_migrate)
    target_compile_options(${target_i} PRIVATE "-fsanitize=address"
                                               "-fno-omit-frame-pointer")
    target_link_options(${target_i} PRIVATE "-fsanitize=address")
  endforeach()
endif()
if(WITH_SANITIZE_UNDEFINED)
  foreach(target_i IN ITEMS test_lighten_number
                            test_clang_format_config_migrate)
    target_compile_options(
      ${target_i} PRIVATE "-fsanitize=undefined" "-fno-sanitize-recover=all"
                          "-fno-omit-frame-pointer")
    target_link_options(${target_i} PRIVATE "-fsanitize=undefined")
  endforeach()
endif()
