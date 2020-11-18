/******************************************************************************
 * Copyright (c) 2020 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Nicolas Bellec
 *****************************************************************************/

#ifndef PHASAR_PHASARLLVM_MONO_PROBLEMS_INTERMONOMEMREACHDEF_H_
#define PHASAR_PHASARLLVM_MONO_PROBLEMS_INTERMONOMEMREACHDEF_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/DataFlowSolver/Mono/InterMonoProblem.h"
#include "phasar/PhasarLLVM/Domain/AnalysisDomain.h"
#include "phasar/PhasarLLVM/Utils/LatticeDomain.h"
#include "phasar/Utils/BitVectorSet.h"

namespace llvm {
class Value;
class Instruction;
class Function;
class StructType;
} // namespace llvm

namespace psr {

class ProjectIRDB;
class LLVMTypeHierarchy;
class LLVMPointsToInfo;

struct InterMonoMemReachDefDomain
    : public LLVMAnalysisDomainDefault {
  using plain_d_t = llvm::Metadata *;
  using d_t = std::pair<const llvm::Value *, plain_d_t>;
  using i_t = LLVMBasedICFG;
};


class InterMonoMemReachDef
    : public InterMonoProblem<InterMonoMemReachDefDomain> {
public:
  using plain_d_t =
      typename InterMonoMemReachDefDomain::plain_d_t;

  InterMonoMemReachDef(const ProjectIRDB *IRDB,
                      const LLVMTypeHierarchy *TH,
                      const LLVMBasedICFG *ICF,
                      LLVMPointsToInfo *PT,
                      std::set<std::string> EntryPoints = {});

  ~InterMonoMemReachDef() override = default;

  BitVectorSet<d_t> join(const BitVectorSet<d_t> &Lhs,
                         const BitVectorSet<d_t> &Rhs) override;

  bool sqSubSetEqual(const BitVectorSet<d_t> &Lhs,
                     const BitVectorSet<d_t> &Rhs) override;

  std::unordered_map<n_t, BitVectorSet<d_t>> initialSeeds() override;

  BitVectorSet<d_t> normalFlow(n_t S, const BitVectorSet<d_t> &In) override;

  BitVectorSet<d_t> callFlow(n_t CallSite, f_t Callee,
                             const BitVectorSet<d_t> &In) override;

  BitVectorSet<d_t> returnFlow(n_t CallSite, f_t Callee, n_t ExitStmt,
                               n_t RetSite,
                               const BitVectorSet<d_t> &In) override;

  BitVectorSet<d_t> callToRetFlow(n_t CallSite, n_t RetSite,
                                  std::set<f_t> Callees,
                                  const BitVectorSet<d_t> &In) override;

  void printNode(std::ostream &os, n_t n) const override;

  void printDataFlowFact(std::ostream &os, d_t d) const override;

  void printFunction(std::ostream &os, f_t f) const override;
};

} // namespace psr

namespace std {
  template <>
  struct hash<std::pair<
      const llvm::Value *,
      llvm::Metadata *>> {
    size_t operator()(const std::pair<const llvm::Value *,
                                      llvm::Metadata *> &P) const {
      std::hash<const llvm::Value *> hash_ptr;
      std::hash<llvm::Metadata *> hash_metadata;
      size_t hp = hash_ptr(P.first);
      size_t hu = hash_metadata(P.second);

      return hp ^ (hu << 1);
    }
  };
}

#endif
