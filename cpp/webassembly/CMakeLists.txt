cmake_minimum_required(VERSION 3.20)

project(web_demangler LANGUAGES CXX)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  execute_process(
    COMMAND "${CMAKE_CXX_COMPILER}" "--cflags"
    OUTPUT_VARIABLE EMCC_FLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(REPLACE "-enable-emscripten-sjlj" "" EMCC_FLAGS ${EMCC_FLAGS})
  separate_arguments(EMCC_FLAGS)
endif()

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

if(WITH_SANITIZE_UNDEFINED)
  set(LLVM_ENABLE_RTTI ON)
endif()
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

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)

add_subdirectory(../third_party/llvm/llvm llvm_build EXCLUDE_FROM_ALL)

add_executable(web_demangler)
target_sources(
  web_demangler
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/web-demangler.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/demangler.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/demangler.h"
          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(web_demangler PRIVATE embind LLVMDemangle)
target_include_directories(web_demangler PRIVATE SYSTEM
                                                 ${LLVM_SOURCE_DIR}/include)
target_compile_options(web_demangler PRIVATE "-fexceptions")
target_link_options(
  web_demangler
  PRIVATE
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s EXPORT_ES6=1"
  "SHELL:-s EXPORT_NAME=web_demangler"
  "SHELL:-s ENVIRONMENT=web"
  "SHELL:-s INVOKE_RUN=0"
  "SHELL:-s MALLOC='emmalloc'"
  "SHELL:-s USE_ES6_IMPORT_META=0"
  "$<IF:$<CONFIG:Debug>,--closure 0,--closure 1>"
  "$<IF:$<CONFIG:Debug>,-s ASSERTIONS=1,-s ASSERTIONS=0>"
  "$<$<CONFIG:Debug>:-s SAFE_HEAP=1>"
  "$<$<CONFIG:Debug>:-s STACK_OVERFLOW_CHECK=2>"
  "$<$<CONFIG:Debug>:--emit-symbol-map>"
  "$<$<CONFIG:Debug>:-gsource-map>"
  "$<$<CONFIG:Debug>:-gseparate-dwarf>"
  "--emit-tsd"
  "web_demangler.d.ts"
  "-fexceptions"
  --no-entry)
if(ipo_supported)
  set_property(TARGET web_demangler PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(web_formatter)
target_sources(
  web_formatter
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/web-formatter.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/formatter.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/formatter.h"
          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(web_formatter PRIVATE embind clangFormat)
target_include_directories(
  web_formatter
  PRIVATE SYSTEM
          ${LLVM_SOURCE_DIR}/include
          ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/include
          ${LLVM_BINARY_DIR}/include/
          ${LLVM_BINARY_DIR}/tools/clang/include
          "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/pfr/include")
target_compile_options(web_formatter PRIVATE "-fexceptions")
target_link_options(
  web_formatter
  PRIVATE
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s EXPORT_ES6=1"
  "SHELL:-s EXPORT_NAME=web_formatter"
  "SHELL:-s ENVIRONMENT=web"
  "SHELL:-s INVOKE_RUN=0"
  "SHELL:-s MALLOC='emmalloc'"
  "SHELL:-s ALLOW_MEMORY_GROWTH=1"
  "SHELL:-s USE_ES6_IMPORT_META=0"
  "$<IF:$<CONFIG:Debug>,--closure 0,--closure 1>"
  "$<IF:$<CONFIG:Debug>,-s ASSERTIONS=1,-s ASSERTIONS=0>"
  "$<$<CONFIG:Debug>:-s SAFE_HEAP=1>"
  "$<$<CONFIG:Debug>:-s STACK_OVERFLOW_CHECK=2>"
  "$<$<CONFIG:Debug>:--emit-symbol-map>"
  "$<$<CONFIG:Debug>:-gsource-map>"
  "$<$<CONFIG:Debug>:-gseparate-dwarf>"
  "--emit-tsd"
  "web_formatter.d.ts"
  "-fexceptions"
  --no-entry)
if(ipo_supported)
  set_property(TARGET web_formatter PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(web_lighten)
target_sources(
  web_lighten
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/web-lighten.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/lighten.cpp"
          "${CMAKE_CURRENT_SOURCE_DIR}/../native/lighten.h"
          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(web_lighten PRIVATE embind)
target_link_options(
  web_lighten
  PRIVATE
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s EXPORT_ES6=1"
  "SHELL:-s EXPORT_NAME=web_lighten"
  "SHELL:-s ENVIRONMENT=web"
  "SHELL:-s INVOKE_RUN=0"
  "SHELL:-s MALLOC='emmalloc'"
  "SHELL:-s ALLOW_MEMORY_GROWTH=1"
  "SHELL:-s USE_ES6_IMPORT_META=0"
  "$<IF:$<CONFIG:Debug>,--closure 0,--closure 1>"
  "$<IF:$<CONFIG:Debug>,-s ASSERTIONS=1,-s ASSERTIONS=0>"
  "$<$<CONFIG:Debug>:-s SAFE_HEAP=1>"
  "$<$<CONFIG:Debug>:-s STACK_OVERFLOW_CHECK=2>"
  "$<$<CONFIG:Debug>:--emit-symbol-map>"
  "$<$<CONFIG:Debug>:-gsource-map>"
  "$<$<CONFIG:Debug>:-gseparate-dwarf>"
  "--emit-tsd"
  "web_lighten.d.ts"
  "--no-entry")
if(ipo_supported)
  set_property(TARGET web_lighten PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(WITH_SANITIZE_ADDRESS)
  foreach(target_i IN ITEMS web_demangler web_formatter web_lighten)
    target_compile_options(${target_i} PRIVATE "-fsanitize=address"
                                               "-fno-omit-frame-pointer")
    target_link_options(${target_i} PRIVATE "-fsanitize=address")
  endforeach()
endif()
if(WITH_SANITIZE_UNDEFINED)
  foreach(target_i IN ITEMS web_demangler web_formatter web_lighten)
    target_compile_options(
      ${target_i} PRIVATE "-fsanitize=undefined" "-fno-sanitize-recover=all"
                          "-fno-omit-frame-pointer")
    target_link_options(${target_i} PRIVATE "-fsanitize=undefined")
  endforeach()
endif()
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  target_compile_options(web_demangler PRIVATE ${EMCC_FLAGS})
  target_compile_options(web_formatter PRIVATE ${EMCC_FLAGS})
  target_compile_options(web_lighten PRIVATE ${EMCC_FLAGS})
endif()
