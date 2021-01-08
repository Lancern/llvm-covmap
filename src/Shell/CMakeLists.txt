add_executable(LLVMCoverageMapShell
        LLVMCoverageMapShell.cpp)
target_link_libraries(LLVMCoverageMapShell
        PRIVATE cxxopts "-lrt")
set_target_properties(LLVMCoverageMapShell
        PROPERTIES OUTPUT_NAME "llvm-covmap-shell")
