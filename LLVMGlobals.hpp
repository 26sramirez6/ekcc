/*
 * LLVMGlobals.hpp
 *
 *  Created on: May 21, 2019
 *      Author: 26sra
 */

#ifndef EKCC_LLVMGLOBALS_HPP_
#define EKCC_LLVMGLOBALS_HPP_

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <string>

using std::string;

static llvm::LLVMContext GlobalContext;
static llvm::IRBuilder<> GlobalBuilder(GlobalContext);
static llvm::Module * GlobalModule;

// static llvm::AllocaInst * CreateEntryBlockAlloca_float(llvm::Function * TheFunction, const string &VarName) {
//     llvm::IRBuilder<> Temp(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
//     return Temp.CreateAlloca(llvm::Type::getFloatTy(GlobalContext), nullptr, VarName);
// }

// static llvm::AllocaInst * CreateEntryBlockAlloca_int(llvm::Function * TheFunction, const string &VarName) {
//     llvm::IRBuilder<> Temp(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
//     return Temp.CreateAlloca(llvm::Type::getInt32Ty(GlobalContext), nullptr, VarName);
// }

// static llvm::AllocaInst * CreateEntryBlockAlloca_bool(llvm::Function * TheFunction, const string &VarName) {
//     llvm::IRBuilder<> Temp(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
//     return Temp.CreateAlloca(llvm::Type::getInt1Ty(GlobalContext), nullptr, VarName);
// }

// static llvm::AllocaInst * CreateEntryBlockAlloca_floatPtr(llvm::Function * TheFunction, const string &VarName) {
//     llvm::IRBuilder<> Temp(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
//     return Temp.CreateAlloca(llvm::Type::getFloatPtrTy(GlobalContext), nullptr, VarName);
// }

// static llvm::AllocaInst * CreateEntryBlockAlloca_intPtr(llvm::Function * TheFunction, const string &VarName) {
//     llvm::IRBuilder<> Temp(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
//     return Temp.CreateAlloca(llvm::Type::getInt32PtrTy(GlobalContext), nullptr, VarName);
// }

// static llvm::AllocaInst * CreateEntryBlockAlloca_boolPtr(llvm::Function * TheFunction, const string &VarName) {
//     llvm::IRBuilder<> Temp(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
//     return Temp.CreateAlloca(llvm::Type::getInt1PtrTy(GlobalContext), nullptr, VarName);
// }


#endif /* EKCC_LLVMGLOBALS_HPP_ */
