#ifndef VALIDTYPES_HPP_
#define VALIDTYPES_HPP_
#include <string.h>
#include <iostream>
#include <memory>

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
	IntVarType,
	CintVarType,
	StringVarType,
	FloatVarType,
	BooleanVarType,
	RefVarType,
	VoidVarType
};

enum UnaryOperationTypes {
	EmptyUnaryOperation,
	Not,
	Minus
};

struct ValidType {
	virtual string
	GetName() = 0;
	VariableTypes varType_;
	virtual ~ValidType() {}
};

struct IntType : public ValidType {
	VariableTypes varType_ = IntVarType;
	string
	GetName() {
		return string("int");
	}
};

struct FloatType : public ValidType {
	VariableTypes varType_ = FloatVarType;
	string
	GetName() {
		return string("float");
	}
};

struct CintType : public ValidType {
	VariableTypes varType_ = CintVarType;
	string
	GetName() {
		return string("cint");
	}
};

struct BoolType : public ValidType {
	VariableTypes varType_ = BooleanVarType;
	string
	GetName() {
		return string("bool");
	}
};

struct VoidType : public ValidType {
	VariableTypes varType_ = VoidVarType;
	string
	GetName() {
		return string("void");
	}
};

struct RefType : public ValidType {
	VariableTypes varType_ = RefVarType;
    bool noAlias_ = false;
    ValidType * referredType_ = nullptr;

    RefType() {}
    RefType(bool noAlias, ValidType * referredType) :
    	noAlias_(noAlias), referredType_(referredType) {
		// Check: a ref type may not contain a 'ref' or 'void' type.
		if(referredType->varType_ == RefVarType){
			cout << "error: ref type point to another ref." << endl;
		}else if(referredType->varType_ == VoidVarType){
			cout << "error: ref type can't be void." << endl;
		}
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

};

struct ControlFlow {
	ControlTypes controlType_ = EmptyControl;
};

struct IfControl: ControlFlow {
	ControlTypes controlType_ = If;
};

struct ElseControl : ControlFlow {
	ControlTypes controlType_ = IfElse;
};

struct WhileControl : ControlFlow {
	ControlTypes controlType_ = While;
};

struct ReturnControl : ControlFlow {
	ControlTypes controlType_ = Return;
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
