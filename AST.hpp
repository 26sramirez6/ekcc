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
	virtual void
	PrintRecursive() {
		PrintSelf();
		for (auto node: this->children_) {
			node->PrintRecursive();
		}
	};

	virtual void
	PrintSelf() {
		cout << "name: " << GetName() << endl;
	};

	virtual string
	GetName() {
		return "";
	}

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

	string
	GetName() {
		return string("prog");
	}
};

struct ExternNode : public ASTNode {
	string
	GetName() {
		return string("extern");
	}
};

struct ExternsNode : public ASTNode {
	string
	GetName() {
		return string("externs");
	}
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

	string
	GetName() {
		return string("func");
	}
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

	string
	GetName() {
		return string("funcs");
	}

};

struct BlockNode: public ASTNode {
	string
	GetName() {
		return string("blk");
	}
};

struct VdeclNode : public ASTNode {
	ValidType * type_;
	string identifier_;
	VdeclNode(ValidType * type, string identifier) :
		type_(type), identifier_(identifier) { }

	string
	GetName() {
		return string("vdecl");
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

	string
	GetName() {
		return string("vdecls");
	}
};

#endif /* EKCC_AST_HPP_ */
