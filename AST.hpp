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
#include "ValidTypes.hpp"
using std::cout;
using std::endl;
using std::vector;
using std::string;

struct ASTNode {
	vector<ASTNode> children;
	string astName;
	virtual void Print();
	virtual ~ASTNode();
};

struct ProgramNode : public ASTNode {

	void
	Build(ASTNode externs, ASTNode funcs) {
		this->children.push_back(externs);
		this->children.push_back(funcs);
	}

	void
	Build(ASTNode funcs) {
		this->children.push_back(funcs);
	}

	void
	Print() {
		cout << "name: prog" << endl;
		for (auto node: this->children) {
			node.Print();
		}
	}
};

struct ExternNode : public ASTNode {

};

struct FuncNode : public ASTNode {

};

struct FuncsNode : public ASTNode {

};

struct VariableNode : public ASTNode {
	string identifier_;
	ValidType type_;
	static string astName = std::string("vardeclstmt");
	VariableNode(ValidType type, string identifier) : type_(type), identifier_(identifier){

	}
};


#endif /* EKCC_AST_HPP_ */
