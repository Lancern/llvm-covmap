set(LLVM_COVMAP_BINARY_DIR "${CMAKE_BINARY_DIR}")
configure_file(Configure.h.in Configure.h)

add_executable(LLVMCovmapClang
        LLVMCovmapClang.cpp)
target_include_directories(LLVMCovmapClang
        PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(LLVMCovmapClang
        PROPERTIES OUTPUT_NAME "llvm-covmap-clang")

add_executable(LLVMCovmapClangPlusplus
        LLVMCovmapClang.cpp)
target_include_directories(LLVMCovmapClangPlusplus
        PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(LLVMCovmapClangPlusplus
        PROPERTIES OUTPUT_NAME "llvm-covmap-clang++")
target_compile_definitions(LLVMCovmapClangPlusplus
        PRIVATE "LLVM_COVMAP_CLANG_PLUSPLUS")
