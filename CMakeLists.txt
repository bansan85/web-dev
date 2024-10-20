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

add_subdirectory(llvm/llvm EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME})

target_sources(
  ${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/web-demangler.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
target_link_libraries(${PROJECT_NAME} PRIVATE embind LLVMDemangle)
target_include_directories(${PROJECT_NAME} PRIVATE ${LLVM_SOURCE_DIR}/include)
target_link_options( ${PROJECT_NAME} PRIVATE "SHELL:-s MODULARIZE" "SHELL:-s EXPORT_ES6=1" "SHELL:-s EXPORT_NAME=web_demangler" "SHELL:-s ENVIRONMENT=web,worker" --no-entry)
