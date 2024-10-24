cmake_minimum_required(VERSION 3.20)

project(web_demangler LANGUAGES CXX)

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

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

add_subdirectory(llvm/llvm EXCLUDE_FROM_ALL)

add_executable(web_demangler)
target_sources(
  web_demangler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/web-demangler.cpp"
                        "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(web_demangler PRIVATE embind LLVMDemangle)
target_include_directories(web_demangler PRIVATE ${LLVM_SOURCE_DIR}/include)
target_link_options(
  web_demangler
  PRIVATE
  "SHELL:-s MODULARIZE=1"
  "SHELL:-s EXPORT_ES6=1"
  "SHELL:-s EXPORT_NAME=web_demangler"
  "SHELL:-s ENVIRONMENT=web"
  "SHELL:-s INVOKE_RUN=0"
  "SHELL:-s MALLOC='emmalloc'"
  "$<IF:$<CONFIG:Debug>,--closure 0,--closure 1>"
  "$<IF:$<CONFIG:Debug>,-s ASSERTIONS=1,-s ASSERTIONS=0>"
  "$<$<CONFIG:Debug>:-s SAFE_HEAP=1>"
  "$<$<CONFIG:Debug>:-s STACK_OVERFLOW_CHECK=2>"
  "$<$<CONFIG:Debug>:--emit-symbol-map>"
  "$<$<CONFIG:Debug>:-gsource-map>"
  "$<$<CONFIG:Debug>:-gseparate-dwarf>"
  --no-entry)
set_property(TARGET web_demangler PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_executable(web_formatter)
target_sources(
  web_formatter PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/web-formatter.cpp"
                        "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(web_formatter PRIVATE embind clangFormat)
target_include_directories(
  web_formatter
  PRIVATE ${LLVM_SOURCE_DIR}/include ${LLVM_EXTERNAL_CLANG_SOURCE_DIR}/include
          ${LLVM_BINARY_DIR}/include/ ${LLVM_BINARY_DIR}/tools/clang/include)
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
  "$<IF:$<CONFIG:Debug>,--closure 0,--closure 1>"
  "$<IF:$<CONFIG:Debug>,-s ASSERTIONS=1,-s ASSERTIONS=0>"
  "$<$<CONFIG:Debug>:-s SAFE_HEAP=1>"
  "$<$<CONFIG:Debug>:-s STACK_OVERFLOW_CHECK=2>"
  "$<$<CONFIG:Debug>:--emit-symbol-map>"
  "$<$<CONFIG:Debug>:-gsource-map>"
  "$<$<CONFIG:Debug>:-gseparate-dwarf>"
  --no-entry)
set_property(TARGET web_formatter PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
