// class ASTNode {
// 	std::String disc;
// 	std::Vector<ASTNode*> Children;
// 	ASTNode() { }
	
// 	virtual outputYAML(int indentLevel, std::ostream) = 0;}

// class outputJSON

// class ASTNode("")

// class BinOpNode : public ASTNode { 
// 	Children.push_back(left)
// 	Children.push_back(right)
// 	OP = theop;}
// 	virtual outputYAML(int level){}

// BinOpNode(char OP, ASTNode* Left, ASTNode *Right){ }

// virtual void outputYAML(int level, …outfile){ outfile << <<;
// for(auto child:children)
// 	child -> outputYAML(level +1, outfile);
// }
// }

// exp:
// 	node '+' node {$$ = new EXPNode ("Add", $1, $3) }

// node:
// 	| INT {$$ = new ASTNode()}


// JSON v1.0 is subset of YAML