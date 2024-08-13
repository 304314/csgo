//===--- CallGraphGen.h - Analyze the callgraph of a LLVM bitcode file using
// pointer analysis ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_CALLGRAPHGEN_H
#define LLVM_TOOLS_CALLGRAPHGEN_H

#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/raw_ostream.h"
#include <deque>
#include <map>
#include <set>

namespace llvm {

/// Base dataflow visitor class, defines the dataflow function
template <class T> class DataflowVisitor {
public:
  virtual ~DataflowVisitor() {}

  Function *CurrentFunction;
  /**
   * @brief Dataflow Function invoked for each basic block.
   *
   * @param Block The Basic Block.
   * @param Dfval The input dataflow value.
   * @param Forward True to compute dfval forward, otherwise backward.
   */
  void compDFVal(BasicBlock *Block, T *Dfval, bool Forward) {
    if (Forward == true) {
      for (BasicBlock::iterator II = Block->begin(), IE = Block->end();
           II != IE; ++II) {
        Instruction *Inst = &*II;
        compDFVal(Inst, Dfval);
      }
    } else {
      for (BasicBlock::reverse_iterator II = Block->rbegin(),
                                        IE = Block->rend();
           II != IE; ++II) {
        Instruction *Inst = &*II;
        compDFVal(Inst, Dfval);
      }
    }
  }

  /**
   * @brief Dataflow Function invoked for each instruction.
   *
   * @param Inst The Instruction.
   * @param Dfval The input dataflow value.
   * @return True if dfval changed.
   */
  virtual void compDFVal(Instruction *Inst, T *Dfval) = 0;

  /**
   * @brief Merge of two dfvals, dest will be the merged result.
   *
   * @param Dest The destination dataflow value.
   * @param Src The source dataflow value.
   * @return True if dest changed.
   */
  virtual void merge(T *Dest, const T &Src) = 0;
};

/**
 * @brief Dummy class to provide a typedef for the detailed result set.
 * For each basicblock, we compute its input dataflow val and its output
 * dataflow val.
 */
template <class T> struct DataflowResult {
  typedef typename std::map<BasicBlock *, std::pair<T, T>> Type;
};

/**
 * @brief Compute a forward iterated fixedpoint dataflow function, using a
 * user-supplied visitor function.
 *
 * Note that the caller must ensure that the function is in fact a monotone
 * function, as otherwise the fixedpoint may not terminate.
 *
 * @param Fn The function.
 * @param Visitor A function to compute dataflow vals.
 * @param Result The results of the dataflow.
 * @param Initval The initial dataflow value.
 */
template <class T>
void compForwardDataflow(Function *Fn, DataflowVisitor<T> *Visitor,
                         typename DataflowResult<T>::Type *Result,
                         const T &Initval) {
  Visitor->CurrentFunction = Fn;
  std::set<BasicBlock *> Worklist;

  // Initialize the worklist with all exit blocks
  for (Function::iterator BI = Fn->begin(); BI != Fn->end(); ++BI) {
    BasicBlock *BB = &*BI;
    Result->insert(std::make_pair(BB, std::make_pair(Initval, Initval)));
    Worklist.insert(BB);
  }

  // Iteratively compute the dataflow result
  while (!Worklist.empty()) {
    BasicBlock *BB = *Worklist.begin();
    Worklist.erase(Worklist.begin());

    // Merge all incoming value
    T BBEnterval = (*Result)[BB].first;
    for (auto PI = pred_begin(BB), PE = pred_end(BB); PI != PE; ++PI) {
      BasicBlock *Pred = *PI;
      Visitor->merge(&BBEnterval, (*Result)[Pred].second);
    }

    (*Result)[BB].first = BBEnterval;

    Visitor->compDFVal(BB, &BBEnterval, true);

    // If outgoing value changed, propagate it along the CFG
    if (BBEnterval == (*Result)[BB].second)
      continue;
    (*Result)[BB].second = BBEnterval;

    for (auto SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI) {
      Worklist.insert(*SI);
    }
  }
}

/**
 * @brief Compute a backward iterated fixedpoint dataflow function, using a
 * user-supplied visitor function.
 *
 * Note that the caller must ensure that the function is in fact a monotone
 * function, as otherwise the fixedpoint may not terminate.
 *
 * @param Fn The function.
 * @param Visitor A function to compute dataflow vals.
 * @param Result The results of the dataflow.
 * @param Initval The initial dataflow value.
 */
template <class T>
void compBackwardDataflow(Function *Fn, DataflowVisitor<T> *Visitor,
                          typename DataflowResult<T>::Type *Result,
                          const T &Initval) {
  Visitor->CurrentFunction = Fn;
  std::set<BasicBlock *> Worklist;

  // Initialize the worklist with all entry blocks
  for (Function::iterator BI = Fn->begin(); BI != Fn->end(); ++BI) {
    BasicBlock *BB = &*BI;
    Result->insert(std::make_pair(BB, std::make_pair(Initval, Initval)));
    Worklist.insert(BB);
  }

  // Iteratively compute the dataflow result
  while (!Worklist.empty()) {
    BasicBlock *BB = *Worklist.begin();
    Worklist.erase(Worklist.begin());

    // Merge all outgoing value
    T BBExitval = (*Result)[BB].second;
    for (auto SI = succ_begin(BB), SE = succ_end(BB); SI != SE; ++SI) {
      BasicBlock *Succ = *SI;
      Visitor->merge(&BBExitval, (*Result)[Succ].first);
    }

    (*Result)[BB].second = BBExitval;

    Visitor->compDFVal(BB, &BBExitval, false);

    // If incoming value changed, propagate it along the CFG
    if (BBExitval == (*Result)[BB].first)
      continue;
    (*Result)[BB].first = BBExitval;

    for (auto PI = pred_begin(BB), PE = pred_end(BB); PI != PE; ++PI) {
      Worklist.insert(*PI);
    }
  }
}

/**
 * @brief Enum representing different kinds of constraints.
 */
enum class ConstraintKind {
  Copy = 0,
  GetAddr,
  Load,
  Store,
  Unsolved,
  Init,
};

class ConstraintGraph;

/**
 * @brief Class representing a node in the constraint graph.
 */
class ConstraintGraphNode {
public:
  static unsigned NodeIndex;
  using ConstraintRecord = std::pair<ConstraintGraphNode *, ConstraintKind>;

  /**
   * @brief Constructor for ConstraintGraphNode.
   *
   * @param VI The value associated with the node.
   * @param CSG The constraint graph.
   */
  ConstraintGraphNode(Value *VI, ConstraintGraph *CSG)
      : CSG(CSG), V(VI), Index(++NodeIndex) {}
  ConstraintGraphNode(const ConstraintGraphNode &) = delete;
  ConstraintGraphNode &operator=(const ConstraintGraphNode &) = delete;

  /**
   * @brief Destructor for ConstraintGraphNode.
   */
  ~ConstraintGraphNode() {
    for (auto Succ : ConstraintedSuccs) {
      Succ.first->dropRef();
    }
  }

  using iterator = std::vector<ConstraintRecord>::iterator;
  using const_iterator = std::vector<ConstraintRecord>::const_iterator;

  inline iterator begin() { return ConstraintedSuccs.begin(); }
  inline iterator end() { return ConstraintedSuccs.end(); }
  inline const_iterator begin() const { return ConstraintedSuccs.begin(); }
  inline const_iterator end() const { return ConstraintedSuccs.end(); }
  inline bool empty() const { return ConstraintedSuccs.empty(); }
  inline unsigned size() const {
    return static_cast<unsigned>(ConstraintedSuccs.size());
  }

  void addRef() { ++NumReferences; }
  void dropRef() { --NumReferences; }
  void allReferencesDropped() { NumReferences = 0; }

  ConstraintGraphNode *operator[](unsigned Idx) const {
    return ConstraintedSuccs[Idx].first;
  }

  /**
   * @brief Add a constraint to the node.
   *
   * @param Succ The successor node.
   * @param Kind The kind of constraint.
   */
  void addConstraint(ConstraintGraphNode *Succ, ConstraintKind Kind) {
    ConstraintedSuccs.push_back(std::make_pair(Succ, Kind));
    Succ->addRef();
  }

  /**
   * @brief Get the successor constraints of the node.
   *
   * @return A vector of successor constraints.
   */
  std::vector<ConstraintRecord> getConstraintSuccs() const {
    return ConstraintedSuccs;
  }

private:
  friend class ConstraintGraph;
  friend class PointerAnalysisVisitor;

  ConstraintGraph *CSG;

  std::vector<ConstraintRecord> ConstraintedSuccs;

  unsigned NumReferences = 0;

  Value *V;

public:
  unsigned Index;
};

/**
 * @brief Struct representing a constraint.
 */
struct Constraint {
  ConstraintGraphNode *Dst;
  ConstraintGraphNode *Src;
  ConstraintKind Kind;

public:
  /**
   * @brief Constructor for Constraint.
   *
   * @param CSDst The destination node.
   * @param CSSrc The source node.
   * @param CSKind The kind of constraint.
   */
  Constraint(ConstraintGraphNode *CSDst, ConstraintGraphNode *CSSrc,
             ConstraintKind CSKind)
      : Dst(CSDst), Src(CSSrc), Kind(CSKind) {}

  bool operator==(const Constraint &Other) const {
    return Src == Other.Src && Dst == Other.Dst && Kind == Other.Kind;
  }

  bool operator<(const Constraint &Other) const {
    if (Src->Index < Other.Src->Index)
      return true;
    if (Dst->Index < Other.Dst->Index)
      return true;
    return false;
  }
};

/**
 * @brief Class representing the constraint graph.
 */
class ConstraintGraph {
  Module &M;

  using ConstraintNodeMap =
      std::map<const Value *, std::unique_ptr<ConstraintGraphNode>>;

  ConstraintNodeMap ConstraintGraphNodes;

  ConstraintGraphNode *InitialConstraintNode;

  /**
   * @brief Helper function to get or create a constraint node for
   * initialization.
   *
   * @return A unique pointer to the created constraint node.
   */
  std::unique_ptr<ConstraintGraphNode> createInitialConstraintNode();

  // for debug purpose
  std::vector<Constraint *> Constraints;

public:
  /**
   * @brief Constructor for ConstraintGraph.
   *
   * @param Module The module.
   */
  explicit ConstraintGraph(Module &Module);
  ~ConstraintGraph();

  using iterator = ConstraintNodeMap::iterator;
  using const_iterator = ConstraintNodeMap::const_iterator;

  inline iterator begin() { return ConstraintGraphNodes.begin(); }
  inline iterator end() { return ConstraintGraphNodes.end(); }
  inline const_iterator begin() const { return ConstraintGraphNodes.begin(); }
  inline const_iterator end() const { return ConstraintGraphNodes.end(); }

  /**
   * @brief Get the initial constraint node.
   *
   * @return The initial constraint node.
   */
  ConstraintGraphNode *getInitialConstraintNode() const {
    return InitialConstraintNode;
  }

  /**
   * @brief Get or insert a constraint node.
   *
   * @param V The value.
   * @return The constraint node.
   */
  ConstraintGraphNode *getOrInsertConstraintNode(Value *V);

  /**
   * @brief Get or insert a constraint.
   *
   * @param Dst The destination value.
   * @param Src The source value.
   * @param Kind The kind of constraint.
   * @return The constraint.
   */
  Constraint *getOrInsertConstraint(Value *Dst, Value *Src,
                                    ConstraintKind Kind);

  ConstraintGraphNode *operator[](const Value *V) {
    return ConstraintGraphNodes[V].get();
  }
};

template <> struct GraphTraits<const ConstraintGraphNode *> {
  using NodeRef = const ConstraintGraphNode *;
  using CSNPairTy = ConstraintGraphNode::ConstraintRecord;
  using EdgeRef = const ConstraintGraphNode::ConstraintRecord;

  static NodeRef getEntryNode(const ConstraintGraphNode *CSN) { return CSN; }
  static const ConstraintGraphNode *CSNGetValue(CSNPairTy P) { return P.first; }

  using ChildIteratorType = mapped_iterator<ConstraintGraphNode::const_iterator,
                                            decltype(&CSNGetValue)>;
  using ChildEdgeIteratorType = ConstraintGraphNode::const_iterator;

  static ChildIteratorType child_begin(NodeRef N) {
    return ChildIteratorType(N->begin(), &CSNGetValue);
  }

  static ChildIteratorType child_end(NodeRef N) {
    return ChildIteratorType(N->end(), &CSNGetValue);
  }

  static ChildEdgeIteratorType child_edge_begin(NodeRef N) {
    return N->begin();
  }

  static ChildEdgeIteratorType child_edge_end(NodeRef N) { return N->end(); }

  static NodeRef edge_dest(EdgeRef E) { return E.first; }
};

template <> struct GraphTraits<ConstraintGraph *> {
  using PairTy =
      std::pair<const Function *const, std::unique_ptr<ConstraintGraphNode>>;
  using NodeRef = const ConstraintGraphNode *;
  using EdgeRef = const ConstraintGraphNode::ConstraintRecord;

  static NodeRef getEntryNode(const ConstraintGraph *CSG) {
    return CSG->getInitialConstraintNode();
  }

  using nodes_iterator = ConstraintGraph::const_iterator;

  static nodes_iterator nodes_begin(const ConstraintGraph *CSG) {
    return CSG->begin();
  }

  static nodes_iterator nodes_end(const ConstraintGraph *CSG) {
    return CSG->end();
  }
};

template <>
struct GraphTraits<const ConstraintGraph *>
    : public GraphTraits<const ConstraintGraphNode *> {
  using PairTy =
      std::pair<const Function *const, std::unique_ptr<ConstraintGraphNode>>;

  static NodeRef getEntryNode(const ConstraintGraph *CSG) {
    return CSG->getInitialConstraintNode();
  }

  using nodes_iterator = ConstraintGraph::const_iterator;

  static nodes_iterator nodes_begin(const ConstraintGraph *CSG) {
    return CSG->begin();
  }

  static nodes_iterator nodes_end(const ConstraintGraph *CSG) {
    return CSG->end();
  }
};

/**
 * @struct PointerAnalysisCLIConfig
 * @brief Configuration options for pointer analysis CLI.
 */
struct PointerAnalysisCLIConfig {
  bool UseAnders;
  bool UseDebug;
};

/// An analysis pass to compute the \c CallGraph for a \c Module using pointer
/// analysis.
///
/// This class implements the concept of an analysis pass used by the \c
/// ModuleAnalysisManager to run an analysis over a module and cache the
/// resulting data.
class PACallGraphAnalysis : public AnalysisInfoMixin<PACallGraphAnalysis> {
  friend AnalysisInfoMixin<PACallGraphAnalysis>;

  static AnalysisKey Key;

  PointerAnalysisCLIConfig Config;

public:
  explicit PACallGraphAnalysis(PointerAnalysisCLIConfig Config)
      : Config(Config) {}

  /// A formulaic type to inform clients of the result type.
  using Result = CallGraph;

  /// Compute the \c CallGraph for the module \c M.
  CallGraph run(Module &M, ModuleAnalysisManager &);
};

using PAAnalysisDataflowFacts = std::vector<Constraint *>;

/**
 * @class PointerAnalysisVisitor
 * @brief A visitor class for performing pointer analysis on a given module.
 *
 * This class inherits from the DataflowVisitor<PAAnalysisDataflowFacts> class
 * and is responsible for performing pointer analysis on a given module. It
 * maintains various data structures and maps to store information related to
 * constraint graphs, call graphs, point-to sets, unresolved arguments, function
 * return value point-to sets, constraint-function mappings, and a deque of
 * constraints.
 *
 * The main functionality of this class includes solving constraints, merging
 * dataflow facts, computing dataflow values for instructions, and transferring
 * dataflow facts for load, store, and call instructions. It also provides
 * methods for propagating constraints, solving specific types of constraints,
 * and printing the constraint graph and point-to set map.
 *
 * @see DataflowVisitor
 * @see PAAnalysisDataflowFacts
 */
class PointerAnalysisVisitor : public DataflowVisitor<PAAnalysisDataflowFacts> {
  ConstraintGraph CSG;
  CallGraph CG;
  // [todo][optimize] use more efficient data structures
  std::map<const ConstraintGraphNode *, std::set<Value *>> PointToSetMap;
  std::map<const ConstraintGraphNode *, std::vector<ConstraintGraphNode *>>
      UnresolvedArgs;
  std::map<const Function *, std::set<Value *>>
      FunctionReturnValuePointToSetMap;
  std::map<const Constraint *, Function *> ConstraintFunctionMap;
  std::deque<Constraint *> Constraints;
  std::set<Constraint *> CollectedConstraints;

public:
  friend class PACallGraphAnalysis;
  PointerAnalysisVisitor(Module &Module);
  void solveConstraint();

public:
  void merge(PAAnalysisDataflowFacts *Facts,
             const PAAnalysisDataflowFacts &OtherFacts) override;
  void compDFVal(Instruction *Inst, PAAnalysisDataflowFacts *Dfval) override;
  void transfer(LoadInst *Inst, PAAnalysisDataflowFacts *Dfval);
  void transfer(StoreInst *Inst, PAAnalysisDataflowFacts *Dfval);
  void transfer(CallInst *Inst, PAAnalysisDataflowFacts *Dfval);

  void propagate(const ConstraintGraphNode *Src);
  void solveLoadConstraint(const Constraint *Cstrt);
  void solveStoreConstraint(const Constraint *Cstrt);
  void solveGetAddrConstraint(const Constraint *Cstrt);
  void solveCopyConstraint(const Constraint *Cstrt);
  void solveUnsolvedConstraint(const Constraint *Cstrt);
  void printConstraintGraph(raw_ostream &OS);
  void printPointToSetMap(raw_ostream &OS);
};

} // namespace llvm

#endif
