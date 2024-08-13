//===--- StackUsage.cpp - Analyze the callgraph of a LLVM bitcode file using
// pointer analysis ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "StackUsage.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

void llvm::parseStackSizeFromSU(
    StringRef Filename, llvm::Module &Module,
    MapVector<const llvm::Function *, unsigned> &StackSizeMap) {

  // Open the file and read its contents
  auto BufferOrError = MemoryBuffer::getFile(Filename);
  if (std::error_code EC = BufferOrError.getError()) {
    errs() << "Error opening file " << Filename << ": " << EC.message() << "\n";
    return;
  }

  std::unique_ptr<MemoryBuffer> Buffer = std::move(BufferOrError.get());
  StringRef Content = Buffer->getBuffer();

  // Split the file content into lines
  SmallVector<StringRef, 16> Lines;
  Content.split(Lines, '\n');

  // Iterate through each line
  for (StringRef Line : Lines) {
    if (Line.trim().empty())
      continue; // Skip empty lines

    // Split the line by tabs
    SmallVector<StringRef, 4> Parts;
    Line.split(Parts, '\t', -1, false);

    if (Parts.size() < 3) {
      errs() << "Invalid format in line: " << Line << "\n";
      continue;
    }

    // Extract the function name and stack size
    StringRef FullFunctionName = Parts[0];
    StringRef StackSizeStr = Parts[1];

    // Parse the stack size
    unsigned StackSize;
    if (StackSizeStr.getAsInteger(10, StackSize)) {
      errs() << "Invalid stack size in line: " << Line << "\n";
      continue;
    }

    // Extract the function name (remove path and extension)
    StringRef FunctionName = sys::path::filename(FullFunctionName);
    FunctionName = FunctionName.rsplit(':').second;

    // Find the corresponding function in the module
    Function *F = Module.getFunction(FunctionName);
    if (!F) {
      errs() << "Function " << FunctionName << " not found in module\n";
      continue;
    }

    // Insert the function and its stack size into the map
    StackSizeMap[F] = StackSize;
  }
}

void StackOverflowDetector::analyze(
    const CallGraph &CG,
    const MapVector<const Function *, unsigned> &StackSizes) {
  for (const auto &Node : CG) {
    Function *F = Node.second->getFunction();
    if (!F)
      continue;
    if (F->isDeclaration())
      continue;
    if (!Visited.count(F)) {
      if (F->getName() == "main")
        dfs(F, CG, StackSizes);
    }
  }
}

void StackOverflowDetector::printResults(raw_ostream &OS) const {
  OS << "Stack Overflow Detector Results(Limit:" << Threshold << ")\n";
  for (const auto &Path : OverflowPaths) {
    OS << "Path:\n";
    for (auto *F : Path) {
      OS << "  " << F->getName() << "\n";
    }
  }
}

bool StackOverflowDetector::dfs(
    Function *F, const CallGraph &CG,
    const MapVector<const Function *, unsigned> &StackSizes) {
  Visited.insert(F);
  unsigned CurrentStackSize = StackSizes.lookup(F);
  PathStack.insert({F, CurrentStackSize});
  unsigned CumulativeStackSize = 0;
  for (auto &Entry : PathStack) {
    CumulativeStackSize += Entry.second;
  }
  if (CumulativeStackSize > Threshold) {
    std::vector<const Function *> Path;
    for (auto &Entry : PathStack) {
      Path.push_back(Entry.first);
    }
    OverflowPaths.push_back(Path);
    return true;
  }

  auto *CGNode = CG[F];
  for (auto &Callee : *CGNode) {
    Function *CalleeF = Callee.second->getFunction();
    if (CalleeF && !Visited.count(CalleeF) && !CalleeF->isDeclaration()) {
      if (dfs(CalleeF, CG, StackSizes)) {
        return true;
      }
    }
  }

  PathStack.pop_back();
  return false;
}