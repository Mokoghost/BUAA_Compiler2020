#include "compiler.h"


int function::getArgumentNum() const {
    return argNum;
}

void function::setArgument(const Identifier &arg) {
    args.push_back(arg);
    argNum++;
}

Identifier function::getArg(int index) {
    return args.at(index);
}

function::function(string name, enum tokenCategory type) : tokenType(std::move(name), type) {
    this->returnNum = 0;
    this->argNum = 0;
}

void function::addToken(Identifier &token, int l) {
    this->tokenTable.addToken(token, l);
}

bool function::contains(string name) {
    return tokenTable.contains(name);
}

Identifier function::getToken(const string &name) {
    return tokenTable.getToken(name);
}

function::function() {
    this->returnNum = 0;
    this->argNum = 0;
}

