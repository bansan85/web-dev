cmake_minimum_required(VERSION 3.20)

project(clang_format_parser LANGUAGES CXX)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

execute_process(
  COMMAND gcc -print-file-name=include
  OUTPUT_VARIABLE GCC_INCLUDE
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_executable(clang_format_parser)
target_sources(
  clang_format_parser
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/clang-format-parser.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
# Ubuntu needs clangAST clangBasic clangFrontend clangTooling.

# Gentoo needs LLVM clang clang-cpp
set(CL_LIBRARIES
    LLVM
    clang
    clang-cpp
    clangAST
    clangBasic
    clangFrontend
    clangTooling)
foreach(lib ${CL_LIBRARIES})
  if(TARGET ${lib})
    target_link_libraries(clang_format_parser PRIVATE ${lib})
  endif()
endforeach()

target_include_directories(clang_format_parser PRIVATE ${LLVM_INCLUDE_DIRS})
if(ipo_supported)
  set_property(TARGET clang_format_parser PROPERTY INTERPROCEDURAL_OPTIMIZATION
                                                   TRUE)
endif()
set_property(TARGET clang_format_parser PROPERTY CXX_STANDARD 20)
target_compile_options(clang_format_parser PRIVATE "-fno-rtti")

add_custom_command(
  TARGET clang_format_parser
  POST_BUILD
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../llvm/clang/include/clang/Format/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}"
    "-OCPP" "${CMAKE_CURRENT_SOURCE_DIR}/../src/web-formatter-binding.cpp.inc")
