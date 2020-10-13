/******************************************************************************
 * Copyright (c) 2020 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Nicolas Bellec
 *****************************************************************************/

#include "llvm/Support/Casting.h"
#include <algorithm>
#include <ostream>

#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

#include "phasar/DB/ProjectIRDB.h"
#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/DataFlowSolver/Mono/Problems/InterMonoMemReachDef.h"
#include "phasar/PhasarLLVM/Pointer/LLVMPointsToInfo.h"
#include "phasar/PhasarLLVM/TypeHierarchy/LLVMTypeHierarchy.h"
#include "phasar/Utils/BitVectorSet.h"
#include "phasar/Utils/LLVMShorthands.h"
#include <unordered_map>
#include <utility>

using namespace std;
using namespace psr;

namespace psr {

InterMonoMemReachDef::InterMonoMemReachDef(
    const ProjectIRDB *IRDB, const LLVMTypeHierarchy *TH,
    const LLVMBasedICFG *ICF, LLVMPointsToInfo *PT,
    std::set<std::string> EntryPoints)
    : InterMonoProblem<InterMonoMemReachDefAnalysisDomain>(
          IRDB, TH, ICF, PT, std::move(EntryPoints)) {}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::join(
    const BitVectorSet<InterMonoMemReachDef::d_t> &Lhs,
    const BitVectorSet<InterMonoMemReachDef::d_t> &Rhs) {
  return Lhs.setUnion(Rhs);
}

bool InterMonoMemReachDef::sqSubSetEqual(
    const BitVectorSet<InterMonoMemReachDef::d_t> &Lhs,
    const BitVectorSet<InterMonoMemReachDef::d_t> &Rhs) {
  return Lhs.includes(Rhs);
}

std::unordered_map<InterMonoMemReachDef::n_t,
                   BitVectorSet<InterMonoMemReachDef::d_t>>
InterMonoMemReachDef::initialSeeds() {
  // TODO Do implementation
  // std::unordered_map<InterMonoMemReachDef::n_t,
  //                    BitVectorSet<InterMonoMemReachDef::d_t>>
  //     Seeds;
  // for (const auto &EntryPoint : EntryPoints) {
  //   if (const auto *Fun = IRDB->getFunctionDefinition(EntryPoint)) {
  //     auto Is = ICF->getStartPointsOf(Fun);
  //     for (const auto *I : Is) {
  //       Seeds[I] = {};
  //     }
  //   }
  // }
  // return Seeds;
  return {};
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::normalFlow(
    InterMonoMemReachDef::n_t S,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  // TODO finish implementation
  BitVectorSet<InterMonoMemReachDef::d_t> Out;
  // if (const auto *Alloc = llvm::dyn_cast<llvm::AllocaInst>(S)) {
  //     Out.insert({Alloc, Top{}});
  // }
  if (const auto *Store = llvm::dyn_cast<llvm::StoreInst>(S)) {
    // llvm::Value *target = Store->getPointerOperand();


    // for ( const auto *point_to : *(PT->getPointsToSet(Store)) ) {
    //   for ( const auto pair : In ) {
    //     if ( pair.first == point_to && PT->alias(pair.first, point_to) == AliasResult::MustAlias ) {
    //       cout << "Killing ( " << llvmIRToString(pair.first) << ", " << pair.second << " )\n";
    //     } else {
    //       cout << "Keeping ( " << llvmIRToString(pair.first) << ", " << pair.second << " )\n";
    //       Out.insert(std::pair(pair));
    //     }
    //   }
    //
    //   int64_t id = 0;
    //   cout << "Generating ( " << llvmIRToString(point_to) << ", " << id << " )\n";
    //   Out.insert(std::pair<const llvm::Value *, LatticeDomain<plain_d_t>>(point_to, id));
    // }
  }
  return Out;
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::callFlow(
    InterMonoMemReachDef::n_t CallSite,
    InterMonoMemReachDef::f_t Callee,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  // TODO implement
  // return In;
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::returnFlow(
    InterMonoMemReachDef::n_t CallSite,
    InterMonoMemReachDef::f_t Callee,
    InterMonoMemReachDef::n_t ExitStmt,
    InterMonoMemReachDef::n_t RetSite,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  // TODO implement
  // return In;
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::callToRetFlow(
    InterMonoMemReachDef::n_t CallSite,
    InterMonoMemReachDef::n_t RetSite,
    std::set<InterMonoMemReachDef::f_t> Callees,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  // TODO implement
  // return In;
}

void InterMonoMemReachDef::printNode(
    std::ostream &OS, InterMonoMemReachDef::n_t N) const {
  OS << llvmIRToString(N);
}

void InterMonoMemReachDef::printDataFlowFact(
    std::ostream &OS, InterMonoMemReachDef::d_t D) const {
  OS << "< " + llvmIRToString(D.first) << ", ";
  if (std::holds_alternative<Top>(D.second)) {
    OS << std::get<Top>(D.second);
  }
  if (std::holds_alternative<Bottom>(D.second)) {
    OS << std::get<Bottom>(D.second);
  }
  if (std::holds_alternative<InterMonoMemReachDef::plain_d_t>(
          D.second)) {
    OS << std::get<InterMonoMemReachDef::plain_d_t>(D.second);
  }
  OS << " >";
}

void InterMonoMemReachDef::printFunction(
    std::ostream &OS, InterMonoMemReachDef::f_t F) const {
  OS << F->getName().str();
}

} // namespace psr
