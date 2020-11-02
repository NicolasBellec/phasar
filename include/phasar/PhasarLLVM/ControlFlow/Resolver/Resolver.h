/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/

/*
 * Resolver.h
 *
 *  Created on: 20.07.2018
 *      Author: nicolas bellec
 */

#ifndef PHASAR_PHASARLLVM_CONTROLFLOW_RESOLVER_RESOLVER_H_
#define PHASAR_PHASARLLVM_CONTROLFLOW_RESOLVER_RESOLVER_H_

#include <set>
#include <string>

namespace llvm {
class Instruction;
class CallBase;
class Function;
class StructType;
} // namespace llvm

namespace psr {
class ProjectIRDB;
class LLVMTypeHierarchy;

int getVFTIndex(llvm::AbstractCallSite CS);

const llvm::StructType *getReceiverType(llvm::AbstractCallSite CS);

std::string getReceiverTypeName(const llvm::CallBase &CB);

class Resolver {
protected:
  ProjectIRDB &IRDB;
  LLVMTypeHierarchy *TH;

  Resolver(ProjectIRDB &IRDB);

  const llvm::Function *getNonPureVirtualVFTEntry(const llvm::StructType *T,
                                                  unsigned Idx,
                                                  llvm::AbstractCallSite CS);

public:
  Resolver(ProjectIRDB &IRDB, LLVMTypeHierarchy &TH);

  virtual ~Resolver() = default;

  virtual void preCall(const llvm::Instruction *Inst);

  virtual void
  handlePossibleTargets(llvm::AbstractCallSite CS,
                        std::set<const llvm::Function *> &PossibleTargets);

  virtual void postCall(const llvm::Instruction *Inst);

  virtual std::set<const llvm::Function *>
  resolveVirtualCall(llvm::AbstractCallSite CS) = 0;

  virtual std::set<const llvm::Function *>
  resolveFunctionPointer(llvm::AbstractCallSite CS);

  virtual void otherInst(const llvm::Instruction *Inst);
};
} // namespace psr

#endif
