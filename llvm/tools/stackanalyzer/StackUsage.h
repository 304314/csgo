//===--- StackUsage.h - Analyze the stack usage of functions inside a LLVM
// bitcode file ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_STACKUSAGE_H
#define LLVM_TOOLS_STACKUSAGE_H

#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include <set>

namespace llvm {
/**
 * @brief Parses the stack size from the stack usage file.
 *
 * This function reads the stack usage information from the specified .su file
 * and populates the provided map with the stack sizes for each function
 * in the given module.
 *
 * @param Filename The name of the stack usage file(in .su format) to read.
 * @param Module The LLVM module containing the functions.
 * @param StackSizeMap A map to be populated with the stack sizes for each
 * function.
 */
void parseStackSizeFromSU(
    llvm::StringRef Filename, llvm::Module &Module,
    llvm::MapVector<const llvm::Function *, unsigned> &StackSizeMap);

/**
 * @brief Emits a stack usage file for the given module.
 *
 * This function generates a stack usage file for the specified module. The
 * stack usage file contains information about the stack usage of the functions
 * in the module.
 *
 * @param Filename The path to the output file.
 * @param Module The LLVM module for which the stack usage file is generated.
 */
void emitSUFile(llvm::StringRef Filename, llvm::Module &Module);

/**
 * @class StackOverflowDetector
 * @brief A class that detects stack overflow in a program.
 *
 * The StackOverflowDetector class analyzes the call graph of a program and
 * detects potential stack overflow paths. It uses a depth-first search
 * algorithm to traverse the call graph and keeps track of the stack sizes of
 * each function. The class provides a method to analyze the call graph and
 * print the results.
 *
 * @note This class assumes that the call graph and stack sizes have already
 * been computed.
 */
class StackOverflowDetector {

  SmallVector<std::vector<const Function *>, 4> OverflowPaths;
  MapVector<const Function *, unsigned> PathStack;
  std::set<Function *> Visited;
  unsigned Threshold;

  bool dfs(Function *F, const CallGraph &CG,
           const MapVector<const Function *, unsigned> &StackSizes);

public:
  StackOverflowDetector(unsigned Limit) : Threshold(Limit) {}

  void analyze(const CallGraph &CG,
               const MapVector<const Function *, unsigned> &);

  void printResults(raw_ostream &OS) const;
};
} // namespace llvm

#endif
