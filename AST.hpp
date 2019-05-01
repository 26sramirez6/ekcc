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
#include<assert.h>
#include "ValidTypes.hpp"
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::move;


struct ASTNode {
	vector<ASTNode *> children_;

	virtual void
	PrintRecursive(unsigned depth) {};

	virtual ~ASTNode() {};

};


struct ProgramNode : public ASTNode {
	ProgramNode(ASTNode * funcsNode) {
		this->children_.push_back(funcsNode);
	}

	ProgramNode(ASTNode * externs, ASTNode * funcsNode) {
		this->children_.push_back(funcsNode);
		this->children_.push_back(externs);
	}

	void
	PrintRecursive(unsigned depth) {
		cout << "---" << endl;
		cout << "name: prog" << endl;
		for (auto node : this->children_) {
			node->PrintRecursive(depth+1);
		}
	};
};

struct TdeclsNode : public ASTNode {
	ValidType * paramType_;
	vector<ValidType *> paramTypes_;
	TdeclsNode(ValidType * validType):
		paramType_(validType) {
		this->paramTypes_.push_back(validType);
	}

	TdeclsNode(TdeclsNode * tdeclsNode, ValidType * validType):
		paramType_(validType) {
		for (auto type : tdeclsNode->paramTypes_) {
			this->paramTypes_.push_back(type);
		}
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		cout << left1 << "tdecls:" << endl;
		cout << left2 << "name: tdecls" << endl;
		cout << left2 << "types:" << endl;
		for (auto type: this->paramTypes_) {
			cout << left3 << "- " << type->GetName() << endl;
		}
	};
};

struct ExternNode : public ASTNode {
	ValidType * retType_;
	string identifier_;
	ExternNode(ValidType * retType, string identifier) :
		retType_(retType), identifier_(identifier) {

	}

	ExternNode(ValidType * retType, string identifier, TdeclsNode * tdeclsNode) :
		retType_(retType), identifier_(identifier) {
		this->children_.push_back(tdeclsNode);

	}

	void
	PrintRecursive(unsigned depth) {
		string left = std::string(depth*2, ' ');
		cout << left << "name: extern" << endl;
		cout << left << "ret_type: " << this->retType_->GetName() << endl;
		cout << left << "globid: " << this->identifier_ << endl;
		if (this->children_.size()>0) {
			this->children_[0]->PrintRecursive(depth+1);
		}
	}
};

struct ExternsNode : public ASTNode {

	ExternsNode(ExternNode * externNode) {
		this->children_.push_back(externNode);
	}

	ExternsNode(ExternsNode * externsNode, ExternNode * externNode) {
		for (auto node : externsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(externNode);
	}

	void
	PrintSelf(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		cout << left1 << "externs:" << endl;
		cout << left2 << "name: externs" << endl;
		cout << left2 << "externs:" << endl;
		cout << left3 << "-" << endl;
	}

	void
	PrintRecursive(unsigned depth) {
		PrintSelf(depth);
		string left3 = std::string((depth+1)*2, ' ');
		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(depth+2);
			if (i!=this->children_.size()-1)
				cout << left3 << "-" << endl;
		}
	};

};

struct FuncNode : public ASTNode {

	ValidType * retType_;
	string identifier_;

	FuncNode(ValidType * retType, string identifier,
		ASTNode * blockNode) : retType_(retType), identifier_(identifier){
		this->children_.push_back(blockNode);
	}

	FuncNode(ValidType * retType, string identifier,
		ASTNode * vdeclsNode, ASTNode * blockNode) :
			retType_(retType), identifier_(identifier) {
		this->children_.push_back(vdeclsNode);
		this->children_.push_back(blockNode);
	}

	void
	PrintSelf(unsigned depth) {
		string left = std::string(depth*2, ' ');
		cout << left << "name: func" << endl;
		cout << left << "ret_type: " << this->retType_->GetName() << endl;
		cout << left << "globid: " << this->identifier_ << endl;
	};

	void
	PrintRecursive(unsigned depth) {
		PrintSelf(depth);
		for (auto node : this->children_) {
			node->PrintRecursive(depth+1);
		}
	};
};

struct FuncsNode : public ASTNode {
	FuncsNode(FuncNode * funcNode) {
		this->children_.push_back(funcNode);
	}

	FuncsNode(FuncsNode * funcsNode, FuncNode * funcNode) {
		for (auto node : funcsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(funcNode);
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		cout << left1 << "funcs:" << endl;
		cout << left2 << "name: funcs" << endl;
		cout << left2 << "funcs:" << endl;
		cout << left3 << "-" << endl;
		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(depth+2);
			if (i!=this->children_.size()-1)
				cout << left3 << "-" << endl;
		}
	}
};

struct BlockNode: public ASTNode {

	BlockNode() {
	}

	BlockNode(ASTNode * statementsNode) {
		this->children_.push_back(statementsNode);
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		cout << left1 << "blk:" << endl;
		cout << left2 << "name: blk" << endl;
		if (this->children_.size()>0) {
			cout << left2 << "contents:"<< endl;
			this->children_[0]->PrintRecursive(depth+1);
		}
	}
};


struct ExistingVarNode: public ASTNode {
	string identifier_;
	ExistingVarNode(string identifier) :
	identifier_(identifier) {}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		cout << left1 << "name: varval" << endl;
		cout << left1 << "var: " <<
		this->identifier_.substr(1, string::npos) << endl;
	}
};


struct ExistingFuncNode: public ASTNode {
	string identifier_;
	ExistingFuncNode(string identifier) :
	identifier_(identifier) {}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		cout << left1 << "name: funccall" << endl;
		cout << left1 << "globid: " << this->identifier_ << endl;
	}

};


struct UnaryOperationNode: public ASTNode {
	UnaryOperationTypes operationType_;
	UnaryOperationNode(UnaryOperationTypes operationType,
		ASTNode * expressionNode1) :
		operationType_(operationType) {
		this->children_.push_back(expressionNode1);
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		cout << left1 << "name: uop" << endl;
		cout << left1 << "op: ";
		switch (this->operationType_) {
		case Not:
			cout << "not" << endl;
			break;
		case Minus:
			cout << "minus" << endl;
			break;
#ifndef NO_DEBUG
		case EmptyUnaryOperation:
			assert(false);
			break;
#endif
		}
		this->children_[0]->PrintRecursive(depth+1);
	}
};

struct BinaryOperationNode: public ASTNode {
	BinaryOperationTypes operationType_;
	ValidType * typeCast_ = nullptr;

	BinaryOperationNode(BinaryOperationTypes operationType,
		ASTNode * expressionNode1,
		ASTNode * expressionNode2) :
		operationType_(operationType) {
		this->children_.push_back(expressionNode1);
		this->children_.push_back(expressionNode2);
	}

	BinaryOperationNode(BinaryOperationTypes operationType,
		ValidType * validType,
		ASTNode * expressionNode1) :
		operationType_(Cast),
		typeCast_(validType) {
		this->children_.push_back(expressionNode1);
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		cout << left1 << "name: binop" << endl;
		cout << left1 << "op: ";
		switch (this->operationType_) {
		case Assign:
			cout << "assign" << endl;
			break;
		case Cast:
			cout << "cast" << endl;
			cout << "type: " << this->typeCast_->GetName() << endl;
			this->children_[0]->PrintRecursive(depth+1);
			return;
		case Multiply:
			cout << "mul" << endl;
			break;
		case Divide:
			cout << "div" << endl;
			break;
		case Add:
			cout << "add" << endl;
			break;
		case Subtract:
			cout << "sub" << endl;
			break;
		case Equality:
			cout << "eq" << endl;
			break;
		case LessThan:
			break;
		case GreaterThan:
			cout << "gt" << endl;
			break;
		case Land:
			cout << "and" << endl;
			break;
		case Lor:
			cout << "or" << endl;
			break;
#ifndef NO_DEBUG
		case EmptyBinaryOperation:
			assert(false);
			break;
#endif
		}
		cout << left1 << "lhs:" << endl;
		this->children_[0]->PrintRecursive(depth+1);
		cout << left1 << "rhs:" << endl;
		this->children_[1]->PrintRecursive(depth+1);
	}
};

struct ExpressionNode: public ASTNode {
	int constructorCase_ = 0;
	Literal * literal_ = nullptr;

	ExpressionNode(ExpressionNode * expressionNode) :
		constructorCase_(0) {
		this->children_.push_back(expressionNode);
	}

	ExpressionNode(BinaryOperationNode * binaryOperationNode) :
		constructorCase_(1) {
		this->children_.push_back(binaryOperationNode);
	}

	ExpressionNode(UnaryOperationNode * unaryOperationNode) :
		constructorCase_(2) {
		this->children_.push_back(unaryOperationNode);
	}

	ExpressionNode(Literal * literal) :
		constructorCase_(3), literal_(literal) {}

	ExpressionNode(ExistingVarNode * existingVarNode) :
		constructorCase_(4) {
		this->children_.push_back(existingVarNode);
	}

	ExpressionNode(ExistingFuncNode * existingFuncNode) :
		constructorCase_(5) {
		this->children_.push_back(existingFuncNode);
	}

	ExpressionNode(ExistingFuncNode * existingFuncNode,
			ASTNode * expressionsNode) :
		constructorCase_(6) {
		this->children_.push_back(existingFuncNode);
		for (auto node : expressionsNode->children_) {
			this->children_.push_back(node);
		}
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		cout << left1 << "exp:" << endl;
		switch (this->constructorCase_) {
		case 0:
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 1:
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 2:
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 3:
		{
			cout << left2 << "name: " << literal_->GetName() << endl;
			cout << left2 << "value: ";
			unique_ptr<LiteralValue>& lv = literal_->GetValue();
			switch (literal_->type_) {
			case Int:
				cout << lv->iValue_ << endl;
				break;
			case Float:
				cout << lv->fValue_ << endl;
				break;
			case String:
				cout << lv->sValue_.substr(1, string::npos - 1) << endl;
				break;
			case Boolean:
				cout << lv->bValue_ << endl;
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
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 5:
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 6:
			this->children_[0]->PrintRecursive(depth+1);
			cout << left2 << "params:" << endl;
			this->children_[1]->PrintRecursive(depth+1);
			break;
		}
	}

};


struct ExpressionsNode: public ASTNode {
	ExpressionsNode(ExpressionNode * expressionNode) {
		this->children_.push_back(expressionNode);
	}

	ExpressionsNode(ExpressionsNode * expressionsNode,
			ExpressionNode * expressionNode) {
		for (auto node : expressionsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(expressionNode);
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		string left2 = std::string((depth+1)*2, ' ');
		cout << left1 << "name: exps" << endl;
		cout << left1 << "exps:" << endl;
		cout << left2 << "-" << endl;

		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(depth+2);
			if (i!=this->children_.size()-1)
				cout << left2 << "-" << endl;
		}
	}

};


struct StatementNode: public ASTNode {
	ControlFlow * controlFlow_ = nullptr;
	Function * function_ = nullptr;
	string stmtName_;
	string printStringLiteral_;
	int constructorCase_ = 0;

	StatementNode(BlockNode * blockNode) :
		stmtName_("blkstmt"), constructorCase_(0) {
		this->children_.push_back(blockNode);
	}

	StatementNode(ReturnControl * returnControl) :
			controlFlow_(returnControl),
			stmtName_("retstmt"),
			constructorCase_(1) {}

	StatementNode(ReturnControl * returnControl,
			ExpressionNode * expressionNode) :
		controlFlow_(returnControl),
		stmtName_("retstmt"),
		constructorCase_(2) {
		this->children_.push_back(expressionNode);
	}

	StatementNode(ASTNode * vdeclNode,
			ExpressionNode * expressionNode) :
			stmtName_("vardeclstmt"),
			constructorCase_(3) {
		this->children_.push_back(vdeclNode);
		this->children_.push_back(expressionNode);
	}

	StatementNode(ExpressionNode * expressionNode) :
		stmtName_("expstmt"),
		constructorCase_(4) {
		this->children_.push_back(expressionNode);
	}

	StatementNode(WhileControl * whileControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode) :
			controlFlow_(whileControl),
			stmtName_("whilestmt"),
			constructorCase_(5) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode);
	}

	StatementNode(IfControl * ifControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode) :
			controlFlow_(ifControl),
			stmtName_("ifstmt"),
			constructorCase_(6) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode);
	}

	StatementNode(ElseControl * elseControl,
			ExpressionNode * expressionNode,
			StatementNode * statementNode1,
			StatementNode * statementNode2) :
			controlFlow_(elseControl),
			stmtName_("ifstmt"),
			constructorCase_(7) {
		this->children_.push_back(expressionNode);
		this->children_.push_back(statementNode1);
		this->children_.push_back(statementNode2);
	}

	StatementNode(PrintFunction * printFunction,
			ExpressionNode * expressionNode) :
			function_(printFunction),
			stmtName_("printstmt"),
			constructorCase_(8) {
		this->children_.push_back(expressionNode);
	}

	StatementNode(PrintFunction * printFunction,
			string stringLiteral) :
			function_(printFunction),
			stmtName_("printslit"),
			printStringLiteral_(stringLiteral),
			constructorCase_(9) {
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		cout << left1 << "name: " << this->stmtName_ << endl;

		switch (this->constructorCase_) {
		case 0: // block
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 1: // return
			break;
		case 2: // return
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 3: // vdecl
			this->children_[0]->PrintRecursive(depth+1);
			this->children_[1]->PrintRecursive(depth+1);
			break;
		case 4: //exp
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 5: // while
			cout << left1 << "cond:" << endl;
			this->children_[0]->PrintRecursive(depth+1);
			cout << left1 << "stmt:" << endl;
			this->children_[1]->PrintRecursive(depth+1);
			break;
		case 6: // if
			cout << left1 << "cond:" << endl;
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 7: // else
			cout << left1 << "cond:" << endl;
			this->children_[0]->PrintRecursive(depth+1);
			cout << left1 << "stmt:" << endl;
			this->children_[1]->PrintRecursive(depth+1);
			cout << left1 << "else_stmt:" << endl;
			this->children_[2]->PrintRecursive(depth+1);
			break;
		case 8: // print exp
			this->children_[0]->PrintRecursive(depth+1);
			break;
		case 9: // print literal
			cout << left1 << "string: " <<
			this->printStringLiteral_ << endl;
			break;
		}

	}
};

struct StatementsNode: public ASTNode {
	StatementsNode(StatementNode * statementNode) {
		this->children_.push_back(statementNode);
	}

	StatementsNode(StatementsNode * statementsNode,
		StatementNode * statementNode) {
		for (auto node : statementsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(statementNode);
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string(depth*2, ' ');
		string left2 = std::string((depth+1)*2, ' ');
		cout << left1 << "name: stmts" << endl;
		cout << left1 << "stmts:" << endl;
		cout << left2 << "-" << endl;

		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(depth+2);
			if (i!=this->children_.size()-1)
				cout << left2 << "-" << endl;
		}
	};
};


struct VdeclNode : public ASTNode {
	ValidType * type_;
	string identifier_;
	VdeclNode(ValidType * type, string identifier) :
		type_(type), identifier_(identifier.substr(1, string::npos)) { }

	void
	PrintSelf(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		cout << left1 << "vdecl:" << endl;
		cout << left2 << "node: vdecl" << endl;
		cout << left2 << "type: " << this->type_->GetName() << endl;
		cout << left2 << "var: " << this->identifier_ << endl;
	}
};

struct VdeclsNode : public ASTNode {
	VdeclsNode(VdeclNode * vdeclNode) {
		this->children_.push_back(vdeclNode);
	}

	VdeclsNode(VdeclsNode * vdeclsNode, VdeclNode * vdeclNode) {
		for (auto node : vdeclsNode->children_) {
			this->children_.push_back(node);
		}
		this->children_.push_back(vdeclNode);
	}

	void
	PrintRecursive(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		cout << left1 << "vdecls:" << endl;
		cout << left2 << "name: vdecls" << endl;
		cout << left2 << "vars:" << endl;
		cout << left3 << "-" << endl;
		for (unsigned i=0; i<this->children_.size(); ++i) {
			this->children_[i]->PrintRecursive(depth+3);
			if (i!=this->children_.size()-1)
				cout << left3 << "-" << endl;
		}
	};
};

#endif /* EKCC_AST_HPP_ */
