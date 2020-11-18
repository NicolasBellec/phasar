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
#include "llvm/IR/Function.h"

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
    : InterMonoProblem<InterMonoFullConstantPropagationAnalysisDomain>(
          IRDB, TH, ICF, PT, std::move(EntryPoints)) {
            std::cout << "=== Point-to info ===\n";
            PT->print();
            std::cout << "=== Point-to info ===\n";

          }

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::join(
    const BitVectorSet<InterMonoMemReachDef::d_t> &Lhs,
    const BitVectorSet<InterMonoMemReachDef::d_t> &Rhs) {
  return Lhs.setUnion(Rhs);
}

bool InterMonoMemReachDef::sqSubSetEqual(
    const BitVectorSet<InterMonoMemReachDef::d_t> &Lhs,
    const BitVectorSet<InterMonoMemReachDef::d_t> &Rhs) {
  return Rhs.includes(Lhs);
}

std::unordered_map<InterMonoMemReachDef::n_t,
                   BitVectorSet<InterMonoMemReachDef::d_t>>
InterMonoMemReachDef::initialSeeds() {
  // TODO Do implementation
  cout << "InterMonoSolverTest::initialSeeds()\n";
  const llvm::Function *Main = ICF->getFunction("main");
  unordered_map<const llvm::Instruction *, BitVectorSet<InterMonoMemReachDef::d_t>>
      Seeds;
  Seeds.insert(
      make_pair(&Main->front().front(), BitVectorSet<InterMonoMemReachDef::d_t>()));
  return Seeds;
  // return {};
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::normalFlow(
    InterMonoMemReachDef::n_t S,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  // TODO finish implementation
  BitVectorSet<InterMonoMemReachDef::d_t> Out(In);
  cout << "normalFlow : " << llvmIRToString(S) << "\n";

  switch (S->getOpcode()) {
    case llvm::Instruction::Store: {
      const llvm::StoreInst* Store = llvm::cast<llvm::StoreInst>(S);
      std::cout << "found store : " << llvmIRToString(Store) << "\n";

      const llvm::Value *target = Store->getPointerOperand();
      auto pts = PT->getPointsToSet(target);

      // Iterate through the point-to set to add the store identifier
      for ( const auto *point_to : *pts ) {
        /*
        We do not kill any variable (an exception could be done for values
        that are in the same function thus allowing us to test the real aliasing)
        */

        int64_t id = std::stol(getMetaDataID(Store));
        cout << "Generating ( " << llvmIRToString(point_to) << ", " << id << " )\n";
        Out.insert(std::pair<const llvm::Value *, LatticeDomain<plain_d_t>>(point_to, id));
      }

    } break;
    case llvm::Instruction::Load: {
      // const llvm::LoadInst* Load = llvm::cast<llvm::LoadInst>(S);
      // cout << "found load : " << llvmIRToString(Load) << "\n";
      // Is there something special to do ?
    } // break;
    default: {}
  }

  return Out;

  // if (const llvm::StoreInst *Store = llvm::dyn_cast<llvm::StoreInst>(S)) {
  //   const llvm::Value *target = Store->getPointerOperand();
  //   cout << "found store : " << llvmIRToString(Store) << "\n";
  //   for ( const auto *point_to : *(PT->getPointsToSet(target)) ) {
  //     cout << " Found point-to : " << llvmIRToString(point_to) << "\n";
  //     for ( const auto &pair : In ) {
  //       if ( pair.first == point_to && PT->alias(pair.first, point_to) == AliasResult::MustAlias ) {
  //         cout << "Killing ( " << llvmIRToString(pair.first) << ", " << pair.second << " )\n";
  //       } else {
  //         cout << "Keeping ( " << llvmIRToString(pair.first) << ", " << pair.second << " )\n";
  //         Out.insert(std::pair(pair));
  //       }
  //     }
  //   //
  //     int64_t id = 0;
  //     cout << "Generating ( " << llvmIRToString(point_to) << ", " << id << " )\n";
  //     Out.insert(std::pair<const llvm::Value *, LatticeDomain<plain_d_t>>(point_to, id));
  //   }
  // } else {
  //   Out = In;
  // }
  // return Out;
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::callFlow(
    InterMonoMemReachDef::n_t CallSite,
    InterMonoMemReachDef::f_t Callee,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  // TODO implement
  std::cout << "InterMonoMemReachDef::callFlow" << "\n";
  auto Out(In);

  if ( const auto* Call = llvm::dyn_cast<llvm::CallBase>(CallSite) ) {
    for ( unsigned arg_n = 0; arg_n < Call->getNumArgOperands(); arg_n++ ) {
      const auto* caller_arg = Call->getArgOperand( arg_n );
      const auto* callee_arg = Callee->getArg( arg_n );
      std::cout << "\targument(" << arg_n << ") : " << llvmIRToString(caller_arg) << "\n";
      std::cout << "\t\tmapped to " << llvmIRToString(callee_arg) << "\n";

      if ( caller_arg->getType()->isPointerTy() ) {
        // Search for current facts about the argument sent
        // and map it to the argument of the function
        for ( const auto &fact : In ) {
          if ( fact.first == caller_arg ) {
            Out.insert(std::make_pair(callee_arg, fact.second));
          }
        }
      }
    }
  } else {
    std::cout << "\tFailed to handle : " << llvmIRToString(CallSite) << "\n";
  }
  return Out;
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::returnFlow(
    InterMonoMemReachDef::n_t CallSite,
    InterMonoMemReachDef::f_t Callee,
    InterMonoMemReachDef::n_t ExitStmt,
    InterMonoMemReachDef::n_t RetSite,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  std::cout << "InterMonoMemReachDef::returnFlow" << "\n";
  auto Out(In);

  if ( const auto* Call = llvm::dyn_cast<llvm::CallBase>(CallSite) ) {

  } else {
    std::cout << "\tFailed to handle : " << llvmIRToString(CallSite) << "\n";
  }

  return Out;
}

BitVectorSet<InterMonoMemReachDef::d_t>
InterMonoMemReachDef::callToRetFlow(
    InterMonoMemReachDef::n_t CallSite,
    InterMonoMemReachDef::n_t RetSite,
    std::set<InterMonoMemReachDef::f_t> Callees,
    const BitVectorSet<InterMonoMemReachDef::d_t> &In) {
  // TODO implement
  // std::cout << "InterMonoMemReachDef::callToRetFlow -- Unimplemented" << "\n";
  // std::cout << "Triggered by " << llvmIRToString(CallSite) << "\n\t--> " << llvmIRToString(RetSite) << "\n";
  return In;
}

void InterMonoMemReachDef::printNode(
    std::ostream &OS, InterMonoMemReachDef::n_t N) const {
  OS << llvmIRToString(N);
}

void InterMonoMemReachDef::printDataFlowFact(
    std::ostream &OS, InterMonoMemReachDef::d_t D) const {
  OS << "\t< " + llvmIRToString(D.first) << ", ";
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
  OS << " >\n";
}

void InterMonoMemReachDef::printFunction(
    std::ostream &OS, InterMonoMemReachDef::f_t F) const {
  OS << F->getName().str();
}

} // namespace psr
