add_library(LLVMCovmap STATIC
        LLVMCoverageMap.c)
target_compile_options(LLVMCovmap
        PUBLIC "-fPIC")
target_link_libraries(LLVMCovmap
        PRIVATE "-lrt")
