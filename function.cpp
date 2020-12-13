#include <utility>

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
    this->tempNum = 0;
    this->spUpOff = 0;
    this->spDownOff = 0;
    memset(stackForTemp, false, 1000 * sizeof(bool));
}

void function::addToken(Identifier &token, int l) {
    this->tokenTable.addToken(token, l);
    if (globalTable.contains(token.getName()))
        sameName.insert(pair<string, string>(token.getName(), token.getName() + "_" + this->getName() + "_LOCAL"));
}

bool function::contains(const string &name) {
    return tokenTable.contains(name);
}

Identifier function::getToken(const string &name) {
    return tokenTable.getToken(name);
}

function::function() {
    this->returnNum = 0;
    this->argNum = 0;
    this->tempNum = 0;
    this->spUpOff = 0;
    this->spDownOff = 0;
    memset(stackForTemp, false, 1000 * sizeof(bool));
}

void function::setRegToVar(const string &varName, Reg reg) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    this->reg2var.insert(pair<Reg, string>(reg, name));
    this->var2reg.insert(pair<string, Reg>(name, reg));
}

Reg function::getRegOfVar(const string &varName, bool use) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    Reg reg = this->var2reg.at(name);
    if (use) {
        if (this->contains(name) || globalTable.contains(name))
            return reg;
        if (tempVarCounter[name] - 2 == 0) {
            usedRegister[reg] = false;
            this->var2reg.erase(name);
            this->reg2var.erase(reg);
            return reg;
        } else {
            tempVarCounter[name]--;
            return reg;
        }
    } else {
        return this->var2reg.at(name);
    }
}

string function::getVarInReg(Reg reg) {
    return this->reg2var.at(reg);
}

void function::deleteVarInReg(const string &varName, Reg reg) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    this->var2reg.erase(name);
    this->reg2var.erase(reg);
}

bool function::varHasReg(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    return this->var2reg.count(name) != 0;
}

void function::setStackToVar(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    int size = getToken(name).getSize();
    mipsCode.emplace_back("addi $sp $sp -" + to_string(size));
    this->var2off.insert(pair<string, int>(name, spUpOff - 4));
    this->spUpOff -= size;
}

bool function::stackHasVar(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    return this->var2off.count(name) != 0;
}

int function::getOffSet(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    int thisOff = var2off.at(name);
    return thisOff - spUpOff;
}

void function::generateSaveBeforeCall() {
    vector<string> argsStatement;
    argsStatement.emplace_back("sw $ra ($sp)");
    int i = 4;
    for (auto &var:var2reg) {
        if (globalTable.contains(var.first)) {
            int memAddress = var2mem.at(var.first);
            stringstream stream;
            stream << "0x" << hex << memAddress;
            argsStatement.emplace_back("sw " + reg2str.at(var.second) + " " + stream.str());
        } else {
            argsStatement.emplace_back("sw " + reg2str.at(var.second) + " " + to_string(i) + "($sp)");
            i += 4;
        }
    }
    i += spDownOff;
    mipsCode.emplace_back("addi $sp $sp -" + to_string(i));
    for (auto &off:var2off) {
        if (off.second > 0) {
            off.second = spUpOff - off.second;
        }
    }
    spUpOff -= i;
    for (auto &arg:argsStatement) {
        mipsCode.push_back(arg);
    }
    var2offSaved = var2off;
    var2regSaved = var2reg;
    reg2varSaved = reg2var;
}

void function::generateLoadAfterCall() {
    mipsCode.emplace_back("lw $ra ($sp)");
    int i = 4;
    var2off = var2offSaved;
    var2reg = var2regSaved;
    reg2var = reg2varSaved;
    for (auto &var:var2reg) {
        if (globalTable.contains(var.first)) {
            int memAddress = var2mem.at(var.first);
            stringstream stream;
            stream << "0x" << hex << memAddress;
            mipsCode.emplace_back("lw " + reg2str.at(var.second) + " " + stream.str());
        } else {
            mipsCode.emplace_back("lw " + reg2str.at(var.second) + " " + to_string(i) + "($sp)");
            i += 4;
        }
    }
    i += spDownOff;
    spUpOff += i;
    for (auto &reg:var2off) {
        if (var2DownOff.count(reg.first) != 0) {
            reg.second = var2DownOff.at(reg.first);
        }
    }
    mipsCode.emplace_back("addi $sp $sp " + to_string(i));
}

map<string, Identifier> function::getTokens() {
    return this->tokenTable.getTokens();
}

void function::functionEnding() {
    for (auto &reg:reg2var) {
        if (globalTable.contains(reg.second)) {
            int memAddress = var2mem.at(reg.second);
            stringstream stream;
            stream << "0x" << hex << memAddress;
            mipsCode.emplace_back("sw " + reg2str.at(reg.first) + " " + stream.str());
        }
    }
    regPointer = $t0;
    memset(usedRegister, false, 32 * sizeof(bool));
    if (spUpOff != 0)
        mipsCode.emplace_back("addi $sp $sp " + to_string(abs(spUpOff)));
}

bool function::hasSameName(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    return sameName.count(name) > 0;
}

string function::getReplaceName(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    return sameName.at(name);
}

void function::replaceSameName() {
    for (auto &name:sameName) {
        tokenTable.changeTokenName(name.first, name.second);
    }
    for (auto &arg:args) {
        if (sameName.count(arg.getName()) != 0) {
            arg.changeName(sameName.at(arg.getName()));
        }
    }
}

void function::functionReturn() {
    for (auto &reg:reg2var) {
        if (globalTable.contains(reg.second)) {
            int memAddress = var2mem.at(reg.second);
            stringstream stream;
            stream << "0x" << hex << memAddress;
            mipsCode.emplace_back("sw " + reg2str.at(reg.first) + " " + stream.str());
        }
    }
    if (spUpOff != 0)
        mipsCode.emplace_back("addi $sp $sp " + to_string(abs(spUpOff)));
}

void function::addGlobalVarUsed(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (this->globalVarUsed.count(name) == 0)
        this->globalVarUsed.insert(name);
}

set<string> function::getGlobalVarUsed() {
    return globalVarUsed;
}

void function::eraseRegister() {
    regPointer = $t0;
    memset(usedRegister, false, 32 * sizeof(bool));
}

void function::setStackToTemp(const string &varName) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    int select = 1;
    for (int i = 1; i < 1000; i++) {
        if (!stackForTemp[i]) {
            select = i;
            stackForTemp[i] = true;
            if (select > (spDownOff / 4))
                spDownOff = select * 4;
            break;
        }
    }
    this->var2DownOff.insert(pair<string, int>(name, select * 4));
    this->var2off.insert(pair<string, int>(name, select * 4));
}

bool function::regsHasFull() {
    return reg2var.size() == 15;
}

int function::getOffsetOfTemp(const string &varName, bool use) {
    string name=varName;
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    int off = -var2off.at(name);
    if (use) {
        if (this->contains(name))
            return off;
        if (tempVarCounter[name] - 2 == 0) {
            this->var2off.erase(name);
            this->var2DownOff.erase(name);
            return off;
        } else {
            tempVarCounter[name]--;
            return off;
        }
    } else
        return off;
}
