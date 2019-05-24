#ifndef VALIDTYPES_HPP_
#define VALIDTYPES_HPP_
#include <string.h>
#include <iostream>
#include <memory>
#include <assert.h>

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

#include "LLVMGlobals.hpp"

using std::cout;
using std::endl;
using std::string;
using std::unique_ptr;

enum ControlTypes {
	EmptyControl,
	If,
	IfElse,
	While,
	Return
};

enum BinaryOperationTypes {
	EmptyBinaryOperation,
	Assign,
	Cast,
	Multiply,
	Divide,
	Add,
	Subtract,
	Equality,
	LessThan,
	GreaterThan,
	Land,
	Lor,
};

enum LiteralTypes {
	EmptyLiteral,
	StringLiteral,
	IntLiteral,
	FloatLiteral,
	BooleanLiteral
};

enum VariableTypes {
	EmptyVarType,
	IntVarType,
	CintVarType,
	StringVarType,
	FloatVarType,
	BooleanVarType,
	RefVarType,
	VoidVarType,
};

enum UnaryOperationTypes {
	EmptyUnaryOperation,
	Not,
	Minus
};


struct ValidType {
	VariableTypes varType_;
	ValidType * referredType_ = nullptr;
	ValidType() : varType_(EmptyVarType) {}
	ValidType(VariableTypes varType) : varType_(varType) {}
	ValidType(VariableTypes varType, ValidType * referredType) :
		varType_(varType), referredType_(referredType) {}
	virtual ~ValidType() {}
	virtual string
	GetName() = 0;

	virtual llvm::Type *
	GetLLVMType() = 0;

	virtual VariableTypes
	GetVarType() {
		return this->varType_;
	}

	static VariableTypes
	ConvertLiteralToVariableType(LiteralTypes in) {
		switch (in) {
		case EmptyLiteral: return EmptyVarType;
		case StringLiteral: return StringVarType;
		case IntLiteral: return IntVarType;
		case FloatLiteral: return FloatVarType;
		case BooleanLiteral: return BooleanVarType;
		}
		return EmptyVarType;
	}

	static llvm::Type *
	ConvertVariableTypeToLLVMType(VariableTypes in) {
		switch (in) {
		case EmptyVarType: return nullptr;
		case IntVarType: return llvm::Type::getInt32Ty(GlobalContext);
		case CintVarType: return llvm::Type::getInt32Ty(GlobalContext);
		case StringVarType: return nullptr;
		case FloatVarType: return llvm::Type::getFloatTy(GlobalContext);
		case BooleanVarType: return llvm::Type::getInt1Ty(GlobalContext);
		case RefVarType: return nullptr;
		case VoidVarType: return llvm::Type::getVoidTy(GlobalContext);
		}
		return nullptr;
	}

	static bool
	IsValidBinaryOp(ValidType * validTypeL, ValidType * validTypeR) {
		if (validTypeL==nullptr || validTypeR==nullptr) {
			return false;
		}

		VariableTypes l = validTypeL->varType_;
		VariableTypes r = validTypeR->varType_;
		if (l==RefVarType) {
			l = validTypeL->referredType_->varType_;
		}

		if (r==RefVarType) {
			r = validTypeR->referredType_->varType_;
		}

		if (l!=r) {
			return false;
		} else if (
				l==VoidVarType ||
				l==StringVarType ||
				l==EmptyVarType ||
				l==BooleanVarType) {
			return false;
		}
		return true;
	}

	static bool
	IsNumericType(ValidType * type) {
		return (type!=nullptr &&
				(type->varType_==FloatVarType ||
				 type->varType_==IntVarType ||
				 type->varType_==CintVarType));
	}

	static bool
	IsValidCast(ValidType * castTo, ValidType * castFrom) {
		if (castTo==nullptr || castFrom==nullptr) {
			return false;
		} else if (castTo->varType_==BooleanVarType && castTo->varType_==BooleanVarType) {
			return true;
		} else if (!IsNumericType(castTo) || !IsNumericType(castTo)) {
			return false;
		}
		return true;
	}

	static ValidType *
	GetUnderlyingType(ValidType * in) {
		assert(in);
		if (in==nullptr) return nullptr;
		if (in->varType_==RefVarType) {
			return in->referredType_;
		}
		return in;
	}

};

struct IntType : public ValidType {
	IntType() : ValidType(IntVarType) {}
	string
	GetName() {
		return string("int");
	}

	llvm::Type *
	GetLLVMType() {
		return llvm::Type::getInt32Ty(GlobalContext);
	}

};

struct FloatType : public ValidType {
	FloatType() : ValidType(FloatVarType) {}
	string
	GetName() {
		return string("float");
	}

	llvm::Type *
	GetLLVMType() {
		return llvm::Type::getFloatTy(GlobalContext);
	}
};

struct CintType : public ValidType {
	CintType() : ValidType(CintVarType) {}
	string
	GetName() {
		return string("cint");
	}

	llvm::Type *
	GetLLVMType() {
		return llvm::Type::getInt32Ty(GlobalContext);
	}
};

struct BoolType : public ValidType {
	BoolType() : ValidType(BooleanVarType) {}
	string
	GetName() {
		return string("bool");
	}

	llvm::Type *
	GetLLVMType() {
		return llvm::Type::getInt1Ty(GlobalContext);
	}
};

struct VoidType : public ValidType {
	VoidType() : ValidType(VoidVarType) {}
	string
	GetName() {
		return string("void");
	}

	llvm::Type *
	GetLLVMType() {
		return llvm::Type::getVoidTy(GlobalContext);
	}
};

struct RefType : public ValidType {
    bool noAlias_ = false;
	bool invalidConstructor_ = false;

    RefType() : ValidType(RefVarType) {}
    RefType(bool noAlias, ValidType * referredType) :
    	ValidType(RefVarType, referredType), noAlias_(noAlias) {
		// Check: a ref type may not contain a 'ref' or 'void' type.
		this->invalidConstructor_ = referredType->varType_ == RefVarType ||
			referredType->varType_ == VoidVarType;
	}

    ~RefType() {
    	if (this->referredType_ != nullptr) {
    		delete this->referredType_;
    	}
    }

    string
	GetName() {
    	if (noAlias_) {
			return string("noalias ref ") + this->referredType_->GetName();
		// To avoid seg fault when "ref ref"
		}else if(this->referredType_ == nullptr){
			return string("ref");
		}else{
			return string("ref ") + this->referredType_->GetName();
		}
	}

    llvm::Type *
	GetLLVMType() {
    	llvm::Type * ret = nullptr;
    	switch (this->referredType_->varType_) {
    	case IntVarType:
    		ret = llvm::Type::getInt32PtrTy(GlobalContext, 0);
    		break;
    	case CintVarType:
    		ret = llvm::Type::getInt32PtrTy(GlobalContext, 0);
    		break;
    	case StringVarType:
    		break;
    	case FloatVarType:
    		ret = llvm::Type::getFloatPtrTy(GlobalContext, 0);
    		break;
    	case BooleanVarType:
    		ret = llvm::Type::getInt1PtrTy(GlobalContext, 0);
    		break;
    	case VoidVarType:
    		break;
    	case EmptyVarType:
    		break;
    	case RefVarType:
    		break;
    	}
		return ret;
	}

};

struct ControlFlow {
	ControlTypes controlType_;
	ControlFlow(): controlType_(EmptyControl) {}
	ControlFlow(ControlTypes controlType): controlType_(controlType) {}
};

struct IfControl: ControlFlow {
	IfControl() : ControlFlow(If) {}
};

struct ElseControl : ControlFlow {
	ElseControl() : ControlFlow(IfElse) {}
};

struct WhileControl : ControlFlow {
	WhileControl() : ControlFlow(While) {}
};

struct ReturnControl : ControlFlow {
	ReturnControl() : ControlFlow(Return) {}
};

struct LiteralValue {
	int iValue_;
	float fValue_;
	string sValue_;
	bool bValue_;
};

struct Literal {
	LiteralTypes type_ = EmptyLiteral;
	string name_;
	unique_ptr<LiteralValue> value_;
	Literal(int value) : type_(IntLiteral),
			name_("ilit"),
			value_(new LiteralValue()) {
		this->value_->iValue_ = value;
	}

	Literal(float value) : type_(FloatLiteral),
			name_("flit"),
			value_(new LiteralValue()) {
		this->value_->fValue_ = value;
	}

	Literal(string value) : type_(StringLiteral),
			name_("slit"),
			value_(new LiteralValue()) {
		this->value_->sValue_ = value;
	}

	Literal(bool value) : type_(BooleanLiteral),
			name_("blit"),
			value_(new LiteralValue()) {
		this->value_->bValue_ = value;
	}

	static ValidType *
	ConvertToValidType(Literal * in) {
		switch (in->type_) {
		case EmptyLiteral: return nullptr;
		case StringLiteral: return nullptr;
		case IntLiteral: return new IntType();
		case FloatLiteral: return new FloatType();
		case BooleanLiteral: return new BoolType();
		}
		return nullptr;
	}

	string
	GetName() {
		return this->name_;
	}

	unique_ptr<LiteralValue>&
	GetValue() {
		return this->value_;
	}
};

struct Function {};
struct PrintFunction : Function {};
struct RunFunction : Function {};
struct DefFunction : Function {};
struct ExternFunction : Function {};

#endif
