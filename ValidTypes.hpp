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


enum ControlTypes { Empty, If, IfElse, While, Return };
struct ControlFlow {
	ControlTypes controlType_ = Empty;
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
	String,
	Int,
	Float,
	False,
	True
};

enum OperationTypes {
	Assign,
	Cast,
	Multiply,
	Divide,
	Add,
	Subtract,
	Equality,
	Lessthan,
	GreaterThan,
	Land,
	Lor,
	Negate,
	Negative
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
