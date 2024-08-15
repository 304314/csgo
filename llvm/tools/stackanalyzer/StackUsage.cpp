//===--- StackUsage.cpp - Analyze the callgraph of a LLVM bitcode file using
// pointer analysis ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "StackUsage.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <optional>

using namespace llvm;
using namespace llvm::sys;

void llvm::parseStackSizeFromSU(
    StringRef Filename, Module &Module,
    MapVector<const Function *, unsigned> &StackSizeMap) {
  emitSUFile(Filename, Module);
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

void llvm::emitSUFile(llvm::StringRef SUFilename, llvm::Module &Module) {
  auto TargetTriple = getDefaultTargetTriple();
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  std::string Error;
  auto *Target = TargetRegistry::lookupTarget(TargetTriple, Error);
  if (!Target) {
    errs() << "Error: " << Error << "\n";
    return;
  }

  auto *CPU = "generic";
  auto *Features = "";

  TargetOptions Opt;
  Opt.StackUsageOutput = SUFilename;
  auto RM = std::optional<Reloc::Model>();
  auto *TargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, Opt, RM);

  Module.setDataLayout(TargetMachine->createDataLayout());
  Module.setTargetTriple(TargetTriple);

  auto *Filename = "./output.o";
  std::error_code EC;
  raw_fd_ostream Dest(Filename, EC, sys::fs::OF_None);

  if (EC) {
    errs() << "Error opening file: " << EC.message() << "\n";
    return;
  }

  legacy::PassManager Pass;
  auto FileType = CGFT_ObjectFile;

  if (TargetMachine->addPassesToEmitFile(Pass, Dest, nullptr, FileType)) {
    errs() << "TargetMachine can't emit a file of this type";
    return;
  }

  Pass.run(Module);
  Dest.flush();
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
        traverse(F, CG, StackSizes);
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

bool StackOverflowDetector::evaluateCurrentPath() {
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
  return false;
}

bool StackOverflowDetector::traverse(
    Function *F, const CallGraph &CG,
    const MapVector<const Function *, unsigned> &StackSizes) {
  // Check for loop detection: if we revisit a node that is in the PathStack,
  // it's a loop
  if (PathStack.count(F)) {
    unsigned LoopStackSize = 0;
    for (auto PI = PathStack.find(F), PE = PathStack.end(); PI != PE; ++PI) {
      LoopStackSize += PI->second;
    }

    // If the loop's stack cost is zero, treat it as a single node and evaluate
    // current path
    if (LoopStackSize == 0) {
      return evaluateCurrentPath();
    }
    // Otherwise, consider it a potential overflow path
    std::vector<const Function *> Path;
    for (auto &Entry : PathStack) {
      Path.push_back(Entry.first);
    }
    OverflowPaths.push_back(Path);
    return true;
  }

  Visited.insert(F);
  unsigned CurrentStackSize = StackSizes.lookup(F);
  PathStack.insert({F, CurrentStackSize});
  if (evaluateCurrentPath()) {
    return true;
  }
  auto *CGNode = CG[F];

  bool FindOverflowPath = false;
  for (auto &Callee : *CGNode) {
    Function *CalleeF = Callee.second->getFunction();
    if (CalleeF && !CalleeF->isDeclaration()) {
      FindOverflowPath = traverse(CalleeF, CG, StackSizes) || FindOverflowPath;
    }
  }

  PathStack.pop_back();
  return FindOverflowPath;
}