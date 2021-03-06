#include "compiler.h"

void functionTable::addToken(function &fn, int l) {
    string name = fn.getName();
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (tokens.count(name) == 0 && !globalTable.contains(name)) {
        tokens.insert(pair<string, function>(name, fn));
    } else
        error_syntax(to_string(l) + " b");
}

function functionTable::getToken(const string &name) {
    string name1 = name;
    transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    return tokens.at(name1);
}

bool functionTable::contains(const string &name) {
    string name1 = name;
    transform(name1.begin(), name1.end(), name1.begin(), ::tolower);
    if (tokens.count(name1) == 0)
        return false;
    else
        return true;
}

void functionTable::changeSameNames() {
    for (auto &fn:tokens) {
        fn.second.replaceSameName();
    }
}

void functionTable::addGlobalVarUsed(const string &fnName, const string &varName) {
    string fName = fnName;
    string vName= varName;
    transform(fName.begin(), fName.end(), fName.begin(), ::tolower);
    transform(vName.begin(), vName.end(), vName.begin(), ::tolower);
    tokens.at(fName).addGlobalVarUsed(vName);
}
