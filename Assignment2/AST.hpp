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
#include<sstream>
#include "ValidTypes.hpp"
using std::vector;
using std::string;
using std::move;
using std::stringstream;

struct ASTNode {
	vector<ASTNode *> children_;

	virtual void
	PrintRecursive(stringstream& ss, unsigned depth) {};

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
	PrintRecursive(stringstream& ss, unsigned depth) {
		ss << "---" << '\n';
		ss << "name: prog" << '\n';
		for (auto node : this->children_) {
			node->PrintRecursive(ss, depth+1);
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

struct BlockNode: public ASTNode {

	BlockNode() {
	}

	BlockNode(ASTNode * statementsNode) {
		this->children_.push_back(statementsNode);
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


struct ExistingVarNode: public ASTNode {
	string identifier_;
	ExistingVarNode(string identifier) :
	identifier_(identifier) {}

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
	ExistingFuncNode(string identifier) :
	identifier_(identifier) {}

	void
	PrintRecursive(stringstream& ss, unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		ss << left1 << "name: funccall" << '\n';
		ss << left1 << "globid: " << this->identifier_ << '\n';
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
			case Int:
				ss << lv->iValue_ << '\n';
				break;
			case Float:
				ss << lv->fValue_ << '\n';
				break;
			case String:
				ss << lv->sValue_.substr(1, string::npos - 1) << '\n';
				break;
			case Boolean:
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
			for (int i=1; i<this->children_.size(); ++i) {
				this->children_[i]->PrintRecursive(ss, depth+1);
			}
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
		// Check: a function may not return a ref type.
		
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


struct VdeclNode : public ASTNode {
	ValidType * type_;
	string identifier_;
	VdeclNode(ValidType * type, string identifier) :
		type_(type), identifier_(identifier.substr(1, string::npos)) { 
			// Check: <vdecl> may not have void type.
			if(type->GetName() == "void"){
				cout << "error: The type for variable decoration cant be void." << endl;
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

#endif /* EKCC_AST_HPP_ */
