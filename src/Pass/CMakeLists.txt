add_library(LLVMCoverageMapPass MODULE
        LLVMCoverageMapPass.cpp)
target_link_llvm(LLVMCoverageMapPass)
