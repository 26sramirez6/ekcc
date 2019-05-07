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
#include<assert.h>
#include "ValidTypes.hpp"
using std::vector;
using std::string;
using std::move;
using std::stringstream;
using std::unordered_map;

typedef std::unordered_map<string, ValidType *> VarTable;
typedef std::unordered_map<string, ValidType *>::const_iterator VarTableEntry;
typedef std::unordered_map<string, vector< ValidType * > > FuncTable;
typedef std::unordered_map<string, vector< ValidType * > >::const_iterator FuncTableEntry;
//typedef std::vector<VarTable> ScopeStack;

struct ASTNode {
	static ASTNode * root_;
	static bool ready_;
	static vector< string > compilerErrors_;
	static vector< int > lineNumberErrors_;
	static FuncTable funcScope_;
	static VarTable varTable_;
	vector< ASTNode * > children_;
	unsigned lineNumber_;

	ASTNode() : lineNumber_(0) {}
	ASTNode(unsigned lineNumber) : lineNumber_(lineNumber) {}

	virtual void
	PrintRecursive(stringstream& ss, unsigned depth) {};

	virtual ~ASTNode() {
		for (auto n : this->children_) {
			delete n;
		}
	}

	static void
	StaticInit() {
		if (!ready_) {
			ready_ = true;
		}
	}
};

struct ProgramNode : public ASTNode {
	ProgramNode(unsigned lineNumber, ASTNode * funcsNode) :
		ASTNode(lineNumber) {
		this->children_.push_back(funcsNode);
		this->root_ = this;
	}

	ProgramNode(unsigned lineNumber, ASTNode * externs,
			ASTNode * funcsNode) : ASTNode(lineNumber){
		this->children_.push_back(funcsNode);
		this->children_.push_back(externs);
		this->root_ = this;
	}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		ss << "---" << '\n';
		ss << "name: prog" << '\n';
		for (auto node : this->children_) {
			node->PrintRecursive(ss, depth+1);
		}
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
		type_(type), identifier_(identifier.substr(1, string::npos)),
		ASTNode(lineNumber) {
//		VarTable top = ASTNode::scopeStack_.back();
		if (ASTNode::varTable_.end() != ASTNode::varTable_.find(identifier_)) {
			stringstream ss;
			ss << "error: line " << lineNumber;
			ss << ": variable identifier ";
			ss << identifier_;
			ss << " already defined\n";
			ASTNode::compilerErrors_.push_back(ss.str());
		}
		ASTNode::varTable_[identifier_] = type;

		// Check: <vdecl> may not have void type.
		if(type->varType_ == VoidVarType){
			stringstream ss;
			ss << "error: line " << lineNumber;
			ss << ": variable identifier ";
			ss << identifier_;
			ss << " cannot be void\n";
			this->compilerErrors_.push_back(ss.str());
		}

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

struct TdeclsNode : public ASTNode {
	ValidType * paramType_ = nullptr;
	vector<ValidType *> paramTypes_;
	TdeclsNode(unsigned lineNumber, ValidType * validType):
		paramType_(validType), ASTNode(lineNumber)  {
		this->paramTypes_.push_back(validType);
	}

	TdeclsNode(unsigned lineNumber, TdeclsNode * tdeclsNode,
			ValidType * validType):
		paramType_(validType), ASTNode(lineNumber) {
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
		retType_(retType), identifier_(identifier),
		ASTNode(lineNumber) { }

	ExternNode(unsigned lineNumber,
			ValidType * retType,
			string identifier,
			TdeclsNode * tdeclsNode) :
		retType_(retType), identifier_(identifier),
		ASTNode(lineNumber) {
		this->children_.push_back(tdeclsNode);

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
	UnaryOperationNode(unsigned lineNumber,
			UnaryOperationTypes operationType,
			ASTNode * expressionNode1) :
		operationType_(operationType), ASTNode(lineNumber) {
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

	BinaryOperationNode(unsigned lineNumber,
			BinaryOperationTypes operationType,
			ASTNode * expressionNode1,
			ASTNode * expressionNode2) :
		operationType_(operationType), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode1);
		this->children_.push_back(expressionNode2);
	}

	BinaryOperationNode(unsigned lineNumber,
			BinaryOperationTypes operationType,
			ValidType * validType,
			ASTNode * expressionNode1) :
		operationType_(Cast),
		typeCast_(validType), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode1);
	}

	virtual ~BinaryOperationNode() {
		if (this->typeCast_ != nullptr) {
			delete this->typeCast_;
		}
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

struct ExistingVarNode: public ASTNode {
	string identifier_;
	ExistingVarNode(unsigned lineNumber, string identifier) :
	identifier_(identifier.substr(1, string::npos)), ASTNode(lineNumber) {
//		VarTable top = ASTNode::scopeStack_.back();
		VarTableEntry hit = ASTNode::varTable_.find(this->identifier_);
		if (hit==ASTNode::varTable_.end()) {
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "variable identifier ";
			ss << identifier_;
			ss << " not declared\n";
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
};


struct ExistingFuncNode: public ASTNode {
	string identifier_;
	ExistingFuncNode(unsigned lineNumber, string identifier) :
	identifier_(identifier), ASTNode(lineNumber) {}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		ss << left1 << "name: funccall" << '\n';
		ss << left1 << "globid: " << this->identifier_ << '\n';
	}

};


struct ExpressionNode: public ASTNode {
	int constructorCase_ = 0;
	Literal * literal_ = nullptr;

	ExpressionNode(unsigned lineNumber,
			ExpressionNode * expressionNode) :
		constructorCase_(0), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
	}

	ExpressionNode(unsigned lineNumber,
			BinaryOperationNode * binaryOperationNode) :
		constructorCase_(1), ASTNode(lineNumber) {
		this->children_.push_back(binaryOperationNode);
	}

	ExpressionNode(unsigned lineNumber,
			UnaryOperationNode * unaryOperationNode) :
		constructorCase_(2), ASTNode(lineNumber) {
		this->children_.push_back(unaryOperationNode);
	}

	ExpressionNode(unsigned lineNumber,
			Literal * literal) :
		constructorCase_(3), literal_(literal),
		ASTNode(lineNumber) {}

	ExpressionNode(unsigned lineNumber,
			ExistingVarNode * existingVarNode) :
		constructorCase_(4), ASTNode(lineNumber) {
		this->children_.push_back(existingVarNode);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingFuncNode * existingFuncNode) :
		constructorCase_(5), ASTNode(lineNumber) {
		this->children_.push_back(existingFuncNode);
	}

	ExpressionNode(unsigned lineNumber,
			ExistingFuncNode * existingFuncNode,
			ASTNode * expressionsNode) :
		constructorCase_(6), ASTNode(lineNumber) {
		this->children_.push_back(existingFuncNode);
		for (auto node : expressionsNode->children_) {
			this->children_.push_back(node);
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
		stmtName_("blkstmt"), constructorCase_(0),
		ASTNode(lineNumber) {
		this->children_.push_back(blockNode);
	}

	StatementNode(unsigned lineNumber, ReturnControl * returnControl) :
			controlFlow_(returnControl),
			stmtName_("retstmt"),
			constructorCase_(1), ASTNode(lineNumber) {}

	StatementNode(unsigned lineNumber,
			ReturnControl * returnControl,
			ExpressionNode * expressionNode) :
		controlFlow_(returnControl),
		stmtName_("retstmt"),
		constructorCase_(2), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
		// Check: a function may not return a ref type.

	}

	StatementNode(unsigned lineNumber,
			ASTNode * vdeclNode,
			ExpressionNode * expressionNode) :
			stmtName_("vardeclstmt"),
			constructorCase_(3), ASTNode(lineNumber) {
		this->varDecl_ = dynamic_cast<VdeclNode *>(vdeclNode);
		this->children_.push_back(vdeclNode);
		this->children_.push_back(expressionNode);
	}

	StatementNode(unsigned lineNumber,
			ExpressionNode * expressionNode) :
		stmtName_("expstmt"),
		constructorCase_(4), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
	}

	StatementNode(unsigned lineNumber,
			WhileControl * whileControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode) :
			controlFlow_(whileControl),
			stmtName_("whilestmt"),
			constructorCase_(5), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode);
	}

	StatementNode(unsigned lineNumber,
			IfControl * ifControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode) :
			controlFlow_(ifControl),
			stmtName_("ifstmt"),
			constructorCase_(6), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode);
	}

	StatementNode(unsigned lineNumber,
			ElseControl * elseControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode1,
			StatementNode * statementNode2) :
			controlFlow_(elseControl),
			stmtName_("ifstmt"),
			constructorCase_(7), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode1);
		this->children_.push_back(statementNode2);
	}

	StatementNode(unsigned lineNumber,
			PrintFunction * printFunction,
			ExpressionNode * expressionNode) :
			function_(printFunction),
			stmtName_("printstmt"),
			constructorCase_(8), ASTNode(lineNumber) {
		this->children_.push_back(expressionNode);
	}

	StatementNode(unsigned lineNumber,
			PrintFunction * printFunction,
			string stringLiteral) :
			function_(printFunction),
			stmtName_("printslit"),
			printStringLiteral_(stringLiteral),
			constructorCase_(9), ASTNode(lineNumber) {
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
};

struct StatementsNode: public ASTNode {
	StatementsNode(unsigned lineNumber,
			StatementNode * statementNode) : ASTNode(lineNumber) {
		// first statement in a block, push a map
//		VarTable varTable;
//		ASTNode::scopeStack_.push_back(varTable);
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
//		assert(!ASTNode::scopeStack_.empty());
//		if (!ASTNode::scopeStack_.empty()) {
//			ASTNode::scopeStack_.pop_back();
//		}
	}

	BlockNode(unsigned lineNumber, ASTNode * statementsNode) :
		ASTNode(lineNumber) {

		this->children_.push_back(statementsNode);

//		assert(!ASTNode::scopeStack_.empty());
//		if (!ASTNode::scopeStack_.empty()) {
//			ASTNode::scopeStack_.pop_back();
//		}

		for(auto node : statementsNode->children_){
			StatementNode * sNode = (StatementNode *) node;
			if(sNode->constructorCase_ == 3){
				ASTNode::varTable_.erase(sNode->varDecl_->identifier_);
			}
		}

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

	FuncNode(unsigned lineNumber,
			ValidType * retType, string identifier,
		BlockNode * blockNode) : retType_(retType),
				identifier_(identifier), ASTNode(lineNumber) {
		this->children_.push_back(blockNode);

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);
		ASTNode::funcScope_[identifier] = vTypes;
	}

	FuncNode(unsigned lineNumber,
			ValidType * retType, string identifier,
			 VdeclsNode * vdeclsNode, BlockNode * blockNode) :
			retType_(retType), identifier_(identifier),
			ASTNode(lineNumber) {

		this->children_.push_back(vdeclsNode);
		this->children_.push_back(blockNode);

		for(auto node : vdeclsNode->children_){
			VdeclNode * vNode = (VdeclNode *) node;
			ASTNode::varTable_.erase(vNode->identifier_);
		}

		vector< ValidType * > vTypes;
		vTypes.push_back(retType);
		for (auto t : vdeclsNode->children_) {
			vTypes.push_back(((VdeclNode *)t)->type_);
		}

		FuncTableEntry hit = ASTNode::funcScope_.find(identifier);
		if (hit != ASTNode::funcScope_.end()) {
			stringstream ss;
			ss << "error: line " << lineNumber << ": ";
			ss << "function identifier '";
			ss << identifier;
			ss << "' already defined\n";
			ASTNode::compilerErrors_.push_back(ss.str());
		}

		ASTNode::funcScope_[identifier] = vTypes;
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
	};

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		PrintSelf(ss, depth);
		for (auto node : this->children_) {
			node->PrintRecursive(ss, depth+1);
		}
	};
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
