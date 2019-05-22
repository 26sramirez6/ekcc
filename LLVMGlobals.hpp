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

static llvm::LLVMContext GlobalContext;
static llvm::IRBuilder<> GlobalBuilder(GlobalContext);
static llvm::Module * GlobalModule;


#endif /* EKCC_LLVMGLOBALS_HPP_ */
