//===- MachineBlockFrequencyInfo.h - MBB Frequency Analysis -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Loops should be simplified before this analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_MACHINEBLOCKFREQUENCYINFO_H
#define LLVM_CODEGEN_MACHINEBLOCKFREQUENCYINFO_H

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/Support/BlockFrequency.h"
#include <cstdint>
#include <memory>
#include <optional>

namespace llvm {

template <class BlockT> class BlockFrequencyInfoImpl;
class MachineBasicBlock;
class MachineBranchProbabilityInfo;
class MachineFunction;
class MachineLoopInfo;
class raw_ostream;

/// MachineBlockFrequencyInfo pass uses BlockFrequencyInfoImpl implementation
/// to estimate machine basic block frequencies.
class MachineBlockFrequencyInfo : public MachineFunctionPass {
  using ImplType = BlockFrequencyInfoImpl<MachineBasicBlock>;
  std::unique_ptr<ImplType> MBFI;

public:
  static char ID;

  MachineBlockFrequencyInfo();
  explicit MachineBlockFrequencyInfo(MachineFunction &F,
                                     MachineBranchProbabilityInfo &MBPI,
                                     MachineLoopInfo &MLI);
  ~MachineBlockFrequencyInfo() override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnMachineFunction(MachineFunction &F) override;

  /// calculate - compute block frequency info for the given function.
  void calculate(const MachineFunction &F,
                 const MachineBranchProbabilityInfo &MBPI,
                 const MachineLoopInfo &MLI);

  void releaseMemory() override;

  /// getblockFreq - Return block frequency. Return 0 if we don't have the
  /// information. Please note that initial frequency is equal to 1024. It means
  /// that we should not rely on the value itself, but only on the comparison to
  /// the other block frequencies. We do this to avoid using of floating points.
  /// For example, to get the frequency of a block relative to the entry block,
  /// divide the integral value returned by this function (the
  /// BlockFrequency::getFrequency() value) by getEntryFreq().
  BlockFrequency getBlockFreq(const MachineBasicBlock *MBB) const;

  /// Compute the frequency of the block, relative to the entry block.
  /// This API assumes getEntryFreq() is non-zero.
  double getBlockFreqRelativeToEntryBlock(const MachineBasicBlock *MBB) const {
    assert(getEntryFreq() != 0 && "getEntryFreq() should not return 0 here!");
    return static_cast<double>(getBlockFreq(MBB).getFrequency()) /
           static_cast<double>(getEntryFreq());
  }

  std::optional<uint64_t>
  getBlockProfileCount(const MachineBasicBlock *MBB) const;
  std::optional<uint64_t> getProfileCountFromFreq(uint64_t Freq) const;

  bool isIrrLoopHeader(const MachineBasicBlock *MBB) const;

  /// incrementally calculate block frequencies when we split edges, to avoid
  /// full CFG traversal.
  void onEdgeSplit(const MachineBasicBlock &NewPredecessor,
                   const MachineBasicBlock &NewSuccessor,
                   const MachineBranchProbabilityInfo &MBPI);

  const MachineFunction *getFunction() const;
  const MachineBranchProbabilityInfo *getMBPI() const;

  /// Pop up a ghostview window with the current block frequency propagation
  /// rendered using dot.
  void view(const Twine &Name, bool isSimple = true) const;

  /// Divide a block's BlockFrequency::getFrequency() value by this value to
  /// obtain the entry block - relative frequency of said block.
  uint64_t getEntryFreq() const;
};

/// Print the block frequency @p Freq relative to the current functions entry
/// frequency. Returns a Printable object that can be piped via `<<` to a
/// `raw_ostream`.
Printable printBlockFreq(const MachineBlockFrequencyInfo &MBFI,
                         BlockFrequency Freq);

/// Convenience function equivalent to calling
/// `printBlockFreq(MBFI, MBFI.getBlockFreq(&MBB))`.
Printable printBlockFreq(const MachineBlockFrequencyInfo &MBFI,
                         const MachineBasicBlock &MBB);

} // end namespace llvm

#endif // LLVM_CODEGEN_MACHINEBLOCKFREQUENCYINFO_H
