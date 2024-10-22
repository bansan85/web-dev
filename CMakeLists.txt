cmake_minimum_required(VERSION 3.20)

project(web_demangler LANGUAGES CXX)

set(LLVM_TARGETS_TO_BUILD "X86")
set(LLVM_INCLUDE_TESTS OFF)
set(LLVM_BUILD_TESTS OFF)
set(LLVM_INCLUDE_EXAMPLES OFF)
set(LLVM_BUILD_EXAMPLES OFF)
set(LLVM_INCLUDE_TOOLS OFF)
set(LLVM_BUILD_TOOLS OFF)
set(LLVM_INCLUDE_BENCHMARKS OFF)
set(LLVM_BUILD_BENCHMARKS OFF)
set(LLVM_INCLUDE_UTILS OFF)
set(LLVM_BUILD_UTILS OFF)
set(LLVM_ENABLE_THREADS OFF)
set(LLVM_INDIVIDUAL_TEST_COVERAGE OFF)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

add_subdirectory(llvm/llvm EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME})

target_sources(
  ${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/web-demangler.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(${PROJECT_NAME} PRIVATE embind LLVMDemangle)
target_include_directories(${PROJECT_NAME} PRIVATE ${LLVM_SOURCE_DIR}/include)
target_link_options(
  ${PROJECT_NAME}
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
set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
