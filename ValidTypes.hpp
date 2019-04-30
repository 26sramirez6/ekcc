#ifndef VALIDTYPES_HPP_
#define VALIDTYPES_HPP_
#include <string.h>
#include <iostream>

using std::cout;
using std::endl;
using std::string;

struct ValidType {
	virtual string
	GetName() = 0;

	virtual ~ValidType() {}
};

struct IntType : public ValidType {

	string
	GetName() {
		return string("int");
	}
};

struct FloatType : public ValidType {

	string
	GetName() {
		return string("float");
	}
};

struct CintType : public ValidType {

	string
	GetName() {
		return string("cint");
	}
};

struct BoolType : public ValidType {

	string
	GetName() {
		return string("bool");
	}
};

struct VoidType : public ValidType {
	string
	GetName() {
		return string("void");
	}
};

struct RefType : public ValidType {
    bool noAlias_ = false;
    ValidType * referredType_ = nullptr;
    RefType() {}
    RefType(bool noAlias, ValidType * referredType) :
    	noAlias_(noAlias), referredType_(referredType) {}

    string
	GetName() {
    	if (noAlias_) return string("noalias ref ") + this->referredType_->GetName();
    	return string("ref") + this->referredType_->GetName();
	}
};


enum ControlTypes {
	Empty,
	If,
	IfElse,
	While,
	Return
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

enum LiteralTypes {
	EmptyLiteral,
	String,
	Int,
	Float,
	Boolean
};

union LiteralValue {
	int iValue_ = 0;
	float fValue_ = 0.;
	string sValue_;
	bool bValue_ = false;
};

struct Literal {
	LiteralTypes type_ = EmptyLiteral;
	string name_;
	LiteralValue value_;
	Literal(int value) : type_(Int), name_("ilit") {
		this->value_.iValue_ = value;
	}

	Literal(float value) : type_(Float), name_("flit") {
		this->value_.fValue_ = value;
	}

	Literal(string value) : type_(String), name_("slit") {
		this->value_.sValue_ = value;
	}

	Literal(bool value) : type_(Boolean), name_("blit") {
		this->value_.bValue_ = value;
	}

	string
	GetName() {
		return this->name_;
	}

	LiteralValue
	GetValue() {
		return this->value_;
	}
};

enum OperationTypes {
	EmptyOperation,
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
	Not,
	Minus
};
//struct LiteralType {
//	LiteralTypes type_;
//	LiteralType(int type) : type_(type) {
//
//	}
//};



struct Function {

};

struct PrintFunction : Function {

};

struct RunFunction : Function {

};

struct DefFunction : Function {

};

struct ExternFunction : Function {

};

#endif
