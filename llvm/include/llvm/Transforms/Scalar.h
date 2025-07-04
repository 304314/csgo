//===-- Scalar.h - Scalar Transformations -----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This header file defines prototypes for accessor functions that expose passes
// in the Scalar transformations library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_H
#define LLVM_TRANSFORMS_SCALAR_H

#include "llvm/Transforms/Utils/SimplifyCFGOptions.h"
#include <functional>
#if defined(ENABLE_AUTOTUNER)
#include "llvm/Pass.h"
#include <string>
#endif

namespace llvm {

class Function;
class FunctionPass;
class Pass;

//===----------------------------------------------------------------------===//
//
// RedundantDbgInstElimination - This pass removes redundant dbg intrinsics
// without modifying the CFG of the function.  It is a FunctionPass.
//
Pass *createRedundantDbgInstEliminationPass();

//===----------------------------------------------------------------------===//
//
// DeadCodeElimination - This pass is more powerful than DeadInstElimination,
// because it is worklist driven that can potentially revisit instructions when
// their other instructions become dead, to eliminate chains of dead
// computations.
//
FunctionPass *createDeadCodeEliminationPass();


//===----------------------------------------------------------------------===//
//
// GuardWidening - An optimization over the @llvm.experimental.guard intrinsic
// that (optimistically) combines multiple guards into one to have fewer checks
// at runtime.
//
FunctionPass *createGuardWideningPass();


//===----------------------------------------------------------------------===//
//
// LoopGuardWidening - Analogous to the GuardWidening pass, but restricted to a
// single loop at a time for use within a LoopPassManager.  Desired effect is
// to widen guards into preheader or a single guard within loop if that's not
// possible.
//
Pass *createLoopGuardWideningPass();


//===----------------------------------------------------------------------===//
//
// SROA - Replace aggregates or pieces of aggregates with scalar SSA values.
//
FunctionPass *createSROAPass(bool PreserveCFG = true);

//===----------------------------------------------------------------------===//
//
// LICM - This pass is a loop invariant code motion and memory promotion pass.
//
Pass *createLICMPass();

//===----------------------------------------------------------------------===//
//
// LoopSink - This pass sinks invariants from preheader to loop body where
// frequency is lower than loop preheader.
//
Pass *createLoopSinkPass();

//===----------------------------------------------------------------------===//
//
// LoopPredication - This pass does loop predication on guards.
//
Pass *createLoopPredicationPass();

//===----------------------------------------------------------------------===//
//
// LoopStrengthReduce - This pass is strength reduces GEP instructions that use
// a loop's canonical induction variable as one of their indices.
//
Pass *createLoopStrengthReducePass();

//===----------------------------------------------------------------------===//
//
// LoopInstSimplify - This pass simplifies instructions in a loop's body.
//
Pass *createLoopInstSimplifyPass();

//===----------------------------------------------------------------------===//
//
// LoopUnroll - This pass is a simple loop unrolling pass.
//
Pass *createLoopUnrollPass(int OptLevel = 2, bool OnlyWhenForced = false,
                           bool ForgetAllSCEV = false, int Threshold = -1,
                           int Count = -1, int AllowPartial = -1,
                           int Runtime = -1, int UpperBound = -1,
                           int AllowPeeling = -1);

//===----------------------------------------------------------------------===//
//
// LoopRotate - This pass is a simple loop rotating pass.
//
Pass *createLoopRotatePass(int MaxHeaderSize = -1, bool PrepareForLTO = false);

//===----------------------------------------------------------------------===//
//
// LoopVersioningLICM - This pass is a loop versioning pass.
//
Pass *createLoopVersioningLICMPass();

//===----------------------------------------------------------------------===//
//
// DemoteRegisterToMemoryPass - This pass is used to demote registers to memory
// references. In basically undoes the PromoteMemoryToRegister pass to make cfg
// hacking easier.
//
FunctionPass *createDemoteRegisterToMemoryPass();
extern char &DemoteRegisterToMemoryID;

//===----------------------------------------------------------------------===//
//
// Reassociate - This pass reassociates commutative expressions in an order that
// is designed to promote better constant propagation, GCSE, LICM, PRE...
//
// For example:  4 + (x + 5)  ->  x + (4 + 5)
//
FunctionPass *createReassociatePass();

//===----------------------------------------------------------------------===//
//
// CFGSimplification - Merge basic blocks, eliminate unreachable blocks,
// simplify terminator instructions, convert switches to lookup tables, etc.
//
FunctionPass *createCFGSimplificationPass(
    SimplifyCFGOptions Options = SimplifyCFGOptions(),
    std::function<bool(const Function &)> Ftor = nullptr);

//===----------------------------------------------------------------------===//
//
// FlattenCFG - flatten CFG, reduce number of conditional branches by using
// parallel-and and parallel-or mode, etc...
//
FunctionPass *createFlattenCFGPass();

//===----------------------------------------------------------------------===//
//
// CFG Structurization - Remove irreducible control flow
//
///
/// When \p SkipUniformRegions is true the structizer will not structurize
/// regions that only contain uniform branches.
Pass *createStructurizeCFGPass(bool SkipUniformRegions = false);

//===----------------------------------------------------------------------===//
//
// TailCallElimination - This pass eliminates call instructions to the current
// function which occur immediately before return instructions.
//
FunctionPass *createTailCallEliminationPass();

//===----------------------------------------------------------------------===//
//
// EarlyCSE - This pass performs a simple and fast CSE pass over the dominator
// tree.
//
FunctionPass *createEarlyCSEPass(bool UseMemorySSA = false);

//===----------------------------------------------------------------------===//
//
// MergedLoadStoreMotion - This pass merges loads and stores in diamonds. Loads
// are hoisted into the header, while stores sink into the footer.
//
FunctionPass *createMergedLoadStoreMotionPass(bool SplitFooterBB = false);

//===----------------------------------------------------------------------===//
//
// ConstantHoisting - This pass prepares a function for expensive constants.
//
FunctionPass *createConstantHoistingPass();

//===----------------------------------------------------------------------===//
//
// Sink - Code Sinking
//
FunctionPass *createSinkingPass();

//===----------------------------------------------------------------------===//
//
// LowerAtomic - Lower atomic intrinsics to non-atomic form
//
Pass *createLowerAtomicPass();

//===----------------------------------------------------------------------===//
//
// LowerGuardIntrinsic - Lower guard intrinsics to normal control flow.
//
Pass *createLowerGuardIntrinsicPass();

//===----------------------------------------------------------------------===//
//
// LowerWidenableCondition - Lower widenable condition to i1 true.
//
Pass *createLowerWidenableConditionPass();

//===----------------------------------------------------------------------===//
//
// MergeICmps - Merge integer comparison chains into a memcmp
//
Pass *createMergeICmpsLegacyPass();

//===----------------------------------------------------------------------===//
//
// InferAddressSpaces - Modify users of addrspacecast instructions with values
// in the source address space if using the destination address space is slower
// on the target. If AddressSpace is left to its default value, it will be
// obtained from the TargetTransformInfo.
//
FunctionPass *createInferAddressSpacesPass(unsigned AddressSpace = ~0u);
extern char &InferAddressSpacesID;

//===----------------------------------------------------------------------===//
//
// LowerExpectIntrinsics - Removes llvm.expect intrinsics and creates
// "block_weights" metadata.
FunctionPass *createLowerExpectIntrinsicPass();

//===----------------------------------------------------------------------===//
//
// TLSVariableHoist - This pass reduce duplicated TLS address call.
//
FunctionPass *createTLSVariableHoistPass();

//===----------------------------------------------------------------------===//
//
// LowerConstantIntrinsicss - Expand any remaining llvm.objectsize and
// llvm.is.constant intrinsic calls, even for the unknown cases.
//
FunctionPass *createLowerConstantIntrinsicsPass();

//===----------------------------------------------------------------------===//
//
// PartiallyInlineLibCalls - Tries to inline the fast path of library
// calls such as sqrt.
//
FunctionPass *createPartiallyInlineLibCallsPass();

//===----------------------------------------------------------------------===//
//
// SeparateConstOffsetFromGEP - Split GEPs for better CSE
//
FunctionPass *createSeparateConstOffsetFromGEPPass(bool LowerGEP = false);

//===----------------------------------------------------------------------===//
//
// SpeculativeExecution - Aggressively hoist instructions to enable
// speculative execution on targets where branches are expensive.
//
FunctionPass *createSpeculativeExecutionPass();

// Same as createSpeculativeExecutionPass, but does nothing unless
// TargetTransformInfo::hasBranchDivergence() is true.
FunctionPass *createSpeculativeExecutionIfHasBranchDivergencePass();

//===----------------------------------------------------------------------===//
//
// StraightLineStrengthReduce - This pass strength-reduces some certain
// instruction patterns in straight-line code.
//
FunctionPass *createStraightLineStrengthReducePass();

//===----------------------------------------------------------------------===//
//
// NaryReassociate - Simplify n-ary operations by reassociation.
//
FunctionPass *createNaryReassociatePass();

//===----------------------------------------------------------------------===//
//
// LoopDataPrefetch - Perform data prefetching in loops.
//
FunctionPass *createLoopDataPrefetchPass();

//===----------------------------------------------------------------------===//
//
// LoopSimplifyCFG - This pass performs basic CFG simplification on loops,
// primarily to help other loop passes.
//
Pass *createLoopSimplifyCFGPass();

//===----------------------------------------------------------------------===//
//
// This pass does instruction simplification on each
// instruction in a function.
//
FunctionPass *createInstSimplifyLegacyPass();

#if defined(ENABLE_AUTOTUNER)
//===--------------------------------------------------------------------===//
//
// createAutotuningCompilePass - It writes IR files with -fautotune-generate
// for autotuning flow. It also enables/disables the execution of optimization
// passes in subsequent compilations (with -fautotune) based on autotuning
// methodology and available opportunities.
//
FunctionPass *
createAutoTuningCompileFunctionLegacyPass(std::string Pass = "unknown");
ModulePass *
createAutoTuningCompileModuleLegacyPass(std::string Pass = "unknown");
#endif

//===----------------------------------------------------------------------===//
//
// createScalarizeMaskedMemIntrinPass - Replace masked load, store, gather
// and scatter intrinsics with scalar code when target doesn't support them.
//
FunctionPass *createScalarizeMaskedMemIntrinLegacyPass();
} // End llvm namespace

#endif
