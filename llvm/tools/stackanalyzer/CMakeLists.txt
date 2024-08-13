set(LLVM_LINK_COMPONENTS
  Analysis
  BitReader
  Core
  Passes
  Support
)

add_llvm_tool(stackanalyzer
  CallGraphGen.cpp
  stackanalyzer.cpp   
  StackUsage.cpp
)