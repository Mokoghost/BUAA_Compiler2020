#include "compiler.h"

void Identifier::setIdentifierCategory(enum identifierCategory kind) {
    this->kind = kind;
}

enum identifierCategory Identifier::getIdCategory() {
    return kind;
}

Identifier::Identifier(string name, enum tokenCategory type) : tokenType(name, type) {

}

void Identifier::setDimension(int d, int num) {
    if (d == 1)
        dimension1 = num;
    else
        dimension2 = num;
}

//int Identifier::getDimension(int d) {
//    if (d == 1)
//        return dimension1;
//    else
//        return dimension2;
//}

Identifier::Identifier() {

}
