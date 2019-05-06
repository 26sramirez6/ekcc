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
	String,
	Int,
	Float,
	Boolean
};

enum UnaryOperationTypes {
	EmptyUnaryOperation,
	Not,
	Minus
};

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
    	noAlias_(noAlias), referredType_(referredType) {
			// Check: a ref type may not contain a 'ref' or 'void' type.
			if(referredType->GetName() == "ref"){
				cout << "error: ref type can't be ref." << endl;
			}else if(referredType->GetName() == "void"){
				cout << "error: ref type can't be void." << endl;
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
	Literal(int value) : type_(Int),
			name_("ilit"),
			value_(new LiteralValue()) {
		this->value_->iValue_ = value;
	}

	Literal(float value) : type_(Float),
			name_("flit"),
			value_(new LiteralValue()) {
		this->value_->fValue_ = value;
	}

	Literal(string value) : type_(String),
			name_("slit"),
			value_(new LiteralValue()) {
		this->value_->sValue_ = value;
	}

	Literal(bool value) : type_(Boolean),
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
