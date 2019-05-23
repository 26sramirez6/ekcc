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
#include<tuple>
#include<memory>
#include<assert.h>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "ValidTypes.hpp"
#include "LLVMGlobals.hpp"
using std::vector;
using std::string;
using std::move;
using std::stringstream;
using std::unordered_map;
using std::tuple;
using std::get;
using std::unique_ptr;
typedef unordered_map<string, tuple<ValidType *, llvm::AllocaInst *>> VarTable;
typedef unordered_map<string, tuple<ValidType *, llvm::AllocaInst * >>::const_iterator VarTableEntry;
typedef unordered_map<string, tuple<vector< ValidType * >, llvm::Function *> > FuncTable;
typedef unordered_map<string, tuple<vector< ValidType * >, llvm::Function *> >::const_iterator FuncTableEntry;

struct ASTNode {
	static ASTNode * root_;
	static bool ready_;
	static vector< string > compilerErrors_;
	static vector< int > lineNumberErrors_;
	static FuncTable funcTable_;
	static tuple<string, int> recursiveFuncPlaceHolder_;
	static VarTable varTable_;
	static bool runDefined_;
	vector< ASTNode * > children_;
	unsigned lineNumber_ = 0;
	VariableTypes resultType_ = EmptyVarType;

	ASTNode() : lineNumber_(0), resultType_(EmptyVarType) {}
	ASTNode(unsigned lineNumber) : lineNumber_(lineNumber) {}
	ASTNode(VariableTypes resultType) : resultType_(resultType) {}
	ASTNode(unsigned lineNumber, VariableTypes resultType) :
		lineNumber_(lineNumber), resultType_(resultType) {}

	virtual void
	PrintRecursive(stringstream& ss, unsigned depth) {};

	virtual
	~ASTNode() {
		for (auto n : this->children_) {
			delete n;
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
			ASTNode::ready_ = true;
			GlobalModule = new llvm::Module(inputFile, GlobalContext);
		}
	}
};

struct ProgramNode : public ASTNode {
	ProgramNode(unsigned lineNumber, ASTNode * funcsNode) :
		ASTNode(lineNumber) {
		this->children_.push_back(funcsNode);
		ASTNode::root_ = this;
		if (!ASTNode::runDefined_) {
			ASTNode::compilerErrors_.push_back(
				"error: Undefined run function\n");
		}
	}

	ProgramNode(unsigned lineNumber, ASTNode * externs,
			ASTNode * funcsNode) : ASTNode(lineNumber){
		this->children_.push_back(funcsNode);
		this->children_.push_back(externs);
		ASTNode::root_ = this;
		if (!ASTNode::runDefined_) {
			ASTNode::compilerErrors_.push_back(
				"error: Undefined run function\n");
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
	 GenerateCodeRecursive(llvm::raw_string_ostream& ss) {
	 	ASTNode::GenerateCode(nullptr);
	 	GlobalModule->print(ss, nullptr);
	 }

	string
	GetCompilerErrors() {
		string ret = "";
		for (auto e : ASTNode::compilerErrors_) {
			ret += e;
		}
		return ret;
	}

	bool
	HasCompilerErrors() {
		return ASTNode::compilerErrors_.size() > 0;
	}
};

struct VdeclNode : public ASTNode {
	ValidType * type_ = nullptr;
	string identifier_;

	virtual ~VdeclNode() {
		if (this->type_ != nullptr) {
			delete this->type_;
		}
	}

	VdeclNode(unsigned lineNumber, ValidType * type, string identifier) :
		ASTNode(lineNumber), type_(type), identifier_(identifier.substr(1, string::npos)) {
		if (ASTNode::varTable_.end() != ASTNode::varTable_.find(identifier_)) {
			stringstream ss;
			ss << "error: line " << lineNumber;
			ss << ": variable identifier ";
			ss << identifier_;
			ss << " already defined. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		} else if (type->varType_ == VoidVarType){
			stringstream ss;
			ss << "error: line " << lineNumber;
			ss << ": variable identifier ";
			ss << identifier_;
			ss << " cannot be void. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		} else if (type->varType_==RefVarType) {
			RefType * refType = (RefType *) type;
			if (refType->invalidConstructor_) {
				stringstream ss;
				ss << "error: line " << lineNumber;
				ss << ": variable identifier ";
				ss << identifier_;
				ss << " is a ref and points to a ref.\n";
				ASTNode::compilerErrors_.push_back(ss.str());
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
			allocaInst = CreateEntryBlockAlloca_int(
					parentFunction, this->identifier_);
			break;
		case FloatVarType:
			allocaInst = CreateEntryBlockAlloca_float(
					parentFunction, this->identifier_);
			break;
		case BooleanVarType:
			allocaInst = CreateEntryBlockAlloca_bool(
					parentFunction, this->identifier_);
			break;
		case RefVarType:
		{
			RefType * refType = (RefType *) this->type_;
			switch (refType->referredType_->varType_) {
			case IntVarType:
				allocaInst = CreateEntryBlockAlloca_intPtr(
						parentFunction, this->identifier_);
				break;
			case FloatVarType:
				allocaInst = CreateEntryBlockAlloca_floatPtr(
						parentFunction, this->identifier_);
				break;
			case BooleanVarType:
				allocaInst = CreateEntryBlockAlloca_boolPtr(
						parentFunction, this->identifier_);
				break;
			}
		}
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
		if (hit==ASTNode::varTable_.end()) {
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "variable identifier ";
			ss << identifier_;
			ss << " not declared. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
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
			cout << "in the right place" << endl;
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
		}
	};

};

struct ExternNode : public ASTNode {
	ValidType * retType_ = nullptr;
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
			ASTNode(lineNumber), retType_(retType), identifier_(identifier) {
		this->Validate(lineNumber,
			retType, identifier);
		this->children_.push_back(tdeclsNode);
	}

	void
	Validate(unsigned lineNumber,
			ValidType * retType, string identifier) {
		if(retType->varType_== RefVarType){
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "function return type can't be ref. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		}

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);

		FuncTableEntry hit = ASTNode::funcTable_.find(identifier);
		if (hit != ASTNode::funcTable_.end()) {
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "function identifier '";
			ss << identifier;
			ss << "' already defined. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		}

		get<0>(ASTNode::funcTable_[identifier]) = vTypes;
	}

	virtual ~ExternNode() {
		if (this->retType_ != nullptr) {
			delete this->retType_;
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
	};

};


struct UnaryOperationNode: public ASTNode {
	UnaryOperationTypes operationType_;
	VariableTypes resultType_ = EmptyVarType;
	UnaryOperationNode(unsigned lineNumber,
			UnaryOperationTypes operationType,
			ASTNode * expressionNode1) :
		ASTNode(lineNumber), operationType_(operationType) {
		if (operationType==Not) {
			this->resultType_ = BooleanVarType;
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
};

struct BinaryOperationNode: public ASTNode {
	BinaryOperationTypes operationType_;
	ValidType * typeCast_ = nullptr;
	VariableTypes castFrom_ = EmptyVarType;

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
		typeCast_(validType) {
		this->children_.push_back(expressionNode1);
		this->castFrom_ = expressionNode1->resultType_;
		SetResultType();
	}

	virtual ~BinaryOperationNode() {
		if (this->typeCast_ != nullptr) {
			delete this->typeCast_;
		}
	}

	void
	SetResultType() {
		switch (this->operationType_) {
		case Assign:
			this->resultType_ = this->children_[0]->resultType_;
			break;
		case Cast:
			this->resultType_ = typeCast_->varType_;
			break;
		case Multiply:
			this->resultType_ = this->children_[0]->resultType_;
			break;
		case Divide:
			this->resultType_ = this->children_[0]->resultType_;
			break;
		case Add:
			this->resultType_ = this->children_[0]->resultType_;
			break;
		case Subtract:
			this->resultType_ = this->children_[0]->resultType_;
			break;
		case Equality:
			this->resultType_ = BooleanVarType;
			break;
		case LessThan:
			this->resultType_ = BooleanVarType;
			break;
		case GreaterThan:
			this->resultType_ = BooleanVarType;
			break;
		case Land:
			this->resultType_ = BooleanVarType;
			break;
		case Lor:
			this->resultType_ = BooleanVarType;
			break;
		}
	}

	llvm::Value *
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Value * ret = nullptr;
		llvm::Value * L = nullptr;
		llvm::Value * R = nullptr;
		string error = "invalid binary operation";

		switch (this->operationType_) {
		case Assign:
		{
			R = this->children_[1]->GenerateCode(endBlock);
			ExistingVarNode * existingNode = (ExistingVarNode *)this->children_[0];
			llvm::AllocaInst * alloca = get<1>(ASTNode::varTable_[existingNode->identifier_]);

			ValidType * vtype = get<0>(ASTNode::varTable_[existingNode->identifier_]);
			if (vtype->varType_==RefVarType) {
				cout << "in the right place!!" << endl;
				llvm::LoadInst * loadInstruction =  GlobalBuilder.CreateLoad(
						alloca, existingNode->identifier_);
				return GlobalBuilder.CreateStore(R, get<1>(ASTNode::varTable_["x"]));
			}

			assert(alloca);
			assert(R);
			return GlobalBuilder.CreateStore(R, alloca);
		}
			// the break below will never run
			break;
		case Cast:
		{
			R = this->children_[0]->GenerateCode(endBlock);
			llvm::Type * castTo = ValidType::ConvertVariableTypeToLLVMType(this->resultType_);
			switch (this->castFrom_) {
			case FloatVarType: // float -> int / cint
				return GlobalBuilder.CreateFPCast(R, castTo, "cast_float");
			case IntVarType: // int -> float / cint
				return GlobalBuilder.CreateIntCast(R, castTo, true, "cast_int");
			case CintVarType: // cint -> float / int
				return GlobalBuilder.CreateIntCast(R, castTo, true, "cast_cint");
			default:
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case Multiply:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFMul(L, R, "mul_float");
			case IntVarType:
				return GlobalBuilder.CreateNSWMul(L, R, "mul_int");
			case CintVarType:
				return GlobalBuilder.CreateNSWMul(L, R, "mul_cint");
			default:
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case Divide:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFDiv(L, R, "div_float");
			case IntVarType:
				return GlobalBuilder.CreateSDiv(L, R, "div_int");
			case CintVarType:
				return GlobalBuilder.CreateSDiv(L, R, "div_cint");
			default:
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case Add:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFAdd(L, R, "add_float");
			case IntVarType:
				return GlobalBuilder.CreateNSWAdd(L, R, "add_int");
			case CintVarType:
				return GlobalBuilder.CreateNSWAdd(L, R, "add_cint");
			default:
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case Subtract:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFSub(L, R, "sub_float");
			case IntVarType:
				return GlobalBuilder.CreateNSWSub(L, R, "sub_int");
			case CintVarType:
				return GlobalBuilder.CreateNSWSub(L, R, "sub_cint");
			default:
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case Equality:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
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
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case LessThan:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFCmpOLT(L, R, "cmplt_float");
			case IntVarType:
				return GlobalBuilder.CreateICmpSLT(L, R, "cmplt_int");
			case CintVarType:
				return GlobalBuilder.CreateICmpSLT(L, R, "cmplt_cint");
			default:
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case GreaterThan:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
			case FloatVarType:
				return GlobalBuilder.CreateFCmpOGT(L, R, "cmpgt_float");
			case IntVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_int");
			case CintVarType:
				return GlobalBuilder.CreateICmpSGT(L, R, "cmpgt_cint");
			default:
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case Land:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
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
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		case Lor:
		{
			L = this->children_[0]->GenerateCode(endBlock);
			R = this->children_[1]->GenerateCode(endBlock);
			switch (this->resultType_) {
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
				ASTNode::compilerErrors_.push_back(error);
				break;
			}
		}
			break;
		default:
			ASTNode::compilerErrors_.push_back(error);
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
			ss << left1 << "type: " << this->typeCast_->GetName() << '\n';
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
		this->resultType_ = ValidType::ConvertLiteralToVariableType(literal_->type_);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingVarNode * existingVarNode) :
		ASTNode(lineNumber), constructorCase_(4) {
		this->resultType_ = ValidType::ConvertLiteralToVariableType(literal_->type_);

		VarTableEntry hit = ASTNode::varTable_.find(existingVarNode->identifier_);
		if (hit==ASTNode::varTable_.end()) {
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "variable identifier ";
			ss << existingVarNode->identifier_;
			ss << " not declared. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		} else {
			ValidType * vtype = get<0>(hit->second);
			this->resultType_ = vtype->varType_;
		}
		this->children_.push_back(existingVarNode);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingFuncNode * existingFuncNode) :
		 ASTNode(lineNumber), constructorCase_(5) {

		FuncTableEntry hit = ASTNode::funcTable_.find(existingFuncNode->identifier_);
		if (hit==ASTNode::funcTable_.end()) {
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "function identifier ";
			ss << existingFuncNode->identifier_;
			ss << " not declared. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		} else {
			vector<ValidType *> vtypes = get<0>(hit->second);
			this->resultType_ = vtypes[0]->varType_;
		}
		this->children_.push_back(existingFuncNode);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingFuncNode * existingFuncNode,
			ASTNode * expressionsNode) :
		ASTNode(lineNumber), constructorCase_(6) {
		this->children_.push_back(existingFuncNode);
		for (auto node : expressionsNode->children_) {
			this->children_.push_back(node);
		}

		FuncTableEntry hit = ASTNode::funcTable_.find(existingFuncNode->identifier_);
		if (hit==ASTNode::funcTable_.end()) {
			get<0>(recursiveFuncPlaceHolder_) = existingFuncNode->identifier_;
			get<1>(recursiveFuncPlaceHolder_) = lineNumber;
		} else {
			vector<ValidType *> vtypes = get<0>(hit->second);
			this->resultType_ = vtypes[0]->varType_;
		}
	}

	virtual ~ExpressionNode() {
		if (this->literal_ != nullptr) {
			delete this->literal_;
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
			return this->children_[0]->GenerateCode(endBlock);
		case 1:
			return this->children_[0]->GenerateCode(endBlock);
		case 2:
			return this->children_[0]->GenerateCode(endBlock);
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
			return this->children_[0]->GenerateCode(endBlock);
		case 5:
			cout << "unhandled yet" << endl;
			break;
		case 6:
			cout << "unhandled yet" << endl;
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
		}
		if (this->function_ != nullptr) {
			delete this->function_;
		}
	}

	StatementNode(unsigned lineNumber, ASTNode * blockNode) :
		ASTNode(lineNumber), stmtName_("blkstmt"), constructorCase_(0) {
		this->children_.push_back(blockNode);
	}

	StatementNode(unsigned lineNumber, ReturnControl * returnControl) :
		ASTNode(lineNumber), controlFlow_(returnControl),
			stmtName_("retstmt"),
			constructorCase_(1) {}

	StatementNode(unsigned lineNumber,
			ReturnControl * returnControl,
			ExpressionNode * expressionNode) :
		ASTNode(lineNumber),
		controlFlow_(returnControl),
		stmtName_("retstmt"),
		exprNode_(expressionNode),
		constructorCase_(2) {
		this->children_.push_back(expressionNode);
		// Check: a function may not return a ref type.

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
			if(expressionNode->constructorCase_ != 4){
				stringstream ss;
				ss << "error: line " << lineNumber << ": ";
				ss << "ref variable must be initialized with a variable.\n";
				ASTNode::compilerErrors_.push_back(ss.str());
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
		llvm::BasicBlock * ret = llvm::BasicBlock::Create(GlobalContext, "stmt", parentFunction, endBlock);
		GlobalBuilder.SetInsertPoint(ret);

		switch (this->constructorCase_) {
		case 1:
			GlobalBuilder.CreateRetVoid();
			break;
		case 2:
		{
			llvm::Value * rhs = this->exprNode_->GenerateCode(ret);
			GlobalBuilder.CreateRet(rhs);
			break;
		}
		case 3:
		{
			llvm::Value * rhs = this->exprNode_->GenerateCode(endBlock);
			this->varDecl_->GenerateCode(endBlock);
			llvm::AllocaInst * alloca = get<1>(ASTNode::varTable_[this->varDecl_->identifier_]);

			// check for ref type for store instruction
			ValidType * vtype = get<0>(ASTNode::varTable_[this->varDecl_->identifier_]);
			assert(vtype);
			if (vtype->GetVarType()==RefVarType) {
				ExistingVarNode * existingVarNode = (ExistingVarNode *)
						this->exprNode_->children_[0];
				llvm::AllocaInst * rhsPtr = get<1>(
					ASTNode::varTable_[existingVarNode->identifier_]);
				GlobalBuilder.CreateStore(rhsPtr, alloca);
			} else {
				GlobalBuilder.CreateStore(rhs, alloca);
			}
		}
			break;
		case 4:
		{
			this->exprNode_->GenerateCode(endBlock);
		}
			break;
		default:
			cout << "unhandled case in statement generate code" << endl;
			break;
		}
//		GlobalBuilder.CreateBr(endBlock);
		return ret;
	}

//	llvm::Value *
//	GetLLVMReturnValueRecursive() {
//		if (this->constructorCase_==2) { // a return expression statemnet
//			return this->exprNode_->GenerateCode(nullptr);
//		} else {
//			return ASTNode::GetLLVMReturnValueRecursive();
//		}
//	}
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
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Function * parentfunction = GlobalBuilder.GetInsertBlock()->getParent();
		llvm::BasicBlock * ret = llvm::BasicBlock::Create(GlobalContext, "stmts", parentfunction, endBlock);
		GlobalBuilder.SetInsertPoint(ret);
		vector<llvm::BasicBlock *> statementBlocks;
		for (int i=0; i<this->children_.size(); ++i) {
			llvm::BasicBlock * statementBlock = (llvm::BasicBlock *)
				this->children_[i]->GenerateCode(endBlock);
			statementBlocks.push_back(statementBlock);
		}

		for (int i=0; i<this->children_.size()-1; ++i) {
			GlobalBuilder.SetInsertPoint(statementBlocks[i]);
			GlobalBuilder.CreateBr(statementBlocks[i+1]);
		}
		GlobalBuilder.SetInsertPoint(ret);
		GlobalBuilder.CreateBr(statementBlocks[0]);
		return ret;
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
		llvm::Function *parentfunction = GlobalBuilder.GetInsertBlock()->getParent();
		llvm::BasicBlock * ret = llvm::BasicBlock::Create(GlobalContext, "blk", parentfunction, endBlock);
		GlobalBuilder.SetInsertPoint(ret);
		llvm::Value * blockUnder = this->children_[0]->GenerateCode(endBlock);
		GlobalBuilder.SetInsertPoint(ret);
		GlobalBuilder.CreateBr((llvm::BasicBlock *)blockUnder);
		return ret;

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

	llvm::Function *
	GenerateCode(llvm::BasicBlock * endBlock) {
		llvm::Type * llvmRetType = this->retType_->GetLLVMType();
		llvm::FunctionType * functionType = nullptr;
		if (this->constructorCase_==0) {
			functionType = llvm::FunctionType::get(llvmRetType, false);
		} else if (this->constructorCase_==1) {
			functionType = llvm::FunctionType::get(llvmRetType, this->llvmParamTypes_, false);
		}

		string name = this->identifier_;
		if (this->isRunFunc_) {
			name = "main";
		}
		llvm::Function * ret = llvm::Function::Create(
			functionType, llvm::Function::ExternalLinkage,
			name, GlobalModule);

		// set names for the arguments
		if (this->constructorCase_==1) {// there are arguments
			unsigned idx = 0;
			VdeclsNode * vdeclsNode = (VdeclsNode *)this->children_[0];
			for (auto &arg : ret->args()) {
				VdeclNode * vdeclNode = (VdeclNode *)vdeclsNode->children_[idx];
				arg.setName(vdeclNode->identifier_);
			}
		}

		// create a new basic block to start insertion into
		llvm::BasicBlock * entryBlock = llvm::BasicBlock::Create(GlobalContext, "entry", ret);
		GlobalBuilder.SetInsertPoint(entryBlock);
//		llvm::Value * retVal = this->blockNode_->GetLLVMReturnValueRecursive();
//
//		if (retVal != nullptr) {
//			GlobalBuilder.CreateRet(retVal);
//		} else {
//			GlobalBuilder.CreateRetVoid();
//		}

		llvm::BasicBlock * blockUnder = (llvm::BasicBlock *)
				this->blockNode_->GenerateCode(nullptr);
		GlobalBuilder.SetInsertPoint(entryBlock);
		GlobalBuilder.CreateBr(blockUnder);
		llvm::verifyFunction(*ret);
		return ret;
	}

	FuncNode(unsigned lineNumber,
			ValidType * retType, string identifier,
		BlockNode * blockNode) : ASTNode(lineNumber), retType_(retType),
				identifier_(identifier), blockNode_(blockNode),
				constructorCase_(0) {

		this->children_.push_back(blockNode);

		if(identifier == "run"){
			if(retType->varType_ != IntVarType){
				stringstream ss;
				ss << "error: line " << lineNumber << ": ";
				ss << "return type of run function has to be int. \n";
				ASTNode::compilerErrors_.push_back(ss.str());
			}
			this->isRunFunc_ = true;
			ASTNode::runDefined_ = true;
		}

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);
		CheckFuncTable(lineNumber, identifier);
		get<0>(ASTNode::funcTable_[identifier]) = vTypes;
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
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "run function can't take any arguments. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		}

		for(auto node : vdeclsNode->children_){
			VdeclNode * vNode = (VdeclNode *) node;
			ASTNode::varTable_.erase(vNode->identifier_);
		}

		if(retType->varType_== RefVarType){
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "function return type can't be ref. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		}

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);
		for (auto t : vdeclsNode->children_) {
			vTypes.push_back(((VdeclNode *)t)->type_);
			this->llvmParamTypes_.push_back(((VdeclNode *)t)->type_->GetLLVMType());
		}

		CheckFuncTable(lineNumber, identifier);

		get<0>(ASTNode::funcTable_[identifier]) = vTypes;
	}

	void
	CheckFuncTable(unsigned lineNumber, string identifier) {
		FuncTableEntry hit = ASTNode::funcTable_.find(identifier);
		if (hit != ASTNode::funcTable_.end()) {
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "function identifier '";
			ss << identifier;
			ss << "' already defined. \n";
			ASTNode::compilerErrors_.push_back(ss.str());
		} else {
			int checkRecursiveLn = get<1>(ASTNode::recursiveFuncPlaceHolder_);
			string checkRecursiveId = get<0>(ASTNode::recursiveFuncPlaceHolder_);
			if (checkRecursiveLn!=-1 && identifier!=checkRecursiveId) {
				stringstream ss;
				ss << "error: line " << checkRecursiveLn << ": ";
				ss << "function identifier ";
				ss << checkRecursiveId;
				ss << " not declared. \n";
				ASTNode::compilerErrors_.push_back(ss.str());
			}
		}
		get<0>(ASTNode::recursiveFuncPlaceHolder_) = "";
		get<1>(ASTNode::recursiveFuncPlaceHolder_) = -1;
	}

	virtual ~FuncNode() {
		if (this->retType_ != nullptr) {
			delete this->retType_;
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
