#include "compiler.h"

void Identifier::setIdentifierCategory(enum identifierCategory kind) {
    this->kind = kind;
}

enum identifierCategory Identifier::getIdCategory() {
    return kind;
}

Identifier::Identifier(string name, enum tokenCategory type) : tokenType(name, type) {
    this->dimension1 = 1;
    this->dimension2 = 1;
    this->changed = false;
    this->initialized = false;
    this->reg = nullReg;
}

void Identifier::setDimension(int d, int num) {
    if (d == 1)
        dimension1 = num;
    else
        dimension2 = num;
}

int Identifier::getDimension(int d) {
    if (d == 1)
        return dimension1;
    else
        return dimension2;
}

Identifier::Identifier() {
    this->dimension1 = 1;
    this->dimension2 = 1;
    this->changed = false;
    this->initialized = false;
    this->reg = nullReg;
}

void Identifier::setValue(int value) {
    this->intValue.push_back(value);
    this->initialized = true;
}

void Identifier::setValue(char value) {
    this->charValue.push_back(value);
    this->initialized = true;
}

void Identifier::setMemory(string memory) {
    this->memoryAddress = std::move(memory);
}


void Identifier::allocateRegister(enum Reg reg1) {
    this->reg = reg1;
}

int Identifier::getIntValue(int index1, int index2) {
    return intValue.at(index1 * dimension2 + index2);
}

char Identifier::getCharValue(int index1, int index2) {
    return charValue.at(index1 * dimension2 + index2);
}

int Identifier::getSize() const {
    return 4 * this->dimension1 * this->dimension2;
}

void Identifier::removeRegister() {
    this->reg = nullReg;
}

void Identifier::change() {
    this->changed = true;
}
