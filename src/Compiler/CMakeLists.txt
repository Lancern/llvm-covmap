add_executable(LLVMCovmapClang
        LLVMCovmapClang.cpp)
set_target_properties(LLVMCovmapClang
        PROPERTIES OUTPUT_NAME "llvm-covmap-clang")

add_executable(LLVMCovmapClangPlusplus
        LLVMCovmapClang.cpp)
set_target_properties(LLVMCovmapClangPlusplus
        PROPERTIES OUTPUT_NAME "llvm-covmap-clang++")
target_compile_definitions(LLVMCovmapClangPlusplus
        PRIVATE "LLVM_COVMAP_CLANG_PLUSPLUS")
