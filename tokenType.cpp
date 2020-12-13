#include "compiler.h"

tokenType::tokenType(string name, enum tokenCategory type) {
    this->name = name;
    this->type = type;
}

enum tokenCategory tokenType::getTokenCategory() {
    return type;
}

string tokenType::getName() {
    return name;
}

bool tokenType::isEqualType(tokenCategory type2) {
    if (type2 == type)
        return true;
    return false;
}

void tokenType::iniTokenType(string fName, enum tokenCategory fType) {
    this->name = std::move(fName);
    this->type = fType;
}

void tokenType::changeName(string newName) {
    this->name = std::move(newName);
}

tokenType::tokenType() = default;