/******************************************************************************
 * Copyright (c) 2020 Fabian Schiebel.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Fabian Schiebel and others
 *****************************************************************************/

#ifndef PHASAR_PHASARLLVM_IFDSIDE_PROBLEMS_IDEGENERALIZEDLCA_IDEGENERALIZEDLCA_H_
#define PHASAR_PHASARLLVM_IFDSIDE_PROBLEMS_IDEGENERALIZEDLCA_IDEGENERALIZEDLCA_H_

#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/DataFlowSolver/IfdsIde/IDETabulationProblem.h"
#include "phasar/PhasarLLVM/DataFlowSolver/IfdsIde/Problems/IDEGeneralizedLCA/EdgeValueSet.h"
#include "phasar/PhasarLLVM/Utils/Printer.h"

namespace psr {
/// \brief An implementation of a linear constant analysis, similar to
/// IDELinearConstantAnalysis, but with an extended edge-value
/// domain. Instead of using single values, we use a bounded set of cadidates to
/// increase precision.

// Forward declare the IDETabulationProblem as we require its toString
// functionality.
template <typename N, typename D, typename F, typename T, typename V,
          typename L, typename I>
class IDETabulationProblem;

class IDEGeneralizedLCA
    : public IDETabulationProblem<const llvm::Instruction *,
                                  const llvm::Value *, const llvm::Function *,
                                  const llvm::StructType *, const llvm::Value *,
                                  EdgeValueSet, LLVMBasedICFG> {
  size_t maxSetSize;

public:
  typedef const llvm::Value *d_t;
  typedef const llvm::Instruction *n_t;
  typedef const llvm::Function *f_t;
  typedef const llvm::StructType *t_t;
  typedef const llvm::Value *v_t;
  typedef LLVMBasedICFG &i_t;
  typedef EdgeValueSet l_t;

  struct LCAResult {
    LCAResult() = default;
    unsigned line_nr = 0;
    std::string src_code;
    std::map<std::string, l_t> variableToValue;
    std::vector<n_t> ir_trace;
    void print(std::ostream &os);
  };

  typedef std::map<std::string, std::map<unsigned, LCAResult>> lca_results_t;

  IDEGeneralizedLCA(
      const ProjectIRDB *IRDB,
      const TypeHierarchy<const llvm::StructType *, const llvm::Function *> *TH,
      const LLVMBasedICFG *ICF,
      const PointsToInfo<const llvm::Value *, const llvm::Instruction *> *PT,
      std::set<std::string> EntryPoints, size_t MaxSetSize);

  std::shared_ptr<FlowFunction<d_t>> getNormalFlowFunction(n_t curr,
                                                           n_t succ) override;

  std::shared_ptr<FlowFunction<d_t>> getCallFlowFunction(n_t callStmt,
                                                         f_t destMthd) override;

  std::shared_ptr<FlowFunction<d_t>> getRetFlowFunction(n_t callSite,
                                                        f_t calleeMthd,
                                                        n_t exitStmt,
                                                        n_t retSite) override;

  std::shared_ptr<FlowFunction<d_t>>
  getCallToRetFlowFunction(n_t callSite, n_t retSite,
                           std::set<f_t> callees) override;

  std::shared_ptr<FlowFunction<d_t>>
  getSummaryFlowFunction(n_t callStmt, f_t destMthd) override;

  std::map<n_t, std::set<d_t>> initialSeeds() override;

  d_t createZeroValue() const override;

  bool isZeroValue(d_t d) const override;

  // in addition provide specifications for the IDE parts

  std::shared_ptr<EdgeFunction<l_t>>
  getNormalEdgeFunction(n_t curr, d_t currNode, n_t succ,
                        d_t succNode) override;

  std::shared_ptr<EdgeFunction<l_t>> getCallEdgeFunction(n_t callStmt,
                                                         d_t srcNode,
                                                         f_t destinationMethod,
                                                         d_t destNode) override;

  std::shared_ptr<EdgeFunction<l_t>>
  getReturnEdgeFunction(n_t callSite, f_t calleeMethod, n_t exitStmt,
                        d_t exitNode, n_t reSite, d_t retNode) override;

  std::shared_ptr<EdgeFunction<l_t>>
  getCallToRetEdgeFunction(n_t callSite, d_t callNode, n_t retSite,
                           d_t retSiteNode, std::set<f_t> callees) override;

  std::shared_ptr<EdgeFunction<l_t>>
  getSummaryEdgeFunction(n_t callStmt, d_t callNode, n_t retSite,
                         d_t retSiteNode) override;

  l_t topElement() override;

  l_t bottomElement() override;

  l_t join(l_t lhs, l_t rhs) override;

  std::shared_ptr<EdgeFunction<l_t>> allTopFunction() override;

  void printNode(std::ostream &os, n_t n) const override;

  void printDataFlowFact(std::ostream &os, d_t d) const override;

  void printFunction(std::ostream &os, f_t m) const override;

  void printEdgeFact(std::ostream &os, l_t v) const override;

  // void printIDEReport(std::ostream &os,
  // SolverResults<n_t, d_t, l_t> &SR) override;
  void emitTextReport(const SolverResults<n_t, d_t, l_t> &SR,
                      std::ostream &os) override;

  lca_results_t getLCAResults(SolverResults<n_t, d_t, l_t> SR);

private:
  void stripBottomResults(std::unordered_map<d_t, l_t> &res);
  bool isEntryPoint(const std::string &name) const;
  template <typename V> std::string VtoString(V v);
  bool isSpecialMemberFunction(const llvm::Function *F);
};

} // namespace psr

#endif
