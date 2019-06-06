/*
 * LLVMGlobals.hpp
 *
 *  Created on: May 21, 2019
 *      Author: 26sra
 */

#ifndef EKCC_LLVMGLOBALS_HPP_
#define EKCC_LLVMGLOBALS_HPP_

#include <string>
#include <memory>
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Target/TargetMachine.h"

using std::unique_ptr;
using std::string;

static llvm::LLVMContext GlobalContext;
static llvm::IRBuilder<> GlobalBuilder(GlobalContext);
static llvm::Module * GlobalModule;
static unique_ptr<llvm::Module> GlobalModuleUPtr;
static std::unique_ptr<llvm::TargetMachine> GlobalTargetMachine;

static llvm::AllocaInst *
CreateEntryBlockAllocaFloat(llvm::Function * function, const string &varName) {
    llvm::IRBuilder<> temp(&function->getEntryBlock(), function->getEntryBlock().begin());
    return temp.CreateAlloca(llvm::Type::getFloatTy(GlobalContext), nullptr, varName);
}

static llvm::AllocaInst *
CreateEntryBlockAllocaInt(llvm::Function * function, const string &varName) {
    llvm::IRBuilder<> temp(&function->getEntryBlock(), function->getEntryBlock().begin());
    return temp.CreateAlloca(llvm::Type::getInt32Ty(GlobalContext), nullptr, varName);
}

static llvm::AllocaInst *
CreateEntryBlockAllocaBool(llvm::Function * function, const string &varName) {
    llvm::IRBuilder<> temp(&function->getEntryBlock(), function->getEntryBlock().begin());
    return temp.CreateAlloca(llvm::Type::getInt1Ty(GlobalContext), nullptr, varName);
}

#endif /* EKCC_LLVMGLOBALS_HPP_ */
