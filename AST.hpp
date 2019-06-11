/*
 * AST.hpp
 *
 *  Created on: Apr 29, 2019
 *      Author: 26sra
 */

#ifndef EKCC_AST_HPP_
#define EKCC_AST_HPP_
#include<vector>
#include<iostream>
#include<string>
#include<utility>
#include<sstream>
#include<unordered_map>
#include<unordered_set>
#include<tuple>
#include<memory>
#include<assert.h>
#include<stdlib.h>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include <llvm/ADT/Triple.h>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Verifier.h"
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LegacyPassNameParser.h>

#include <llvm/IRReader/IRReader.h>

#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/CallGraphSCCPass.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Analysis/RegionPass.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>

#include <llvm/Bitcode/BitcodeWriterPass.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/Config/llvm-config.h>
// #include <llvm/ExecutionEngine/JIT.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>


#include <llvm/IRReader/IRReader.h>
#include <llvm/InitializePasses.h>
#include <llvm/MC/SubtargetFeature.h>

#include <llvm/Support/Debug.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/InitLLVM.h>
#include "llvm/Support/CommandLine.h"
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/SystemUtils.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Support/YAMLTraits.h>
// #include <llvm/Support/PluginLoader.h>
//CommandLine Error: Option 'load' registered more than once!
//LLVM ERROR: inconsistency in registered CommandLine options

#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/Coroutines.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "llvm-c/Types.h"
#include "llvm/Object/ObjectFile.h"


#include "ValidTypes.hpp"
#include "LLVMGlobals.hpp"

using std::vector;
using std::string;
using std::to_string;
using std::move;
using std::stringstream;
using std::unordered_map;
using std::tuple;
using std::get;
using std::unique_ptr;
using std::unordered_set;

typedef unordered_map<string, tuple<ValidType *, llvm::AllocaInst *>> VarTable;
typedef unordered_map<string, tuple<ValidType *, llvm::AllocaInst * >>::const_iterator VarTableEntry;
typedef unordered_map<string, tuple<vector< ValidType * >, llvm::Function *> > FuncTable;
typedef unordered_map<string, tuple<vector< ValidType * >, llvm::Function *> >::const_iterator FuncTableEntry;
typedef int (*JitFunc)();

struct ASTNode {
	static ASTNode * root_;
	static bool ready_;
	static vector< string > compilerErrors_;
	static vector< int > lineNumberErrors_;
	static FuncTable funcTable_;
	static tuple<string, int> recursiveFuncPlaceHolder_;
	static unordered_set<string> recursiveFuncNames_;
	static llvm::Function * currentLLVMFunctionPrototype_;
	static VarTable varTable_;
	static bool runDefined_;
	static VariableTypes currentFunctionReturnType_;
	static llvm::Function * runFunction_;
	static llvm::Function * printfFunction_;
	static llvm::Function * cintAddFunction_;
	static llvm::Function * cintMultiplyFunction_;
	static llvm::Function * cintDivideFunction_;
	static llvm::Function * cintSubtractFunction_;
	static llvm::Function * cintNegateFunction_;
	vector< ASTNode * > children_;
	unsigned lineNumber_ = 0;
	ValidType * resultType_ = nullptr;

	ASTNode() : lineNumber_(0), resultType_(nullptr) {}
	ASTNode(unsigned lineNumber) : lineNumber_(lineNumber) {}
	ASTNode(ValidType * resultType) : resultType_(resultType) {}
	ASTNode(unsigned lineNumber, ValidType * resultType) :
		lineNumber_(lineNumber), resultType_(resultType) {}

	virtual void
	PrintRecursive(stringstream& ss, unsigned depth) {};

	virtual
	~ASTNode() {
		for (auto n : this->children_) {
			delete n;
			n = nullptr;
		}
		if (this->resultType_) {
			delete this->resultType_;
			this->resultType_ = nullptr;
		}
	}

	virtual llvm::Value *
	GetLLVMValue(string identifier) {
		llvm::Value * ret = nullptr;
		VarTableEntry hit = ASTNode::varTable_.find(identifier);
		if (ASTNode::varTable_.end() != hit) {
			ret = get<1>(hit->second);
		}
		return ret;
	}

	virtual ValidType *
	GetValidType(string identifier) {
		ValidType * ret = nullptr;
		VarTableEntry hit = ASTNode::varTable_.find(identifier);
		if (ASTNode::varTable_.end() != hit) {
			ret = get<0>(hit->second);
		}
		return ret;
	}

	virtual llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		if (this->children_.size() > 0) {
			for (auto n: this->children_) {
				n->GenerateCode(endBlock);
			}
		}
		return nullptr;
	}

	virtual llvm::Value *
	GetLLVMReturnValueRecursive() {
		llvm::Value * ret = nullptr;
		if(this->children_.size()>0) {
			for (auto node : this->children_) {
				ret = node->GetLLVMReturnValueRecursive();
				if (ret != nullptr) {
					return ret;
				}
			}
		}
		return ret;
	}

	static void
	StaticInit(string inputFile) {
		if (!ASTNode::ready_) {
			// Set up for JIT
			std::string HostTriple(llvm::sys::getProcessTriple());
			std::string CPU = llvm::sys::getHostCPUName();

			llvm::StringMap<bool> HostFeatures;
			std::string FeaturesStr;
			if (llvm::sys::getHostCPUFeatures(HostFeatures)) {
				llvm::SubtargetFeatures Features;

				for (auto &F : HostFeatures)
				Features.AddFeature(F.first(), F.second);

				FeaturesStr = Features.getString();
			}

			// construct global module
			GlobalModuleUPtr = unique_ptr<llvm::Module>{new llvm::Module(inputFile, GlobalContext)};
			GlobalModule = GlobalModuleUPtr.get();

			// Note: If you're using the JIT, you'll get the TM from the JIT engine...
			std::string Err;
			const llvm::Target* Target = llvm::TargetRegistry::lookupTarget (HostTriple, Err);
			std::unique_ptr<llvm::TargetMachine> TM(Target->createTargetMachine(HostTriple, CPU, FeaturesStr,
																			llvm::TargetOptions(), llvm::Reloc::PIC_));

			GlobalTargetMachine = move(TM);
			GlobalModule->setDataLayout(GlobalTargetMachine->createDataLayout());
			GlobalModule->setTargetTriple(HostTriple);

			// construct a print function
			vector<llvm::Type *> printfCharPtrParam;
			printfCharPtrParam.push_back(llvm::Type::getInt8PtrTy(GlobalContext));
			llvm::FunctionType * printFType =
				llvm::FunctionType::get(
				llvm::Type::getInt32Ty(GlobalContext),
				printfCharPtrParam, true);

			ASTNode::printfFunction_ = llvm::Function::Create(
				printFType, llvm::Function::ExternalLinkage,
				llvm::Twine("printf"),
				GlobalModule);
			// construct the cint binary operations
			vector<llvm::Type *> cintBinaryParams;
			cintBinaryParams.push_back(llvm::Type::getInt32Ty(GlobalContext));
			cintBinaryParams.push_back(llvm::Type::getInt32Ty(GlobalContext));
			llvm::FunctionType * cintBinaryFType =
				llvm::FunctionType::get(
				llvm::Type::getInt32Ty(GlobalContext),
				cintBinaryParams, false);

			ASTNode::cintAddFunction_ = llvm::Function::Create(
				cintBinaryFType, llvm::Function::ExternalLinkage,
				llvm::Twine("cint_add"),
				GlobalModule);

			ASTNode::cintSubtractFunction_ = llvm::Function::Create(
				cintBinaryFType, llvm::Function::ExternalLinkage,
				llvm::Twine("cint_subtract"),
				GlobalModule);

			ASTNode::cintMultiplyFunction_ = llvm::Function::Create(
				cintBinaryFType, llvm::Function::ExternalLinkage,
				llvm::Twine("cint_multiply"),
				GlobalModule);

			ASTNode::cintDivideFunction_ = llvm::Function::Create(
				cintBinaryFType, llvm::Function::ExternalLinkage,
				llvm::Twine("cint_divide"),
				GlobalModule);

			// construct cint unary operations
			vector<llvm::Type *> cintUnaryParams;
			cintUnaryParams.push_back(llvm::Type::getInt32Ty(GlobalContext));
			llvm::FunctionType * cintUnaryFType =
				llvm::FunctionType::get(
				llvm::Type::getInt32Ty(GlobalContext),
				cintUnaryParams, false);
			ASTNode::cintNegateFunction_ = llvm::Function::Create(
					cintUnaryFType, llvm::Function::ExternalLinkage,
				llvm::Twine("cint_negate"),
				GlobalModule);

			ASTNode::ready_ = true;
		}
	}

	static void
	LogError(unsigned lineNumber, string errorMessage, bool exitFast=false) {
		stringstream ss;
		ss << "error: line " << lineNumber << ": ";
		ss << errorMessage;
		ss << endl;
		ASTNode::compilerErrors_.push_back(ss.str());
		if (exitFast) {
			cout << ASTNode::GetCompilerErrors() << endl;
			exit(1);
		}
	}

	static string
	GetCompilerErrors() {
		string ret = "";
		for (auto e : ASTNode::compilerErrors_) {
			ret += e;
		}
		return ret;
	}

	static bool
	HasCompilerErrors() {
		return ASTNode::compilerErrors_.size() > 0;
	}

};

struct ProgramNode : public ASTNode {
	ProgramNode(unsigned lineNumber, ASTNode * funcsNode) :
		ASTNode(lineNumber) {
		this->children_.push_back(funcsNode);
		ASTNode::root_ = this;
		if (!ASTNode::runDefined_) {
			ASTNode::LogError(lineNumber, "Undefined run function");
		}
	}

	ProgramNode(unsigned lineNumber, ASTNode * externs,
			ASTNode * funcsNode) : ASTNode(lineNumber){
		this->children_.push_back(funcsNode);
		this->children_.push_back(externs);
		ASTNode::root_ = this;
		if (!ASTNode::runDefined_) {
			ASTNode::LogError(lineNumber, "Undefined run function");
		}
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		ss << "---" << '\n';
		ss << "name: prog" << '\n';
		for (auto node : this->children_) {
			node->PrintRecursive(ss, depth+1);
		}
	}

	void
	GenerateCodeRecursive(llvm::raw_string_ostream& ss, bool optimize) {
		ASTNode::GenerateCode(nullptr);

		if (optimize) {
			AddOptimizations();
		}

		// Print IR without optimization
		GlobalModule->print(ss, nullptr);
	}

	void
	AddOptimizations() {

		// To build and run the pass manager...

		llvm::Triple ModuleTriple(GlobalModule->getTargetTriple());

		std::unique_ptr<llvm::legacy::PassManager> MPM(new llvm::legacy::PassManager);
		llvm::TargetLibraryInfoImpl TLII(ModuleTriple);
		MPM->add(new llvm::TargetLibraryInfoWrapperPass(TLII));

		std::unique_ptr<llvm::legacy::FunctionPassManager> FPM(new llvm::legacy::FunctionPassManager(GlobalModule));
		FPM->add(createTargetTransformInfoWrapperPass(GlobalTargetMachine->getTargetIRAnalysis()));

		llvm::PassManagerBuilder PMBuilder;
		PMBuilder.OptLevel = 3;
		PMBuilder.SizeLevel = 0;
		PMBuilder.Inliner = llvm::createFunctionInliningPass(PMBuilder.OptLevel, PMBuilder.SizeLevel, false);
		PMBuilder.LoopVectorize = true;

		GlobalTargetMachine->adjustPassManager(PMBuilder);

		PMBuilder.populateFunctionPassManager(*FPM);
		PMBuilder.populateModulePassManager(*MPM);

		FPM->doInitialization();
		for (llvm::Function &F : *GlobalModule)
			FPM->run(F);
		FPM->doFinalization();

		MPM->run(*GlobalModule);
	}

	void
	ExecuteJIT(int argc, char ** argv){
		string error;
		llvm::EngineBuilder enginebuilder(move(GlobalModuleUPtr));
		llvm::ExecutionEngine * engine = enginebuilder.setErrorStr(&error).create();
		if (!engine) {
			cout << "error: failed to create execution engine" << endl;
			exit(1);  
		}
		llvm::SMDiagnostic diagnostic;
		unique_ptr<llvm::Module> m = parseIRFile("main.ll", diagnostic, GlobalContext);
		engine->addModule(move(m));
		engine->finalizeObject();
		auto mainLLVMFunction = engine->FindFunctionNamed("main");
		int (*mainFunc)(int, char **) = (int(*)(int,char**))engine->getPointerToFunction(mainLLVMFunction);
		exit(mainFunc(argc, argv));
	}
};

struct VdeclNode : public ASTNode {
	ValidType * type_ = nullptr;
	string identifier_;

	virtual ~VdeclNode() {
		if (this->type_ != nullptr) {
			delete this->type_;
			this->type_ = nullptr;
		}
	}

	VdeclNode(unsigned lineNumber, ValidType * type, string identifier) :
		ASTNode(lineNumber), type_(type),
		identifier_(identifier.substr(1, string::npos)) {
		if (ASTNode::varTable_.end() != ASTNode::varTable_.find(identifier_)) {
			ASTNode::LogError(lineNumber,
				string("variable identifier ") +
				identifier +
				string(" already defined"));
		} else if (type->varType_ == VoidVarType){
			ASTNode::LogError(lineNumber,
				string("variable identifier ") +
				identifier +
				string(" cannot be void"));
		} else if (type->varType_==RefVarType) {
			RefType * refType = (RefType *) type;
			if (refType->invalidConstructor_) {
				ASTNode::LogError(lineNumber,
					string("variable identifier ") +
					identifier +
					string(" is a ref and points to a ref."));
			}
		}
		get<0>(ASTNode::varTable_[identifier_]) = type;
	}

	void
	PrintSelf(stringstream & ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		ss << left1 << "vdecl:" << '\n';
		ss << left2 << "node: vdecl" << '\n';
		ss << left2 << "type: " << this->type_->GetName() << '\n';
		ss << left2 << "var: " << this->identifier_ << '\n';
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {

		llvm::Function * parentFunction = GlobalBuilder.GetInsertBlock()->getParent();
		llvm::AllocaInst * allocaInst = nullptr;
		switch (this->type_->varType_) {
		case IntVarType:
			allocaInst = CreateEntryBlockAllocaInt(
					parentFunction, this->identifier_);
			break;
		case FloatVarType:
			allocaInst = CreateEntryBlockAllocaFloat(
					parentFunction, this->identifier_);
			break;
		case BooleanVarType:
			allocaInst = CreateEntryBlockAllocaBool(
					parentFunction, this->identifier_);
			break;
		case CintVarType:
			allocaInst = CreateEntryBlockAllocaInt(
					parentFunction, this->identifier_);
			break;
		default:
			cout << "unhandled case in vdeclnode" << endl;
			break;
		}
		get<0>(ASTNode::varTable_[this->identifier_]) = this->type_;
		get<1>(ASTNode::varTable_[this->identifier_]) = allocaInst;
		return allocaInst;
	}

	void
	PrintRecursive(stringstream & ss, unsigned depth) {
		PrintSelf(ss, depth);
	}
};


struct VdeclsNode : public ASTNode {
	VdeclsNode(unsigned lineNumber, VdeclNode * vdeclNode) :
		ASTNode(lineNumber) {
		this->children_.push_back(vdeclNode);
	}

	VdeclsNode(unsigned lineNumber, VdeclsNode * vdeclsNode,
			VdeclNode * vdeclNode) : ASTNode(lineNumber) {
		for (auto node : vdeclsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(vdeclNode);
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		ss << left1 << "vdecls:" << '\n';
		ss << left2 << "name: vdecls" << '\n';
		ss << left2 << "vars:" << '\n';
		ss << left3 << "-" << '\n';
		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(ss, depth+3);
			if (i!=this->children_.size()-1)
				ss << left3 << "-" << '\n';
		}
	};
};

struct ExistingVarNode: public ASTNode {
	string identifier_;
	ExistingVarNode(unsigned lineNumber, string identifier) :
		ASTNode(lineNumber), identifier_(identifier.substr(1, string::npos)) {

		VarTableEntry hit = ASTNode::varTable_.find(this->identifier_);
		this->resultType_ = get<0>(hit->second);
		if (hit==ASTNode::varTable_.end()) {
			ASTNode::LogError(lineNumber,
				string("variable identifier ") +
				identifier +
				string(" not declared"));
		}
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		ss << left1 << "name: varval" << '\n';
		ss << left1 << "var: " <<
		this->identifier_.substr(1, string::npos) << '\n';
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Value * val = GetLLVMValue(this->identifier_);
		ValidType * vtype = get<0>(ASTNode::varTable_[this->identifier_]);
		if (vtype->varType_==RefVarType) {
			llvm::AllocaInst * alloca = get<1>(ASTNode::varTable_[this->identifier_]);
			llvm::LoadInst * loadInstruction =  GlobalBuilder.CreateLoad(alloca, this->identifier_);
			return GlobalBuilder.CreateLoad(alloca, this->identifier_);
		}
		return GlobalBuilder.CreateLoad(val, this->identifier_);
	}

};


struct ExistingFuncNode: public ASTNode {
	string identifier_;
	ExistingFuncNode(unsigned lineNumber, string identifier) :
	ASTNode(lineNumber), identifier_(identifier){}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		ss << left1 << "name: funccall" << '\n';
		ss << left1 << "globid: " << this->identifier_ << '\n';
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		if ( this->identifier_ == ASTNode::currentLLVMFunctionPrototype_->getName().str() ) {
			return ASTNode::currentLLVMFunctionPrototype_;
		}
		llvm::Function * function = get<1>(ASTNode::funcTable_[this->identifier_]);
		return function;
	}
};


struct TdeclsNode : public ASTNode {
	ValidType * paramType_ = nullptr;
	vector<ValidType *> paramTypes_;
	TdeclsNode(unsigned lineNumber, ValidType * validType):
		ASTNode(lineNumber), paramType_(validType) {
		this->paramTypes_.push_back(validType);
	}

	TdeclsNode(unsigned lineNumber, TdeclsNode * tdeclsNode,
			ValidType * validType):
		 ASTNode(lineNumber), paramType_(validType) {
		for (auto type : tdeclsNode->paramTypes_) {
			this->paramTypes_.push_back(type);
		}
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		ss << left1 << "tdecls:" << '\n';
		ss << left2 << "name: tdecls" << '\n';
		ss << left2 << "types:" << '\n';
		for (auto type: this->paramTypes_) {
			ss << left3 << "- " << type->GetName() << '\n';
		}
	};

	virtual ~TdeclsNode() {
		if (this->paramType_ != nullptr) {
			delete this->paramType_;
			this->paramType_ = nullptr;
		}
	};

};

struct ExternNode : public ASTNode {
	ValidType * retType_ = nullptr;
	TdeclsNode * tdeclsNode_ = nullptr;
	string identifier_;

	ExternNode(unsigned lineNumber,
		ValidType * retType, string identifier) :
		ASTNode(lineNumber), retType_(retType), identifier_(identifier)	 {
		this->Validate(lineNumber,
			retType, identifier);
	}

	ExternNode(unsigned lineNumber,
			ValidType * retType,
			string identifier,
			TdeclsNode * tdeclsNode) :
			ASTNode(lineNumber), retType_(retType),
			tdeclsNode_(tdeclsNode), identifier_(identifier) {
		this->Validate(lineNumber,
			retType, identifier);
		this->children_.push_back(tdeclsNode);
	}

	void
	Validate(unsigned lineNumber,
			ValidType * retType, string identifier) {
		if(retType->varType_== RefVarType) {
			ASTNode::LogError(lineNumber,
				string("function return type can't be ref. \n"));
		}

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);
		if (this->tdeclsNode_!=nullptr) {
			for (auto t : this->tdeclsNode_->paramTypes_) {
				vTypes.push_back(t);
			}
		}

		FuncTableEntry hit = ASTNode::funcTable_.find(identifier);
		if (hit != ASTNode::funcTable_.end()) {
			ASTNode::LogError(lineNumber,
				string("function identifer ") +
				identifier + string("already defined\n"));
		}


		llvm::Type * getArgReturnType = nullptr;
		if (identifier=="arg") {
			getArgReturnType = llvm::Type::getInt32Ty(GlobalContext);

		} else if (identifier=="argf") {
			getArgReturnType = llvm::Type::getFloatTy(GlobalContext);
		} else {
			ASTNode::LogError(lineNumber, "invalid extern function", true);
		}

		vector<llvm::Type *> getArgParam { llvm::Type::getInt32Ty(GlobalContext) };

		llvm::FunctionType * getArgFuncType =
			llvm::FunctionType::get(
			getArgReturnType,
			getArgParam, false);

		llvm::Function * getArgFunc = llvm::Function::Create(
				getArgFuncType, llvm::Function::ExternalLinkage,
				llvm::Twine(identifier),
				GlobalModule);


		get<0>(ASTNode::funcTable_[identifier]) = vTypes;
		get<1>(ASTNode::funcTable_[identifier]) = getArgFunc;
	}

	virtual ~ExternNode() {
		if (this->retType_ != nullptr) {
			delete this->retType_;
			this->retType_ = nullptr;
		}
		if (this->tdeclsNode_ != nullptr) {
			delete this->tdeclsNode_;
			this->tdeclsNode_ = nullptr;
		}
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left = std::string(depth*2, ' ');
		ss << left << "name: extern" << '\n';
		ss << left << "ret_type: " << this->retType_->GetName() << '\n';
		ss << left << "globid: " << this->identifier_ << '\n';
		if (this->children_.size()>0) {
			this->children_[0]->PrintRecursive(ss, depth+1);
		}
	}
};

struct ExternsNode : public ASTNode {

	ExternsNode(unsigned lineNumber,
			ExternNode * externNode) : ASTNode(lineNumber) {
		this->children_.push_back(externNode);
	}

	ExternsNode(unsigned lineNumber, ExternsNode * externsNode,
			ExternNode * externNode): ASTNode(lineNumber) {
		for (auto node : externsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(externNode);
	}

	void
	PrintSelf(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		ss << left1 << "externs:" << '\n';
		ss << left2 << "name: externs" << '\n';
		ss << left2 << "externs:" << '\n';
		ss << left3 << "-" << '\n';
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		PrintSelf(ss, depth);
		string left3 = std::string((depth+1)*2, ' ');
		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(ss, depth+2);
			if (i!=this->children_.size()-1)
				ss << left3 << "-" << '\n';
		}
	}

};


struct UnaryOperationNode: public ASTNode {
	UnaryOperationTypes operationType_;
	UnaryOperationNode(unsigned lineNumber,
			UnaryOperationTypes operationType,
			ASTNode * expressionNode1) :
		ASTNode(lineNumber), operationType_(operationType) {
		if (operationType==Not) {
			this->resultType_ = new BoolType();
		} else {
			this->resultType_ = expressionNode1->resultType_;
		}
		this->children_.push_back(expressionNode1);
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		ss << left1 << "name: uop" << '\n';
		ss << left1 << "op: ";
		switch (this->operationType_) {
		case Not:
			ss << "not" << '\n';
			break;
		case Minus:
			ss << "minus" << '\n';
			break;
#ifndef NO_DEBUG
		case EmptyUnaryOperation:
			assert(false);
			break;
#endif
		}
		this->children_[0]->PrintRecursive(ss, depth+1);
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Value * ret = nullptr;
		llvm::Value * R = this->children_[0]->GenerateCode(endBlock);
		string error = "error: invalid unary operation in GenerateCode";
		this->resultType_ = ValidType::GetUnderlyingType(
			this->children_[0]->resultType_);

		switch (this->operationType_) {
		case Not:
			if (this->resultType_->varType_==BooleanVarType) {
				return GlobalBuilder.CreateNot(R, "not");
			} else {
				cout << error << endl;
				exit(1);
			}
			break;
		case Minus:
			switch (this->resultType_->varType_) {
			case IntVarType:
			case FloatVarType:
				return GlobalBuilder.CreateNeg(R, "negate");
			case CintVarType:
			{
				vector<llvm::Value *> params { R };
				return GlobalBuilder.CreateCall(
					ASTNode::cintNegateFunction_, params, "cint_negate");
			}
			default:
				cout << error << endl;
				exit(1);
			}
		}

	}
};

struct BinaryOperationNode: public ASTNode {
	BinaryOperationTypes operationType_;
	ValidType * castTo_ = nullptr;
	ValidType * castFrom_ = nullptr;

	BinaryOperationNode(unsigned lineNumber,
			BinaryOperationTypes operationType,
			ASTNode * expressionNode1,
			ASTNode * expressionNode2) :
		ASTNode(lineNumber), operationType_(operationType) {
		this->children_.push_back(expressionNode1);
		this->children_.push_back(expressionNode2);
		SetResultType();
	}

	BinaryOperationNode(unsigned lineNumber,
			BinaryOperationTypes operationType,
			ValidType * validType,
			ASTNode * expressionNode1) :
		ASTNode(lineNumber) , operationType_(Cast),
		castTo_(validType) {
		this->children_.push_back(expressionNode1);
		this->castFrom_ = expressionNode1->resultType_;
		SetResultType();
	}

	virtual ~BinaryOperationNode() {
		if (this->castTo_ != nullptr) {
			delete this->castTo_;
			this->castTo_ = nullptr;
		}
	}

	void
	SetResultType() {
		string error = "error: line " +
				to_string(this->lineNumber_) +
				" invalid binary operation";
		if (this->operationType_ == Cast) {
			if (!ValidType::IsValidCast(
					this->castTo_, this->castFrom_)) {
				ASTNode::LogError(this->lineNumber_,
					string("invalid binary operation."));
			}
		} else {
			if (!ValidType::IsValidBinaryOp(
					this->children_[0]->resultType_,
					this->children_[1]->resultType_)) {
				ASTNode::LogError(this->lineNumber_,
					string("invalid binary operation."));
			}
		}

		switch (this->operationType_) {
		case Assign:
			this->resultType_ = ValidType::GetUnderlyingType(
					this->children_[0]->resultType_);
			break;
		case Cast:
			// castTo_ should be valid by this point
			this->resultType_ = this->castTo_;
			break;
		case Multiply:
			this->resultType_ = ValidType::GetUnderlyingType(
					this->children_[0]->resultType_);
			break;
		case Divide:
			this->resultType_ = ValidType::GetUnderlyingType(
					this->children_[0]->resultType_);
			break;
		case Add:
			this->resultType_ = ValidType::GetUnderlyingType(
					this->children_[0]->resultType_);
			break;
		case Subtract:
			this->resultType_ = ValidType::GetUnderlyingType(
					this->children_[0]->resultType_);
			break;
		case Equality:
			this->resultType_ = new BoolType();
			break;
		case LessThan:
			this->resultType_ = new BoolType();
			break;
		case GreaterThan:
			this->resultType_ = new BoolType();
			break;
		case Land:
			this->resultType_ = new BoolType();
			break;
		case Lor:
			this->resultType_ = new BoolType();
			break;
		}
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Value * ret = nullptr;
		llvm::Value * L = nullptr;
		llvm::Value * R = nullptr;
		string error = "error: invalid binary operation in GenerateCode";
		switch (this->operationType_) {
		case Assign:
		{
			R = this->children_[1]->GenerateCode(endBlock);
			ExistingVarNode * existingNode = (ExistingVarNode *)this->children_[0];
			llvm::AllocaInst * alloca = get<1>(ASTNode::varTable_[existingNode->identifier_]);
			return GlobalBuilder.CreateStore(R, alloca);
		}
			// the break below will never run
			break;
		case Cast:
		{
			R = this->children_[0]->GenerateCode(endBlock);
			llvm::Type * castTo = ValidType::ConvertVariableTypeToLLVMType(
					this->resultType_->varType_);
			switch (this->castFrom_->varType_) {
			case FloatVarType: // float -> int / cint
				return GlobalBuilder.CreateFPCast(R, castTo, "cast_float");
			case IntVarType: // int -> float / cint
				return GlobalBuilder.CreateIntCast(R, castTo, true, "cast_int");
			case CintVarType: // cint -> float / int
				return GlobalBuilder.CreateIntCast(R, castTo, true, "cast_cint");
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case Multiply:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFMul(L, R, "mul_float");
			case IntVarType:
				return GlobalBuilder.CreateNSWMul(L, R, "mul_int");
			case CintVarType:
			{
				vector<llvm::Value *> params { L, R };
				return GlobalBuilder.CreateCall(
					ASTNode::cintMultiplyFunction_, params, "cint_multiply");
			}
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case Divide:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFDiv(L, R, "div_float");
			case IntVarType:
				return GlobalBuilder.CreateSDiv(L, R, "div_int");
			case CintVarType:
			{
				vector<llvm::Value *> params { L, R };
				return GlobalBuilder.CreateCall(
					ASTNode::cintDivideFunction_, params, "cint_divide");
			}
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case Add:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFAdd(L, R, "add_float");
			case IntVarType:
				return GlobalBuilder.CreateNSWAdd(L, R, "add_int");
			case CintVarType:
			{
				vector<llvm::Value *> params { L, R };
				return GlobalBuilder.CreateCall(
					ASTNode::cintAddFunction_, params, "cint_add");
			}
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case Subtract:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFSub(L, R, "sub_float");
			case IntVarType:
				return GlobalBuilder.CreateNSWSub(L, R, "sub_int");
			case CintVarType:
			{
				vector<llvm::Value *> params { L, R };
				return GlobalBuilder.CreateCall(
					ASTNode::cintSubtractFunction_, params, "cint_subtract");
			}
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case Equality:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (ValidType::GetUnderlyingType(this->children_[0]->resultType_)->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFCmpOEQ(L, R, "cmpeq_float");
			case IntVarType:
				return GlobalBuilder.CreateICmpEQ(L, R, "cmpeq_int");
			case CintVarType:
				return GlobalBuilder.CreateICmpEQ(L, R, "cmpeq_cint");
			case BooleanVarType:
				return GlobalBuilder.CreateICmpEQ(L, R, "cmpeq_bool");
			case RefVarType:
				return GlobalBuilder.CreateICmpEQ(L, R, "cmpeq_ref");
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case LessThan:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (ValidType::GetUnderlyingType(this->children_[0]->resultType_)->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFCmpOLT(L, R, "cmplt_float");
			case IntVarType:
				return GlobalBuilder.CreateICmpSLT(L, R, "cmplt_int");
			case CintVarType:
				return GlobalBuilder.CreateICmpSLT(L, R, "cmplt_cint");
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case GreaterThan:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (ValidType::GetUnderlyingType(this->children_[0]->resultType_)->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFCmpOGT(L, R, "cmpgt_float");
			case IntVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_int");
			case CintVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_cint");
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case Land:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (ValidType::GetUnderlyingType(this->children_[0]->resultType_)->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFCmpOGT(L, R, "cmpgt_float");
			case IntVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_int");
			case CintVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_cint");
			case BooleanVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_bool");
			case RefVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_ref");
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		case Lor:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (ValidType::GetUnderlyingType(this->children_[0]->resultType_)->varType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFCmpOGT(L, R, "cmpgt_float");
			case IntVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_int");
			case CintVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_cint");
			case BooleanVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_bool");
			case RefVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_ref");
			default:
				cout << error << endl;
				exit(1);
				break;
			}
		}
			break;
		default:
			cout << error << endl;
			exit(1);
			break;
		}
		return ret;
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		ss << left1 << "name: binop" << '\n';
		ss << left1 << "op: ";

		switch (this->operationType_) {
		case Assign:
			ss << "assign" << '\n';
			break;
		case Cast:
			ss << "cast" << '\n';
			ss << left1 << "type: " << this->castTo_->GetName() << '\n';
			this->children_[0]->PrintRecursive(ss, depth);
			return;
		case Multiply:
			ss << "mul" << '\n';
			break;
		case Divide:
			ss << "div" << '\n';
			break;
		case Add:
			ss << "add" << '\n';
			break;
		case Subtract:
			ss << "sub" << '\n';
			break;
		case Equality:
			ss << "eq" << '\n';
			break;
		case LessThan:
			ss << "lt" << '\n';
			break;
		case GreaterThan:
			ss << "gt" << '\n';
			break;
		case Land:
			ss << "and" << '\n';
			break;
		case Lor:
			ss << "or" << '\n';
			break;
#ifndef NO_DEBUG
		case EmptyBinaryOperation:
			assert(false);
			break;
#endif
		}
		ss << left1 << "lhs:" << '\n';
		this->children_[0]->PrintRecursive(ss, depth+1);
		ss << left1 << "rhs:" << '\n';
		this->children_[1]->PrintRecursive(ss, depth+1);
	}
};


struct ExpressionNode: public ASTNode {
	int constructorCase_ = 0;
	Literal * literal_ = nullptr;

	ExpressionNode(unsigned lineNumber,
			ExpressionNode * expressionNode) :
		ASTNode(lineNumber), constructorCase_(0) {
		this->children_.push_back(expressionNode);
		this->resultType_ = expressionNode->resultType_;
	}

	ExpressionNode(unsigned lineNumber,
			BinaryOperationNode * binaryOperationNode) :
			ASTNode(lineNumber), constructorCase_(1)  {
		this->children_.push_back(binaryOperationNode);
		this->resultType_ = binaryOperationNode->resultType_;
	}

	ExpressionNode(unsigned lineNumber,
			UnaryOperationNode * unaryOperationNode) :
		ASTNode(lineNumber), constructorCase_(2) {
		this->children_.push_back(unaryOperationNode);
		this->resultType_ = unaryOperationNode->resultType_;
	}

	ExpressionNode(unsigned lineNumber,
			Literal * literal) :
		ASTNode(lineNumber), constructorCase_(3), literal_(literal) {
		this->resultType_ = Literal::ConvertToValidType(literal_);
		assert(this->resultType_);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingVarNode * existingVarNode) :
		ASTNode(lineNumber), constructorCase_(4) {

		VarTableEntry hit = ASTNode::varTable_.find(existingVarNode->identifier_);
		if (hit==ASTNode::varTable_.end()) {
			ASTNode::LogError(lineNumber,
				string("variable identifier ") +
				existingVarNode->identifier_ +
				string("not declared."));
		} else {
			this->resultType_ = get<0>(hit->second);
		}
		this->children_.push_back(existingVarNode);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingFuncNode * existingFuncNode) :
		 ASTNode(lineNumber), constructorCase_(5) {

		FuncTableEntry hit = ASTNode::funcTable_.find(existingFuncNode->identifier_);
		if (hit==ASTNode::funcTable_.end()) {

			ASTNode::LogError(lineNumber,
				string("function identifier ") +
				existingFuncNode->identifier_ +
				string(" not declared."));
		} else {
			vector<ValidType *> vtypes = get<0>(hit->second);
			this->resultType_ = vtypes[0];
		}
		this->children_.push_back(existingFuncNode);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingFuncNode * existingFuncNode,
			ASTNode * expressionsNode) :
		ASTNode(lineNumber), constructorCase_(6) {
		this->children_.push_back(existingFuncNode);
		for (unsigned i=0; i<expressionsNode->children_.size(); ++i) {
			this->children_.push_back(expressionsNode->children_[i]);
		}


		FuncTableEntry hit = ASTNode::funcTable_.find(existingFuncNode->identifier_);
		if (hit==ASTNode::funcTable_.end()) {
			get<0>(recursiveFuncPlaceHolder_) = existingFuncNode->identifier_;
			get<1>(recursiveFuncPlaceHolder_) = lineNumber;
		} else {
			vector<ValidType *> vtypes = get<0>(hit->second);
			this->resultType_ = vtypes[0];

			for (unsigned i=1; i<expressionsNode->children_.size(); ++i) {
				if (vtypes[i]->varType_==RefVarType) {
					bool isLiteral = ((ExpressionNode *)expressionsNode->children_[i])->literal_ != nullptr;
					if (isLiteral) {
						ASTNode::LogError(lineNumber,
						"Can't assign function parameter of type ref to a literal");
					}
				}
			}
		}
	}

	virtual ~ExpressionNode() {
		if (this->literal_ != nullptr) {
			delete this->literal_;
			this->literal_ = nullptr;
		}
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		ss << left1 << "exp:" << '\n';
		switch (this->constructorCase_) {
		case 0:
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 1:
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 2:
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 3:
		{
			ss << left2 << "name: " << literal_->GetName() << '\n';
			ss << left2 << "value: ";
			unique_ptr<LiteralValue>& lv = literal_->GetValue();
			switch (literal_->type_) {
			case IntLiteral:
				ss << lv->iValue_ << '\n';
				break;
			case FloatLiteral:
				ss << lv->fValue_ << '\n';
				break;
			case StringLiteral:
				ss << lv->sValue_.substr(1, string::npos - 1) << '\n';
				break;
			case BooleanLiteral:
				ss << lv->bValue_ << '\n';
				break;
#ifndef NO_DEBUG
			case EmptyLiteral:
				assert(false);
				break;
#endif
			}
		}
			break;
		case 4:
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 5:
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 6:
			this->children_[0]->PrintRecursive(ss, depth+1);
			ss << left2 << "params:" << '\n';
			for (unsigned i=1; i<this->children_.size(); ++i) {
				this->children_[i]->PrintRecursive(ss, depth+1);
			}
			break;
		}
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {

		llvm::Value * ret = nullptr;
		switch (this->constructorCase_) {
		case 0:
			ret = this->children_[0]->GenerateCode(endBlock);
			break;
		case 1:
			ret = this->children_[0]->GenerateCode(endBlock);
			break;
		case 2:
			ret = this->children_[0]->GenerateCode(endBlock);
			break;
		case 3:
		{
			unique_ptr<LiteralValue>& lv = literal_->GetValue();
			switch (literal_->type_) {
			case IntLiteral:
				ret = llvm::ConstantInt::get(GlobalContext, llvm::APInt(32, lv->iValue_));
				break;
			case FloatLiteral:
				ret = llvm::ConstantFP::get(GlobalContext, llvm::APFloat(lv->fValue_));
				break;
			case StringLiteral:
				break;
			case BooleanLiteral:
				ret = llvm::ConstantInt::get(GlobalContext, llvm::APInt(1, lv->iValue_));
				break;
			case EmptyLiteral:
				break;
			}
		}
			break;
		case 4:
			ret = this->children_[0]->GenerateCode(endBlock);
			break;
		case 5:
		{
			ExistingFuncNode * existingFuncNode = (ExistingFuncNode *)this->children_[0];
			llvm::Function * function = (llvm::Function *)
					existingFuncNode->GenerateCode(nullptr);
			llvm::Type * retType = function->getReturnType();
			if (retType->isVoidTy()) {
				ret = GlobalBuilder.CreateCall(function, llvm::None);
			} else {
				ret = GlobalBuilder.CreateCall(function, llvm::None, "existing_func_call");
			}
		}
			break;
		case 6:
		{
			ExistingFuncNode * existingFuncNode = (ExistingFuncNode *)this->children_[0];
			llvm::Function * function = (llvm::Function *)
					existingFuncNode->GenerateCode(nullptr);
			vector<llvm::Value *> llvmFunctionParams;
			vector<ValidType *> funcParams =
				get<0>(ASTNode::funcTable_[existingFuncNode->identifier_]);
			for (int i=1; i<this->children_.size(); ++i) {
				if (funcParams[i]->varType_==RefVarType) {
					if (this->children_[i]->children_.size()==0) {
						// should never be here
						cout << "error: invalid function call" << endl;
						exit(1);
					}
					ExistingVarNode * existingVar = (ExistingVarNode*) (this->children_[i]->children_[0]);
					llvm::AllocaInst * alloca = get<1>(ASTNode::varTable_[existingVar->identifier_]);

					llvm::AllocaInst * newPtrAlloca = GlobalBuilder.CreateAlloca(
							funcParams[i]->GetLLVMType(),
							nullptr, existingVar->identifier_);

					llvm::StoreInst * storeInst = GlobalBuilder.CreateStore(alloca, newPtrAlloca);

					llvm::LoadInst * loadInstruction =  GlobalBuilder.CreateLoad(
							newPtrAlloca, existingVar->identifier_);

					llvmFunctionParams.push_back(loadInstruction);

				} else {
					llvmFunctionParams.push_back(
						this->children_[i]->GenerateCode(endBlock));
				}
			}
			if (function->getReturnType()->isVoidTy()) {
				ret = GlobalBuilder.CreateCall(function,
					llvmFunctionParams);
			} else {
				ret = GlobalBuilder.CreateCall(function,
						llvmFunctionParams, "existing_func_call");
			}
		}
			break;
		default:
			cout << "unhandled GenerateCode() in ExpressionNode" << endl;
			break;
		}

		return ret;
	}
};


struct ExpressionsNode: public ASTNode {
	ExpressionsNode(unsigned lineNumber,
			ExpressionNode * expressionNode) : ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
	}

	ExpressionsNode(unsigned lineNumber,
			ExpressionsNode * expressionsNode,
			ExpressionNode * expressionNode) : ASTNode(lineNumber) {
		for (auto node : expressionsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(expressionNode);
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		string left2 = std::string((depth+1)*2, ' ');
		ss << left1 << "name: exps" << '\n';
		ss << left1 << "exps:" << '\n';
		ss << left2 << "-" << '\n';

		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(ss, depth+2);
			if (i!=this->children_.size()-1)
				ss << left2 << "-" << '\n';
		}
	}

};


struct StatementNode: public ASTNode {
	ControlFlow * controlFlow_ = nullptr;
	Function * function_ = nullptr;
	VdeclNode * varDecl_ = nullptr;
	string stmtName_;
	string printStringLiteral_;
	ExpressionNode * exprNode_ = nullptr;
	int constructorCase_ = 0;

	virtual ~StatementNode() {
		if (this->controlFlow_ != nullptr) {
			delete this->controlFlow_;
			this->controlFlow_ = nullptr;
		}
		if (this->function_ != nullptr) {
			delete this->function_;
			this->function_ = nullptr;
		}
	}

	StatementNode(unsigned lineNumber, ASTNode * blockNode) :
		ASTNode(lineNumber), stmtName_("blkstmt"), constructorCase_(0) {
		this->children_.push_back(blockNode);
	}

	StatementNode(unsigned lineNumber, ReturnControl * returnControl) :
		ASTNode(lineNumber), controlFlow_(returnControl),
			stmtName_("retstmt"),
			constructorCase_(1) {
		ASTNode::currentFunctionReturnType_ = VoidVarType;
	}

	StatementNode(unsigned lineNumber,
			ReturnControl * returnControl,
			ExpressionNode * expressionNode) :
		ASTNode(lineNumber),
		controlFlow_(returnControl),
		stmtName_("retstmt"),
		exprNode_(expressionNode),
		constructorCase_(2) {
		this->children_.push_back(expressionNode);
		ASTNode::currentFunctionReturnType_ = expressionNode->resultType_->varType_;
	}

	StatementNode(unsigned lineNumber,
			ASTNode * vdeclNode,
			ExpressionNode * expressionNode) :
			ASTNode(lineNumber),
			stmtName_("vardeclstmt"),
			exprNode_(expressionNode), constructorCase_(3){
		this->varDecl_ = (VdeclNode *)(vdeclNode);
		this->children_.push_back(vdeclNode);
		this->children_.push_back(expressionNode);

		if(this->varDecl_->type_->varType_ == RefVarType){
			// ref int $x = $y + 4;
			if (expressionNode->constructorCase_ != 4) {
				ASTNode::LogError(this->lineNumber_,
				"ref variable must be initialized with a variable.\n");
			}
			// ref float $y = $z;
			// ref int $x = $y;
			else if (ValidType::GetUnderlyingType(expressionNode->resultType_)->varType_ !=
					 ValidType::GetUnderlyingType(this->varDecl_->type_)->varType_) {
				ASTNode::LogError(this->lineNumber_,
				"underlying ref types different.\n");
			}
		}
	}

	StatementNode(unsigned lineNumber,
			ExpressionNode * expressionNode) :
		ASTNode(lineNumber), stmtName_("expstmt"),
		exprNode_(expressionNode), constructorCase_(4) {
		this->children_.push_back(expressionNode);
	}

	StatementNode(unsigned lineNumber,
			WhileControl * whileControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode) :
			ASTNode(lineNumber),
			controlFlow_(whileControl),
			stmtName_("whilestmt"),
			exprNode_(expressionNode),
			constructorCase_(5) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode);
	}

	StatementNode(unsigned lineNumber,
			IfControl * ifControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode) :
			ASTNode(lineNumber),
			controlFlow_(ifControl),
			stmtName_("ifstmt"),
			exprNode_(expressionNode),
			constructorCase_(6) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode);
	}

	StatementNode(unsigned lineNumber,
			ElseControl * elseControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode1,
			StatementNode * statementNode2) :
			ASTNode(lineNumber),
			controlFlow_(elseControl),
			stmtName_("ifstmt"),
			exprNode_(expressionNode),
			constructorCase_(7) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode1);
		this->children_.push_back(statementNode2);
	}

	StatementNode(unsigned lineNumber,
			PrintFunction * printFunction,
			ExpressionNode * expressionNode) :
			ASTNode(lineNumber),
			function_(printFunction),
			stmtName_("printstmt"),
			exprNode_(expressionNode),
			constructorCase_(8) {
		this->children_.push_back(expressionNode);
	}

	StatementNode(unsigned lineNumber,
			PrintFunction * printFunction,
			string stringLiteral) :
			ASTNode(lineNumber),
			function_(printFunction),
			stmtName_("printslit"),
			printStringLiteral_(stringLiteral),
			constructorCase_(9) {
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		ss << left1 << "name: " << this->stmtName_ << '\n';

		switch (this->constructorCase_) {
		case 0: // block
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 1: // return
			break;
		case 2: // return
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 3: // vdecl
			this->children_[0]->PrintRecursive(ss, depth+1);
			this->children_[1]->PrintRecursive(ss, depth+1);
			break;
		case 4: //exp
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 5: // while
			ss << left1 << "cond:" << '\n';
			this->children_[0]->PrintRecursive(ss, depth+1);
			ss << left1 << "stmt:" << '\n';
			this->children_[1]->PrintRecursive(ss, depth+1);
			break;
		case 6: // if
			ss << left1 << "cond:" << '\n';
			this->children_[0]->PrintRecursive(ss, depth+1);
			this->children_[1]->PrintRecursive(ss, depth+1);
			break;
		case 7: // else
			ss << left1 << "cond:" << '\n';
			this->children_[0]->PrintRecursive(ss, depth+1);
			ss << left1 << "stmt:" << '\n';
			this->children_[1]->PrintRecursive(ss, depth+1);
			ss << left1 << "else_stmt:" << '\n';
			this->children_[2]->PrintRecursive(ss, depth+1);
			break;
		case 8: // print exp
			this->children_[0]->PrintRecursive(ss, depth+1);
			break;
		case 9: // print literal
			ss << left1 << "string: " <<
			this->printStringLiteral_ << '\n';
			break;
		}
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Function * parentFunction = GlobalBuilder.GetInsertBlock()->getParent();
		llvm::BasicBlock * startBlock = llvm::BasicBlock::Create(
				GlobalContext, "stmt_entry", parentFunction, nullptr);
		llvm::BasicBlock * endOfMiddleBlock = llvm::BasicBlock::Create(
				GlobalContext, "stmt_exit", parentFunction, nullptr);
		GlobalBuilder.SetInsertPoint(startBlock);

		switch (this->constructorCase_) {
		case 0: // blk ( in an if or else statement i.e.)
		{

			llvm::BasicBlock * middleBlock = (llvm::BasicBlock *)
				this->children_[0]->GenerateCode(endOfMiddleBlock);
			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateBr(middleBlock);

		}
			break;
		case 1:
			GlobalBuilder.CreateRetVoid();
			// GlobalBuilder.SetInsertPoint(startBlock);
			// GlobalBuilder.CreateBr(endOfMiddleBlock);
			break;
		case 2:
		{
			llvm::Value * rhs = this->exprNode_->GenerateCode(endOfMiddleBlock);
			GlobalBuilder.CreateRet(rhs);
			// GlobalBuilder.SetInsertPoint(startBlock);
			// GlobalBuilder.CreateBr(endOfMiddleBlock);
			break;
		}
		case 3:
		{
			llvm::Value * rhs = this->exprNode_->GenerateCode(endOfMiddleBlock);
			this->varDecl_->GenerateCode(endOfMiddleBlock);
			llvm::AllocaInst * alloca = nullptr;
			// check for ref type for store instruction
			ValidType * vtype = get<0>(ASTNode::varTable_[this->varDecl_->identifier_]);
			assert(vtype);
			if (vtype->GetVarType()==RefVarType) {
				// dont need to generate a store here, since
				// references dont need stack space
				ExistingVarNode * existingVarNode = (ExistingVarNode *)
						this->exprNode_->children_[0];

				get<1>(ASTNode::varTable_[this->varDecl_->identifier_]) =
					get<1>(ASTNode::varTable_[existingVarNode->identifier_]);

			} else {
				alloca = get<1>(ASTNode::varTable_[this->varDecl_->identifier_]);
				GlobalBuilder.CreateStore(rhs, alloca);
			}
			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateBr(endOfMiddleBlock);
		}
			break;
		case 4:
		{
			this->exprNode_->GenerateCode(endOfMiddleBlock);
			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateBr(endOfMiddleBlock);
		}
			break;
		case 5:
		{
			llvm::Value * condValue = this->exprNode_->GenerateCode(nullptr);

			// generate while portion
			llvm::BasicBlock * whileStmtBlock =
					(llvm::BasicBlock *)this->children_[1]->GenerateCode(startBlock);

			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateCondBr(condValue, whileStmtBlock, endOfMiddleBlock);
		}
			break;
		case 6:
		{
			llvm::Value * condValue = this->exprNode_->GenerateCode(nullptr);

			// generate if portion
			llvm::BasicBlock * ifStmtBlock =
				(llvm::BasicBlock *)this->children_[1]->GenerateCode(endOfMiddleBlock);

			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateCondBr(condValue, ifStmtBlock, endOfMiddleBlock);

		}
			break;
		case 7: // if else
		{
			llvm::Value * condValue = this->exprNode_->GenerateCode(nullptr);

			// generate if portion
			llvm::BasicBlock * ifStmtBlock =
				(llvm::BasicBlock *)this->children_[1]->GenerateCode(endOfMiddleBlock);

			// generate else portion
			llvm::BasicBlock * elseStmtBlock =
				(llvm::BasicBlock *)this->children_[2]->GenerateCode(endOfMiddleBlock);

			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateCondBr(condValue, ifStmtBlock, elseStmtBlock);
		}
			break;
		case 8:
		{
			vector<llvm::Value *> printfParams;
			llvm::Constant * formatStr = nullptr;
			switch (ValidType::GetUnderlyingType(this->exprNode_->resultType_)->varType_) {
			case IntVarType:
			case CintVarType:
			case BooleanVarType:
				formatStr = PrintFunction::Geti8LLVMValueFromStr("%d\n");
				printfParams.push_back(formatStr);
				printfParams.push_back(this->exprNode_->GenerateCode(endOfMiddleBlock));
				break;
			case FloatVarType:
			{
				formatStr = PrintFunction::Geti8LLVMValueFromStr("%f\n");
				printfParams.push_back(formatStr);
				llvm::Value * floatInst = this->exprNode_->GenerateCode(endOfMiddleBlock);
				llvm::AllocaInst * doubleInstAlloca = GlobalBuilder.CreateAlloca(
						llvm::Type::getDoubleTy(GlobalContext), nullptr, "floatToDouble");
				llvm::Value * floatExtendedInst = GlobalBuilder.CreateFPCast(
						floatInst, llvm::Type::getDoubleTy(GlobalContext),
						llvm::Twine("floatToDouble"));
				GlobalBuilder.CreateStore(floatExtendedInst, doubleInstAlloca);
				llvm::LoadInst * doubleInstLoad = GlobalBuilder.CreateLoad(
						doubleInstAlloca, "floatToDouble");
				printfParams.push_back(doubleInstLoad);
			}
				break;
			default:
				cout << "unhandled case in statement GenerateNode" << endl;
				break;
			}

			GlobalBuilder.CreateCall(ASTNode::printfFunction_, printfParams, "printfCall");
			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateBr(endOfMiddleBlock);
		}
			break;
		case 9:
		{
			vector<llvm::Value *> printfParams;
			llvm::Constant * formatStr = PrintFunction::Geti8LLVMValueFromStr("%s\n");
			this->printStringLiteral_.erase(
				remove(this->printStringLiteral_.begin(),
					   this->printStringLiteral_.end(), '"'),
			    this->printStringLiteral_.end());

			llvm::Constant * llvmStr = PrintFunction::Geti8LLVMValueFromStr(
				(this->printStringLiteral_ + "\n").c_str());
			printfParams.push_back(llvmStr);
			GlobalBuilder.CreateCall(ASTNode::printfFunction_, printfParams, "printfCall");
			GlobalBuilder.SetInsertPoint(startBlock);
			GlobalBuilder.CreateBr(endOfMiddleBlock);
		}
			break;
		default:
			cout << "unhandled case in statement generate code" << endl;
			break;
		}

		if (endBlock!=nullptr) {
			GlobalBuilder.SetInsertPoint(endOfMiddleBlock);
			GlobalBuilder.CreateBr(endBlock);
		}

		return startBlock;
	}

};

struct StatementsNode: public ASTNode {
	StatementsNode(unsigned lineNumber,
			StatementNode * statementNode) : ASTNode(lineNumber) {
		this->children_.push_back(statementNode);
	}

	StatementsNode(unsigned lineNumber,
			StatementsNode * statementsNode,
			StatementNode * statementNode) : ASTNode(lineNumber) {
		for (auto node : statementsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(statementNode);
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock3) {
		llvm::Function * parentfunction = GlobalBuilder.GetInsertBlock()->getParent();
		llvm::BasicBlock * startBlock = llvm::BasicBlock::Create(GlobalContext, "stmts_entry", parentfunction, nullptr);


		llvm::BasicBlock * endOfMiddleBlock = startBlock;
		for (auto n: this->children_) {

			llvm::BasicBlock * endBlock =
				llvm::BasicBlock::Create(GlobalContext, "stmts_merge", parentfunction, endBlock);

			llvm::BasicBlock * newBlock = (llvm::BasicBlock *)n->GenerateCode(endBlock);

			GlobalBuilder.SetInsertPoint(endOfMiddleBlock);
			GlobalBuilder.CreateBr(newBlock);

			endOfMiddleBlock = endBlock;

		}
		if (endBlock3!=nullptr) {
			GlobalBuilder.SetInsertPoint(endOfMiddleBlock);
			GlobalBuilder.CreateBr(endBlock3);
		}

		return startBlock;
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		string left2 = std::string((depth+1)*2, ' ');
		ss << left1 << "name: stmts" << '\n';
		ss << left1 << "stmts:" << '\n';
		ss << left2 << "-" << '\n';

		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(ss, depth+2);
			if (i!=this->children_.size()-1)
				ss << left2 << "-" << '\n';
		}
	};
};

struct BlockNode: public ASTNode {
	BlockNode(unsigned lineNumber): ASTNode(lineNumber) {
	}

	BlockNode(unsigned lineNumber, ASTNode * statementsNode) :
		ASTNode(lineNumber) {

		this->children_.push_back(statementsNode);

		for(auto node : statementsNode->children_){
			StatementNode * sNode = (StatementNode *) node;
			if(sNode->constructorCase_ == 3){
				ASTNode::varTable_.erase(sNode->varDecl_->identifier_);
			}
		}

	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Function * parentfunction = GlobalBuilder.GetInsertBlock()->getParent();
		llvm::BasicBlock * startBlock = llvm::BasicBlock::Create(GlobalContext, "blk_entry", parentfunction);
		llvm::BasicBlock * endOfMiddleBlock = llvm::BasicBlock::Create(GlobalContext, "blk_end", parentfunction);
		llvm::BasicBlock * middleBlock = (llvm::BasicBlock *)this->children_[0]->GenerateCode(endOfMiddleBlock);

		GlobalBuilder.SetInsertPoint(startBlock);
		GlobalBuilder.CreateBr(middleBlock);

		if (endBlock!=nullptr) {
			GlobalBuilder.SetInsertPoint(endOfMiddleBlock);
			GlobalBuilder.CreateBr(endBlock);
		}


		return startBlock;
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		ss << left1 << "blk:" << '\n';
		ss << left2 << "name: blk" << '\n';
		if (this->children_.size()>0) {
			ss << left2 << "contents:"<< '\n';
			this->children_[0]->PrintRecursive(ss, depth+1);
		}
	}
};


struct FuncNode : public ASTNode {
	ValidType * retType_ = nullptr;
	string identifier_;
	BlockNode * blockNode_ = nullptr;
	vector<llvm::Type *> llvmParamTypes_;
	bool isRunFunc_ = false;
	int constructorCase_ = -1;

	FuncNode(unsigned lineNumber,
			ValidType * retType, string identifier,
		BlockNode * blockNode) : ASTNode(lineNumber), retType_(retType),
				identifier_(identifier), blockNode_(blockNode),
				constructorCase_(0) {

		this->children_.push_back(blockNode);

		if(identifier == "run"){
			if(retType->varType_ != IntVarType){
				ASTNode::LogError(lineNumber,
					string("return type of run function has to be int"));
			}
			this->isRunFunc_ = true;
			ASTNode::runDefined_ = true;
		} else if(retType->varType_== RefVarType) {
			ASTNode::LogError(lineNumber,
				string("function return type can't be ref."));
		} else if (retType->varType_!=ASTNode::currentFunctionReturnType_) {
			ASTNode::LogError(lineNumber,
				string("mismatched return type from function prototype"));
		}

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);
		CheckFuncTable(lineNumber, identifier);
		get<0>(ASTNode::funcTable_[identifier]) = vTypes;
		ASTNode::currentFunctionReturnType_ = VoidVarType;
	}

	FuncNode(unsigned lineNumber,
		ValidType * retType, string identifier,
		VdeclsNode * vdeclsNode, BlockNode * blockNode) :
			ASTNode(lineNumber), retType_(retType),
			identifier_(identifier), blockNode_(blockNode),
			constructorCase_(1) {

		this->children_.push_back(vdeclsNode);
		this->children_.push_back(blockNode);

		if(identifier == "run"){
			ASTNode::LogError(lineNumber,
				string("run function can't take any arguments"));
		}

		for(auto node : vdeclsNode->children_){
			VdeclNode * vNode = (VdeclNode *) node;
			ASTNode::varTable_.erase(vNode->identifier_);
		}

		if(retType->varType_==RefVarType) {
			ASTNode::LogError(lineNumber,
				string("function return type can't be ref."));
		} else if (retType->varType_!=ASTNode::currentFunctionReturnType_) {
			ASTNode::LogError(lineNumber,
				string("mismatched return type from function prototype"));
		}

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);
		for (auto t : vdeclsNode->children_) {
			vTypes.push_back(((VdeclNode *)t)->type_);
			this->llvmParamTypes_.push_back(((VdeclNode *)t)->type_->GetLLVMType());
		}

		CheckFuncTable(lineNumber, identifier);

		get<0>(ASTNode::funcTable_[identifier]) = vTypes;
		ASTNode::currentFunctionReturnType_ = VoidVarType;
	}

	llvm::Function *
	GenerateCode(llvm::BasicBlock *) {
		llvm::Type * llvmRetType = this->retType_->GetLLVMType();
		llvm::FunctionType * functionType = nullptr;
		if (this->constructorCase_==0) {
			functionType = llvm::FunctionType::get(llvmRetType, false);
		} else if (this->constructorCase_==1) {
			functionType = llvm::FunctionType::get(llvmRetType, this->llvmParamTypes_, false);
		}

		llvm::Function * ret = llvm::Function::Create(
			functionType, llvm::Function::ExternalLinkage,
			this->identifier_, GlobalModule);


		ASTNode::currentLLVMFunctionPrototype_ = ret;
		// create a new basic block to start insertion into
		llvm::BasicBlock * entryBlock = llvm::BasicBlock::Create(GlobalContext, "entry", ret);
		llvm::BasicBlock * endBlock = llvm::BasicBlock::Create(GlobalContext, "exit", ret);
		GlobalBuilder.SetInsertPoint(entryBlock);
		// set names for the arguments
		if (this->constructorCase_==1) {// there are arguments
			unsigned idx = 0;
			VdeclsNode * vdeclsNode = (VdeclsNode *)this->children_[0];
			for (auto &arg : ret->args()) {
				VdeclNode * vdeclNode = (VdeclNode *)vdeclsNode->children_[idx];
				arg.setName(vdeclNode->identifier_);
				get<0>(ASTNode::varTable_[vdeclNode->identifier_]) = vdeclNode->type_;
//				llvm::AllocaInst * alloca = vdeclNode->type_->CreateEntryBlockAlloca(
//						ret, vdeclNode->identifier_);
//				get<1>(ASTNode::varTable_[vdeclNode->identifier_]) = alloca;
//				GlobalBuilder.CreateStore(&arg, alloca);

				if (vdeclNode->type_->varType_!=RefVarType) {
					llvm::AllocaInst * alloca = vdeclNode->type_->CreateEntryBlockAlloca(
							ret, vdeclNode->identifier_);
					get<1>(ASTNode::varTable_[vdeclNode->identifier_]) = alloca;
					GlobalBuilder.CreateStore(&arg, alloca);
				} else {
					get<1>(ASTNode::varTable_[vdeclNode->identifier_]) = (llvm::AllocaInst *)&arg;
					//GlobalBuilder.CreateStore(&arg, alloca);
				}
				++idx;
			}
		}

		llvm::BasicBlock * middleBlock = (llvm::BasicBlock *)
				this->blockNode_->GenerateCode(endBlock);

		GlobalBuilder.SetInsertPoint(entryBlock);
		GlobalBuilder.CreateBr(middleBlock);
		GlobalBuilder.SetInsertPoint(endBlock);
		if (this->retType_->varType_==VoidVarType) {
			GlobalBuilder.CreateRetVoid();
		} else {
			GlobalBuilder.CreateUnreachable();
		}
		llvm::verifyFunction(*ret);

		// now put it in func table
		get<1>(ASTNode::funcTable_[this->identifier_]) = ret;
		if (this->isRunFunc_) {
			ASTNode::runFunction_ = ret;
		}
		return ret;
	}

	void
	CheckFuncTable(unsigned lineNumber, string identifier) {
		FuncTableEntry hit = ASTNode::funcTable_.find(identifier);
		if (hit != ASTNode::funcTable_.end()) {
			ASTNode::LogError(lineNumber,
				string("function identifier ") +
				identifier +
				string(" already defined"));
		} else {
			int checkRecursiveLn = get<1>(ASTNode::recursiveFuncPlaceHolder_);
			string checkRecursiveId = get<0>(ASTNode::recursiveFuncPlaceHolder_);
			bool recursiveCall = checkRecursiveLn!=-1 && identifier==checkRecursiveId;
			bool missingProto = checkRecursiveLn!=-1 && identifier!=checkRecursiveId;

			if (missingProto) {
				ASTNode::LogError(checkRecursiveLn,
					string("function identifier ") +
					checkRecursiveId +
					string(" not declared"));
			} else if (recursiveCall) {
				ASTNode::recursiveFuncNames_.insert(identifier);
			}
		}
		get<0>(ASTNode::recursiveFuncPlaceHolder_) = "";
		get<1>(ASTNode::recursiveFuncPlaceHolder_) = -1;
	}

	virtual ~FuncNode() {
		if (this->retType_ != nullptr) {
			delete this->retType_;
			this->retType_ = nullptr;
		}
	}

	void
	PrintSelf(stringstream& ss, unsigned depth) {
		string left = std::string(depth*2, ' ');
		ss << left << "name: func" << '\n';
		ss << left << "ret_type: " << this->retType_->GetName() << '\n';
		ss << left << "globid: " << this->identifier_ << '\n';
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		PrintSelf(ss, depth);
		for (auto node : this->children_) {
			node->PrintRecursive(ss, depth+1);
		}
	}
};

struct FuncsNode : public ASTNode {
	FuncsNode(unsigned lineNumber,
			FuncNode * funcNode) : ASTNode(lineNumber) {
		this->children_.push_back(funcNode);
	}

	FuncsNode(unsigned lineNumber, FuncsNode * funcsNode,
			FuncNode * funcNode) {
		for (auto node : funcsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(funcNode);
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		ss << left1 << "funcs:" << '\n';
		ss << left2 << "name: funcs" << '\n';
		ss << left2 << "funcs:" << '\n';
		ss << left3 << "-" << '\n';
		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(ss, depth+2);
			if (i!=this->children_.size()-1)
				ss << left3 << "-" << '\n';
		}
	}
};

#endif /* EKCC_AST_HPP_ */
