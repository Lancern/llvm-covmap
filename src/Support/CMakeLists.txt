add_library(LLVMCovmapSupport STATIC
        "${LLVM_COVMAP_INCLUDE_DIR}/llvm-covmap/Support/SharedMemory.h"
        SharedMemory.cpp)
target_link_libraries(LLVMCovmapSupport
        PRIVATE "-lrt")
