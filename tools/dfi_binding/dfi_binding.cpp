/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Nicolas Bellec
 *****************************************************************************/

#include <fstream>
#include <iostream>

#include "boost/filesystem/operations.hpp"

#include "phasar/DB/ProjectIRDB.h"
#include "phasar/PhasarLLVM/AnalysisStrategy/WholeProgramAnalysis.h"
#include "phasar/PhasarLLVM/ControlFlow/LLVMBasedICFG.h"
#include "phasar/PhasarLLVM/Pointer/LLVMPointsToSet.h"
#include "phasar/PhasarLLVM/TypeHierarchy/LLVMTypeHierarchy.h"
#include "phasar/PhasarLLVM/DataFlowSolver/Mono/Solver/InterMonoSolver.h"
#include "phasar/PhasarLLVM/DataFlowSolver/Mono/Problems/InterMonoMemReachDef.h"
#include "phasar/Utils/Logger.h"
#include "phasar/Utils/BitVectorSet.h"
#include "phasar/Config/Configuration.h"

namespace llvm {
class Value;
class Function;
class Module;
class BasicBlock;
class Instruction;
class LoadInst;
class StoreInst;
} // namespace llvm

using namespace psr;
using namespace std;
using namespace llvm;

template <typename AnalysisDomainTy, unsigned K> class MemReachDefInterMonoSolver
  : public InterMonoSolver<AnalysisDomainTy, K> {
public:
  MemReachDefInterMonoSolver(InterMonoProblem<AnalysisDomainTy> &IMP)
      : InterMonoSolver<AnalysisDomainTy, K>(IMP) {}

  // MemReachDefInterMonoSolver(const MemReachDefInterMonoSolver &) = delete;
  // MemReachDefInterMonoSolver &operator=(const MemReachDefInterMonoSolver &) = delete;
  // MemReachDefInterMonoSolver(MemReachDefInterMonoSolver &&) = delete;
  // MemReachDefInterMonoSolver &operator=(MemReachDefInterMonoSolver &&) = delete;
  // virtual ~MemReachDefInterMonoSolver() = default;

  void emitModuleWithResults(llvm::Module* M) {
    unsigned long store_id = 0;
    unsigned long load_id = 0;
    for (auto &F : *M) {
      for (auto &BB : F) {
        for ( auto &I : BB ) {
          if ( llvm::LoadInst *L = llvm::dyn_cast<llvm::LoadInst>(&I) ) {
            auto &ContextMap = this->Analysis[L];

            if (ContextMap.empty()) {
              std::cout << "Analysis return an empty context map for ";
              llvm::outs() << *L << "\n";
            }

            auto &C = L->getContext();
            std::set<llvm::Metadata *> dfi_store_set;

            for (auto &[Context, FlowFacts] : ContextMap) {
              for (auto FlowFact : FlowFacts) {
                if ( FlowFact.first == L->getPointerOperand() ) {
                  dfi_store_set.insert(FlowFact.second);
                }
              }
            }

            auto *metadata_id = L->getMetadata("dfi_load_id");

            if ( !metadata_id ) {
              std::cout << "Error finding dfi_load_id\n";
              return;
            }

            auto *load_id = dyn_cast<ConstantAsMetadata>(
                metadata_id->getOperand(0).get()
              );

            std::vector<llvm::Metadata *> metadataVec;

            metadataVec.push_back(
              metadata_id
            );
            metadataVec.push_back(
              MDString::get(C, "dfi_load_set")
            );

            metadataVec.insert(metadataVec.end(), dfi_store_set.begin(), dfi_store_set.end());

            llvm::ArrayRef<llvm::Metadata *> metadatas(metadataVec);


            L->setMetadata("dfi_load_set",
              MDNode::get(C, metadatas)
            );
          }
        }
      }
    }
  }
};

template <typename Problem, unsigned K>
using MemReachDefInterMonoSolver_P =
    MemReachDefInterMonoSolver<typename Problem::ProblemAnalysisDomain, K>;

class MemReachWPA : public WholeProgramAnalysis<MemReachDefInterMonoSolver_P<InterMonoMemReachDef, 3>, InterMonoMemReachDef> {
public:
  MemReachWPA(ProjectIRDB &IRDB,
                std::set<std::string> EntryPoints = {},
                PointerAnalysisTy *PointerInfo = nullptr,
                CallGraphAnalysisTy *CallGraph = nullptr,
                TypeHierarchyTy *TypeHierarchy = nullptr)
    : WholeProgramAnalysis(IRDB, EntryPoints, PointerInfo, CallGraph, TypeHierarchy) {}

  void erasePSRMetadata() {
    // Erase the metadata placed by PhASAR, reducing the cost of eye-ball debugging :)
    // Warning : Does not remove metadata other than on instructions for now
    for (auto &F : *(IRDB.getWPAModule())) {
      for (auto &BB : F) {
        for (auto &I : BB) {
          I.setMetadata(PhasarConfig::MetaDataKind(), nullptr);
        }
      }
    }
  }

  void emitModuleWithResults() {
    erasePSRMetadata();
    DataFlowSolver.emitModuleWithResults(IRDB.getWPAModule());
  }
};

int main(int argc, const char **argv) {
  if (argc < 3 || !boost::filesystem::exists(argv[1]) ||
      boost::filesystem::is_directory(argv[1]) ||
      boost::filesystem::is_directory(argv[2])) {
    std::cerr << "dfi_binding\n"
                 "Usage: myphasartool <LLVM IR file> <target>\n";
    return 1;
  }
  initializeLogger(false);
  ProjectIRDB DB({argv[1]});
  if (llvm::Module *M = DB.getWPAModule()) {
    auto EntryPoints = std::set<std::string>({"main"});

    LLVMPointsToSet PT(DB, /* UseLazyEvaluation = */ false);

    LLVMTypeHierarchy TH(DB);
    LLVMBasedICFG ICF(DB,
      CallGraphAnalysisType::OTF,
      EntryPoints, &TH, &PT);

    MemReachWPA WPA(DB, EntryPoints, &PT, &ICF, &TH);
    WPA.solve();
    WPA.releaseAllHelperAnalyses();

    WPA.emitModuleWithResults();

    std::error_code EC;
    llvm::raw_fd_ostream file(argv[2], EC);
    M->print(file, nullptr);

  } else {
    std::cerr << "error: file does not contain a 'main' function!\n";
  }
  return 0;
}
