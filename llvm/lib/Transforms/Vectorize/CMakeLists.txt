add_llvm_component_library(LLVMVectorize
  LoadStoreVectorizer.cpp
  LoopVectorizationLegality.cpp
  LoopVectorize.cpp
  SLPVectorizer.cpp
  Vectorize.cpp
  VectorCombine.cpp
  VPlan.cpp
  VPlanHCFGBuilder.cpp
  VPlanRecipes.cpp
  VPlanSLP.cpp
  VPlanTransforms.cpp
  VPlanVerifier.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms/Vectorize

  DEPENDS
  intrinsics_gen

  LINK_COMPONENTS
  Analysis
  AutoTuner
  Core
  Support
  TransformUtils
  )
