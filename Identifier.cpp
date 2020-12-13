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
    this->initialized = false;
    this->isArray = false;
}

void Identifier::setDimension(int d, int num) {
    if (d == 1)
        dimension1 = num;
    else
        dimension2 = num;
}

int Identifier::getDimension(int d) const {
    if (d == 1)
        return dimension1;
    else
        return dimension2;
}

Identifier::Identifier() {
    this->dimension1 = 1;
    this->dimension2 = 1;
    this->initialized = false;
    this->isArray = false;
}

void Identifier::setValue(int value) {
    this->intValue.push_back(value);
    this->initialized = true;
}

void Identifier::setValue(char value) {
    this->charValue.push_back(value);
    this->initialized = true;
}

int Identifier::getIntValue(int index) {
    return intValue.at(index);
}

char Identifier::getCharValue(int index) {
    return charValue.at(index);
}

int Identifier::getSize() const {
    return 4 * this->dimension1 * this->dimension2;
}

void Identifier::setIsArray() {
    this->isArray = true;
}
