//===--- CallGraphGen.cpp - Analyze the callgraph of a LLVM bitcode file using
// pointer analysis ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CallGraphGen.h"

using namespace llvm;

unsigned ConstraintGraphNode::NodeIndex = 0;

std::unique_ptr<ConstraintGraphNode>
ConstraintGraph::createInitialConstraintNode() {
  auto *Int32Type = llvm::Type::getInt32Ty(M.getContext());
  auto *Int32PtrType = llvm::PointerType::get(Int32Type, 0);
  auto *NullInt32Ptr = llvm::ConstantPointerNull::get(Int32PtrType);
  return std::make_unique<ConstraintGraphNode>((NullInt32Ptr), this);
}

ConstraintGraph::ConstraintGraph(Module &Module) : M(Module) {
  InitialConstraintNode = createInitialConstraintNode().get();
  for (auto &F : M) {
    if (F.hasExternalLinkage())
      continue;
    if (F.isIntrinsic())
      continue;
  }
}

ConstraintGraph::~ConstraintGraph() {
  // [todo][implementation] memory management
}

ConstraintGraphNode *ConstraintGraph::getOrInsertConstraintNode(Value *V) {
  auto &CSN = ConstraintGraphNodes[V];
  if (CSN) {
    return CSN.get();
  }
  CSN = std::make_unique<ConstraintGraphNode>(V, this);
  return CSN.get();
}

Constraint *ConstraintGraph::getOrInsertConstraint(Value *Dst, Value *Src,
                                                   ConstraintKind Kind) {
  for (auto &C : Constraints) {
    if (C->Src->V == Src && C->Dst->V == Dst && C->Kind == Kind) {
      return C;
    }
  }
  auto *SrcNode = getOrInsertConstraintNode(Src);
  auto *DstNode = getOrInsertConstraintNode(Dst);
  auto *ConstraitEdge = new Constraint(DstNode, SrcNode, Kind);
  SrcNode->addConstraint(DstNode, Kind);
  Constraints.push_back(ConstraitEdge);
  return ConstraitEdge;
}

AnalysisKey PACallGraphAnalysis::Key;

PACallGraphAnalysis::Result PACallGraphAnalysis::run(Module &M,
                                                     ModuleAnalysisManager &) {
  auto FI = M.rbegin(), FE = M.rend();
  for (; (FI->hasExternalLinkage() || FI->isIntrinsic()) && FI != FE; ++FI) {
    // start from main for now
    if (FI->getName() == "main") {
      break;
    }
  }
  DataflowResult<PAAnalysisDataflowFacts>::Type ResultFact;
  PAAnalysisDataflowFacts InitFact;
  PointerAnalysisVisitor PAVisitor(M);
  if (Config.UseAnders) {
    compForwardDataflow(&(*FI), &PAVisitor, &ResultFact, InitFact);
    PAVisitor.solveConstraint();
  }
  PAVisitor.canonicalizeCallGraph();
  if (Config.UseDebug) {
    PAVisitor.printConstraintGraph(outs());
    PAVisitor.printPointToSetMap(outs());
  }
  return std::move(PAVisitor.CG);
}

// for debugging purpose, reference:
// https://github.com/SunnyWadkar/LLVM-DataFlow-Analysis/blob/master/Dataflow/available-support.cpp
static std::string getShortValueName(const Value *V) {
  if (auto *Arg = dyn_cast<Argument>(V)) {
    return Arg->getName().str() + ":" + Arg->getParent()->getName().str();
  }
  if (auto *InstV = dyn_cast<Instruction>(V)) {
    std::string S = "";
    raw_string_ostream *Strm = new raw_string_ostream(S);
    V->print(*Strm);
    std::string Inst = Strm->str();
    size_t Idx1 = Inst.find("%");
    size_t Idx2 = Inst.find(" ", Idx1);
    if (Idx1 != std::string::npos && Idx2 != std::string::npos) {
      return Inst.substr(Idx1, Idx2 - Idx1) + ":" +
             InstV->getFunction()->getName().str();
    }
    return "\"" + Inst + "\"";
  }
  if (const ConstantInt *Cint = dyn_cast<ConstantInt>(V)) {
    std::string S = "";
    raw_string_ostream *Strm = new raw_string_ostream(S);
    Cint->getValue().print(*Strm, true);
    return Strm->str();
  }
  if (V->getName().str().length() > 0) {
    return V->getName().str();
  }
  std::string S = "";
  raw_string_ostream *Strm = new raw_string_ostream(S);
  V->print(*Strm);
  std::string Inst = Strm->str();
  return "\"" + Inst + "\"";
}

PointerAnalysisVisitor::PointerAnalysisVisitor(Module &InitModule)
    : CSG(ConstraintGraph(InitModule)), CG(InitModule) {
  for (auto &F : InitModule) {
    if (F.hasExternalLinkage())
      continue;
    if (F.isIntrinsic())
      continue;
  }
}

void PointerAnalysisVisitor::merge(PAAnalysisDataflowFacts *Facts,
                                   const PAAnalysisDataflowFacts &OtherFacts) {
  Facts->insert(Facts->end(), OtherFacts.begin(), OtherFacts.end());
  std::sort(Facts->begin(), Facts->end());
  auto Last = std::unique(Facts->begin(), Facts->end());
  Facts->erase(Last, Facts->end());
}

void PointerAnalysisVisitor::compDFVal(Instruction *Inst,
                                       PAAnalysisDataflowFacts *Dfval) {
  switch (Inst->getOpcode()) {
  case Instruction::Load: {
    transfer(dyn_cast<LoadInst>(Inst), Dfval);
    break;
  }
  case Instruction::Store: {
    transfer(dyn_cast<StoreInst>(Inst), Dfval);
    break;
  }
  case Instruction::Call: {
    transfer(dyn_cast<CallInst>(Inst), Dfval);
    break;
  }
  }
}

void PointerAnalysisVisitor::transfer(LoadInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Addr = Inst->getPointerOperand();
  auto *Constraint =
      CSG.getOrInsertConstraint(Inst, Addr, ConstraintKind::Load);
  Dfval->push_back(Constraint);
  Constraints.push_back(Constraint);
}

void PointerAnalysisVisitor::transfer(StoreInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Addr = Inst->getPointerOperand();
  auto *Val = Inst->getValueOperand();
  Constraint *Cstrt;
  if (isa<Function>(Val)) {
    Cstrt = CSG.getOrInsertConstraint(Addr, Val, ConstraintKind::GetAddr);
  } else {
    Cstrt = CSG.getOrInsertConstraint(Addr, Val, ConstraintKind::Store);
  }
  Dfval->push_back(Cstrt);
  Constraints.push_back(Cstrt);
}

void PointerAnalysisVisitor::transfer(CallInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Callee = Inst->getCalledFunction();
  if (!Callee) {
    auto *CalleeValue = Inst->getCalledOperand();
    auto *Constraint =
        CSG.getOrInsertConstraint(Inst, CalleeValue, ConstraintKind::Unsolved);
    for (unsigned I = 0, NumOperands = Inst->arg_size(); I != NumOperands;
         ++I) {
      auto *RArg = Inst->getArgOperand(I);
      auto *CSGN = CSG.getOrInsertConstraintNode(RArg);
      UnresolvedArgs[Constraint->Src].push_back(CSGN);
    }
    Dfval->push_back(Constraint);
    Constraints.push_back(Constraint);
    ConstraintFunctionMap[Constraint] = CurrentFunction;
  } else {
    auto *PrevFunction = CurrentFunction;
    if (Callee->isIntrinsic() || Callee->isDeclaration())
      return;
    for (unsigned I = 0, NumOperands = Inst->arg_size(); I != NumOperands;
         ++I) {
      auto *RArg = Inst->getArgOperand(I);
      auto *FArg = Callee->getArg(I);
      auto *Constraint =
          CSG.getOrInsertConstraint(FArg, RArg, ConstraintKind::Copy);
      Dfval->push_back(Constraint);
      Constraints.push_back(Constraint);
    }
    DataflowResult<PAAnalysisDataflowFacts>::Type SubroutineResult;
    PAAnalysisDataflowFacts SubroutineInitFact;
    compForwardDataflow(Callee, this, &SubroutineResult, SubroutineInitFact);
    CurrentFunction = PrevFunction;
  }
}

using ConstraintSolverFn =
    void (PointerAnalysisVisitor::*)(const Constraint *Cstrt);

static ConstraintSolverFn ConstraintSolvers[] = {
    &PointerAnalysisVisitor::solveCopyConstraint,
    &PointerAnalysisVisitor::solveGetAddrConstraint,
    &PointerAnalysisVisitor::solveLoadConstraint,
    &PointerAnalysisVisitor::solveStoreConstraint,
    &PointerAnalysisVisitor::solveUnsolvedConstraint,
    nullptr // ConstraintKind::Init
};

void PointerAnalysisVisitor::solveConstraint() {
  // [todo][optimize] optimize this worklist algorithm using incremental
  // point-to set information
  CollectedConstraints =
      std::set<Constraint *>(Constraints.begin(), Constraints.end());
  while (!Constraints.empty()) {
    auto *Constraint = Constraints.front();
    Constraints.pop_front();
    auto FormerPointToSet = PointToSetMap;
    (this->*ConstraintSolvers[static_cast<unsigned>(Constraint->Kind)])(
        Constraint);
    if ((PointToSetMap[Constraint->Src] != FormerPointToSet[Constraint->Src]) ||
        (PointToSetMap[Constraint->Dst] != FormerPointToSet[Constraint->Dst])) {
      Constraints.push_back(Constraint);
    }
    CollectedConstraints.insert(Constraint);
  }
}

void PointerAnalysisVisitor::propagate(const ConstraintGraphNode *Src) {
  // [todo][optimize] handle cycles in propagation
  for (auto Constraint : Src->getConstraintSuccs()) {
    auto *Dst = Constraint.first;
    auto Kind = Constraint.second;
    auto Solver = ConstraintSolvers[static_cast<unsigned>(Kind)];
    (this->*Solver)(CSG.getOrInsertConstraint(Dst->V, Src->V, Kind));
  }
}

void PointerAnalysisVisitor::solveLoadConstraint(const Constraint *Cstrt) {
  if (PointToSetMap[Cstrt->Src].empty()) {
    PointToSetMap[Cstrt->Src].insert(Cstrt->Dst->V);
    return;
  }
  for (auto *PointToValue : PointToSetMap[Cstrt->Src]) {
    auto *CSGN = CSG.getOrInsertConstraintNode(PointToValue);
    PointToSetMap[Cstrt->Dst].insert(PointToSetMap[CSGN].begin(),
                                     PointToSetMap[CSGN].end());
  }
  propagate(Cstrt->Dst);
}

void PointerAnalysisVisitor::solveStoreConstraint(const Constraint *Cstrt) {
  if (PointToSetMap[Cstrt->Dst].empty()) {
    PointToSetMap[Cstrt->Dst].insert(Cstrt->Src->V);
    return;
  }
  for (auto *PointToValue : PointToSetMap[Cstrt->Dst]) {
    auto *Node = CSG.getOrInsertConstraintNode(PointToValue);
    PointToSetMap[Node].insert(PointToSetMap[Cstrt->Src].begin(),
                               PointToSetMap[Cstrt->Src].end());
    if (Node != Cstrt->Src)
      propagate(Node);
  }
}

void PointerAnalysisVisitor::solveGetAddrConstraint(const Constraint *Cstrt) {
  PointToSetMap[Cstrt->Src].insert(Cstrt->Src->V);
  PointToSetMap[Cstrt->Dst].insert(Cstrt->Src->V);
  propagate(Cstrt->Dst);
}

void PointerAnalysisVisitor::solveCopyConstraint(const Constraint *Cstrt) {
  PointToSetMap[Cstrt->Dst].insert(PointToSetMap[Cstrt->Src].begin(),
                                   PointToSetMap[Cstrt->Src].end());
  propagate(Cstrt->Dst);
}

void PointerAnalysisVisitor::solveUnsolvedConstraint(const Constraint *Cstrt) {
  auto *Call = dyn_cast<CallInst>(Cstrt->Dst->V);
  assert(Call && "Dst should be a CallInst in a Unresolved constraint");
  for (auto *PointToValue : PointToSetMap[Cstrt->Src]) {
    if (auto *Callee = dyn_cast<Function>(PointToValue)) {
      DataflowResult<PAAnalysisDataflowFacts>::Type SubroutineResult;
      PAAnalysisDataflowFacts SubroutineInitFact;
      compForwardDataflow(Callee, this, &SubroutineResult, SubroutineInitFact);
      PointToSetMap[Cstrt->Dst].insert(
          FunctionReturnValuePointToSetMap[Callee].begin(),
          FunctionReturnValuePointToSetMap[Callee].end());
      for (unsigned I = 0, NumOperands = static_cast<unsigned>(
                               UnresolvedArgs[Cstrt->Src].size());
           I != NumOperands; ++I) {
        auto *RArg = UnresolvedArgs[Cstrt->Src][I]->V;
        auto *FArg = Callee->getArg(I);
        auto *Constraint =
            CSG.getOrInsertConstraint(FArg, RArg, ConstraintKind::Copy);
        Constraints.push_back(Constraint);
      }
      auto *CallGraphNode = CG[ConstraintFunctionMap[Cstrt]];
      bool Extend = false;
      for (auto CallNode : *CallGraphNode) {
        if (CallNode.second->getFunction() == Callee)
          Extend = true;
      }
      if (!Extend) {
        CallGraphNode->addCalledFunction(Call, CG.getOrInsertFunction(Callee));
      }
    }
  }
}

/**
 * @brief Canonicalizes the call graph by removing redundant self-loops.
 *
 * This function iterates over all nodes in the call graph and checks if there
 * are multiple self-loop edges (calls where a function calls itself). If more
 * than one self-loop edge is found, the redundant edges are removed, leaving
 * only one self-loop per node.
 */
void PointerAnalysisVisitor::canonicalizeCallGraph() {
  for (auto &Node : CG) {
    auto *CallGraphNode = Node.second.get();
    unsigned Cycles = 0;
    for (auto CI = CallGraphNode->begin(), CE = CallGraphNode->end(); CI != CE;
         CI++) {
      auto *CallRecord = CI->second;
      auto *Callee = CallRecord->getFunction();
      if (Callee == CallGraphNode->getFunction()) {
        Cycles++;
        if (Cycles > 1)
          CallGraphNode->removeCallEdge(CI);
      }
    }
  }
}

std::array<std::string, static_cast<unsigned>(ConstraintKind::Init)>
    ConstraintKindToString = {"Copy", "GetAddr", "Load", "Store", "Unresolved"};

/** * @brief Prints the constraint graph.
 *
 * This function prints the constraint graph in the DOT format. The graph
 * represents the constraints between different values in the analysis. The
 * constraints are grouped by function name and printed accordingly. If a value
 * has function information, it is grouped under the respective function name.
 * If a value does not have function information, it is grouped under
 * "CrossFunction".
 *
 * @param OS The output stream to which the graph will be printed.
 */
void PointerAnalysisVisitor::printConstraintGraph(raw_ostream &OS) {
  OS << "digraph \"Constraint Graph\" {\n";

  // Map to group constraints by function name
  std::map<std::string, std::vector<std::string>> FunctionConstraints;

  for (const auto &Constraint : CollectedConstraints) {
    std::string SrcName = getShortValueName(Constraint->Src->V);
    std::string DstName = getShortValueName(Constraint->Dst->V);

    // Extract function names from source and destination
    size_t SrcFunctionIdx = SrcName.find(":");
    size_t DstFunctionIdx = DstName.find(":");

    std::string FunctionName;

    if (SrcFunctionIdx != std::string::npos) {
      FunctionName = SrcName.substr(SrcFunctionIdx + 1);
    } else if (DstFunctionIdx != std::string::npos) {
      FunctionName = DstName.substr(DstFunctionIdx + 1);
    }

    std::string ConstraintStr =
        "  \"" + SrcName + "\" -> \"" + DstName + "\" [label=\"" +
        ConstraintKindToString[static_cast<unsigned>(Constraint->Kind)] +
        "\"];\n";

    if (SrcFunctionIdx != std::string::npos &&
        DstFunctionIdx != std::string::npos &&
        SrcName.substr(SrcFunctionIdx + 1) ==
            DstName.substr(DstFunctionIdx + 1)) {
      // Both have function information and it's the same
      FunctionConstraints[FunctionName].push_back(ConstraintStr);
    } else if (!FunctionName.empty()) {
      // At least one has function information, so group by that
      FunctionConstraints[FunctionName].push_back(ConstraintStr);
    } else {
      // If neither has function information (should not happen), group under
      // "CrossFunction"
      FunctionConstraints["CrossFunction"].push_back(ConstraintStr);
    }
  }

  // Print grouped constraints by function
  for (const auto &Entry : FunctionConstraints) {
    OS << "// Function: " << Entry.first << "\n";
    for (const auto &Cstrt : Entry.second) {
      OS << Cstrt;
    }
  }

  OS << "}\n";
}

void PointerAnalysisVisitor::printPointToSetMap(raw_ostream &OS) {
  // Map to group PointToSet entries by function name
  std::map<std::string,
           std::vector<std::pair<std::string, std::vector<Value *>>>>
      FunctionPointSetMap;

  for (const auto &Map : PointToSetMap) {
    std::string VarName = getShortValueName(Map.first->V);

    // Extract function name from the variable name
    size_t FunctionIdx = VarName.find(":");
    if (FunctionIdx != std::string::npos) {
      std::string FunctionName = VarName.substr(FunctionIdx + 1);

      // Store the variable and its PointToSet values in the corresponding
      // function's group
      FunctionPointSetMap[FunctionName].emplace_back(
          VarName, std::vector<Value *>(Map.second.begin(), Map.second.end()));
    }
  }

  // Print grouped PointToSet entries by function
  for (const auto &Entry : FunctionPointSetMap) {
    OS << "// Function: " << Entry.first << "\n";
    for (const auto &VarAndPointToSet : Entry.second) {
      OS << "PointToSet for " << VarAndPointToSet.first << ":";
      for (const auto &PointToValue : VarAndPointToSet.second) {
        OS << "  " << getShortValueName(PointToValue) << "";
      }
      OS << "\n";
    }
  }
}