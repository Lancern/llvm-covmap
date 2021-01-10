set(LLVM_COVMAP_BINARY_DIR "${CMAKE_BINARY_DIR}")
configure_file(Configure.h.in Configure.h)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_executable(LLVMCovmapClang
        LLVMCovmapClang.cpp
        Utils.hpp)
set_target_properties(LLVMCovmapClang
        PROPERTIES OUTPUT_NAME "llvm-covmap-clang")

add_executable(LLVMCovmapClangPlusplus
        LLVMCovmapClang.cpp
        Utils.hpp)
set_target_properties(LLVMCovmapClangPlusplus
        PROPERTIES OUTPUT_NAME "llvm-covmap-clang++")
target_compile_definitions(LLVMCovmapClangPlusplus
        PRIVATE "LLVM_COVMAP_CLANG_PLUSPLUS")

add_executable(LLVMCovmapLLD
        LLVMCovmapLLD.cpp
        Utils.hpp)
set_target_properties(LLVMCovmapLLD
        PROPERTIES OUTPUT_NAME "llvm-covmap-lld")
