add_library(LLVMCovmap STATIC
        LLVMCoverageMap.c)
target_link_libraries(LLVMCovmap
        PRIVATE "-lrt")
