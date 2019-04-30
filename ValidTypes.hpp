#ifndef VALIDTYPES_HPP_
#define VALIDTYPES_HPP_
#include <string.h>
#include <iostream>

using std::cout;
using std::endl;

struct ValidType {
};

struct IntType : public ValidType {
};

struct FloatType : public ValidType {
};

struct CintType : public ValidType {
};

struct BoolType : public ValidType {
};

struct VoidType : public ValidType {
};

struct RefType : public ValidType {
    bool noAlias_ = false;
    RefType() {}
    RefType(bool noAlias) : noAlias_(noAlias) {}
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
