#include <utility>

#include "compiler.h"

void identityTable::addToken(Identifier &token, int l) {
    string name = token.getName();
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (this->tokens.count(name) == 0)
        this->tokens.insert(pair<string, Identifier>(name, token));
    else
        error_syntax(to_string(l) + " b");
}

Identifier identityTable::getToken(const string &name) {
    string name1 = name;
    transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    return tokens.at(name1);
}

bool identityTable::contains(const string &name) {
    string name1 = name;
    transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    if (tokens.count(name1) == 0)
        return false;
    else
        return true;
}

map<string, Identifier> identityTable::getTokens() {
    return tokens;
}

void identityTable::changeTokenName(const string &origin, const string &replace) {
    tokens.at(origin).changeName(replace);
    string replaceNeo = replace;
    transform(replaceNeo.begin(), replaceNeo.end(), replaceNeo.begin(), ::tolower);
    tokens.insert(pair<string, Identifier>(replaceNeo, tokens.at(origin)));
    tokens.erase(origin);
}

identityTable::identityTable() = default;
