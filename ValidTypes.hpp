#ifndef VALIDTYPES_HPP_
#define VALIDTYPES_HPP_
#include <string.h>
#include <iostream>

using std::cout;
using std::endl;

struct IntType {
};

struct FloatType {
};

struct CintType {
};

struct BoolType {
};

struct VoidType {
};

struct RefType {
    bool noAlias_;
    void
    Init(char * line) {
        if (strstr(line, "alias")!=NULL) {
            cout << "no alias defined" << endl;
            this->noAlias_ = true;
        } else {
            this->noAlias_ = false;
        }
    }
};

struct IfControl {
};

struct ElseControl {
};

struct WhileControl {
};

struct ReturnControl {
};

struct ForControl {
};

struct PrintFunction {

};

struct RunFunction {

};

struct DefFunction {

};

struct ExternFunction {

};
#endif