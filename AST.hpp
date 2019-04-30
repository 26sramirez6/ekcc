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
	unsigned depth_ = 0;

	virtual void
	PrintRecursive(unsigned depth) {
		PrintSelf(depth);
		for (auto node: this->children_) {
			node->PrintRecursive(depth+1);
		}
	};

	virtual void
	PrintSelf(unsigned depth) {
		string left = std::string(depth*2, ' ');
		cout << left << "name: " << GetName() << endl;
	};

	virtual string
	GetName() {
		return "";
	}

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

	string
	GetName() {
		return string("prog");
	}

	virtual void
	PrintSelf(unsigned depth) {
		string left = std::string(depth*2, ' ');
		cout << left << "---" << endl;
		cout << left << "name: " << GetName() << endl;
	};
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

	virtual void
	PrintRecursive(unsigned depth) {
		PrintSelf(depth);
		for (auto node: this->children_) {
			node->PrintRecursive(depth+2);
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

	string
	GetName() {
		return string("func");
	}

	void
	PrintSelf(unsigned depth) {
		string left = std::string(depth*2, ' ');
		cout << left << "name: " << this->GetName() << endl;
		cout << left << "retType: " << this->retType_->GetName() << endl;
		cout << left << "globid: " << this->identifier_ << endl;
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

	string
	GetName() {
		return string("funcs");
	}

	void
	PrintSelf(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		cout << left1 << "funcs:" << endl;
		cout << left2 << "name: funcs" << endl;
		cout << left2 << "funcs:" << endl;
		cout << left3 << "-" << endl;
	}

	virtual void
	PrintRecursive(unsigned depth) {
		PrintSelf(depth);
		for (auto node: this->children_) {
			node->PrintRecursive(depth+2);
		}
	};

};

struct BlockNode: public ASTNode {
	string
	GetName() {
		return string("blk");
	}

	void
	PrintSelf(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		cout << left1 << "blk:" << endl;
		cout << left2 << "name: blk" << endl;
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

	void
	PrintSelf(unsigned depth) {
		string left1 = std::string((depth-1)*2, ' ');
		string left2 = std::string(depth*2, ' ');
		string left3 = std::string((depth+1)*2, ' ');
		cout << left1 << "vdecls:" << endl;
		cout << left2 << "name: vdecls" << endl;
		cout << left2 << "vars:" << endl;
		cout << left3 << "-" << endl;
	}

	virtual void
	PrintRecursive(unsigned depth) {
		PrintSelf(depth);
		for (auto node: this->children_) {
			node->PrintRecursive(depth+2);
		}
	};
};

#endif /* EKCC_AST_HPP_ */
