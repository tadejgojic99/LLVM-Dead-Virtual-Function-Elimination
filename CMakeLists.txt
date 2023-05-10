add_llvm_library( LLVMDeadVirtual MODULE
    deadVirtualRemoval.cpp
    
    PLUGIN_TOOL
    opt
)
