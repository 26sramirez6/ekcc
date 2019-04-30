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
#include "ValidTypes.hpp"
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::move;


struct ASTNode {
	vector<ASTNode *> children_;
	static string astName_;
	virtual void PrintRecursive() {
		PrintSelf();
		for (auto node: this->children_) {
			node->PrintRecursive();
		}
	};
	virtual void PrintSelf() {};
	virtual ~ASTNode() {};

};


struct ProgramNode : public ASTNode {
	ProgramNode(ASTNode * funcNodes) {
		this->children_.push_back(funcNodes);
	}

	ProgramNode(ASTNode * externs, ASTNode * funcNodes) {
		this->children_.push_back(externs);
		this->children_.push_back(funcNodes);
	}

	void
	PrintSelf() {
		cout << "name: prog" << endl;
	}
};


struct ExternNode : public ASTNode {
	void
	PrintSelf() {
		cout << "name: extern" << endl;
	}
};

struct ExternsNode : public ASTNode {

};

struct FuncNode : public ASTNode {

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


};

struct VariableNode : public ASTNode {
	ValidType type_;
	string identifier_;
	VariableNode(ValidType type, string identifier) : type_(type), identifier_(identifier){
		this->astName_ = "vardeclstmt";
	}
};

#endif /* EKCC_AST_HPP_ */
