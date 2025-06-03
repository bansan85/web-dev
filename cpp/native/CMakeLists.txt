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

target_include_directories(
  clang_format_parser
  PRIVATE SYSTEM ${LLVM_INCLUDE_DIRS}
          "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/frozen/include")
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
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/llvm/clang/include/clang/Format/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-formatter-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.3.0/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.3.0"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v3.3.0-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.4.2/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.4.2"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v3.4.2-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.5.2/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.5.2"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v3.5.2-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.6.2/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.6.2"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v3.6.2-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.7.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.7.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v3.7.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.8.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.8.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v3.8.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.9.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/3.9.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v3.9.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/4.0.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/4.0.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v4.0.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/5.0.2/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/5.0.2"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v5.0.2-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/6.0.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/6.0.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v6.0.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/7.1.0/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/7.1.0"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v7.1.0-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/8.0.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/8.0.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v8.0.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/9.0.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/9.0.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v9.0.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/10.0.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/10.0.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v10.0.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/11.1.0/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/11.1.0"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v11.1.0-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/12.0.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/12.0.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v12.0.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/13.0.1/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/13.0.1"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v13.0.1-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/14.0.6/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/14.0.6"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v14.0.6-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/15.0.7/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/15.0.7"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v15.0.7-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/16.0.6/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/16.0.6"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v16.0.6-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/17.0.6/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/17.0.6"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v17.0.6-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/18.1.8/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/18.1.8"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v18.1.8-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/19.1.6/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/19.1.6"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v19.1.6-binding.cpp.inc"
  COMMAND
    $<TARGET_FILE:clang_format_parser>
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/20.1.6/Format.h"
    "-I" "${LLVM_INCLUDE_DIRS}" "-I" "${GCC_INCLUDE}" "-I"
    "${CMAKE_CURRENT_SOURCE_DIR}/../native/clang-format-config-migrate/20.1.6"
    "-OCPP"
    "${CMAKE_CURRENT_SOURCE_DIR}/../webassembly/web-clang-format-config-migrate-v20.1.6-binding.cpp.inc"
)
