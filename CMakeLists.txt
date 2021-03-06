cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(llvm-covmap)

set(CMAKE_CXX_STANDARD 14)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in ${LLVM_DIR}")

set(LLVM_COVMAP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${LLVM_COVMAP_INCLUDE_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

function(target_link_llvm name)
    target_include_directories("${name}"
            PUBLIC ${LLVM_INCLUDE_DIRS})
    target_compile_definitions("${name}"
            PUBLIC ${LLVM_DEFINITIONS})
endfunction()

add_subdirectory(third_party)
add_subdirectory(src)

add_custom_target(BuildUtilityLinks ALL "${CMAKE_CURRENT_SOURCE_DIR}/scripts/SetupLinks.sh"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
