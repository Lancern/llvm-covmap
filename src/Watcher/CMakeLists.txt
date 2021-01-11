add_executable(LLVMCovmapWatcher
        LLVMCovmapWatcher.cpp)
target_link_libraries(LLVMCovmapWatcher
        PRIVATE cxxopts "-lrt" LLVMCovmapSupport)
set_target_properties(LLVMCovmapWatcher
        PROPERTIES OUTPUT_NAME "llvm-covmap-watcher")
