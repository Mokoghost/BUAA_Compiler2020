#include "compiler.h"

vector<string> mipsCode;
map<string, string> strings;
int strNum;
const char registers[32][6] = {"$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1",
                               "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4",
                               "$s5", "$s6", "$s7", "$t8", "$t9", "$gp", "$sp", "$fp", "$ra"};
const int syscallService[6] = {1, 4, 5, 10, 11, 12};
bool usedRegister[32];
int pos;
enum Reg regPointer = $t0;//寄存器循环指针，永远指向下一个要用的寄存器
int memoryPointer;//内存使用量
map<string, enum Reg> var2reg;////变量对应的寄存器
map<enum Reg, string> reg2var;//寄存器存储的变量
map<string, enum tokenCategory> tempType;//临时变量类型
map<string, int> var2mem;//变量对应的内存
map<int, string> mem2var;//内存地址存储的变量
void enter() {
    mipsCode.emplace_back("");
}

void generateMIPSAssembly() {
    allocateMemory();
    mipsCode.emplace_back(".text");
    enter();
    mipsCode.emplace_back("j " + functions.getToken("main").getName());
    enter();
    while (pos < intermediateCodeInner.size()) {
        vector<string> inter = intermediateCodeInner.at(pos);
        string fnName = getRealName(inter.at(1), '(');
        if (functions.contains(fnName)) {
            function fn = functions.getToken(fnName);
            generateFunctionMIPS(fn);
        }
    }
}

void allocateMemory() {
    mipsCode.emplace_back(".data");
    for (auto &token : globalTable.getTokens()) {
        int space = token.second.getSize();
        mipsCode.emplace_back(token.second.getName() + ": " + ".space" + " " + to_string(space));
        token.second.setMemory(to_string(DATA_BASE_ADDRESS + memoryPointer));
        memoryPointer += space;
    }
    for (auto &str:strings) {
        int space = int(ceil(double(str.second.size()) / 4)) * 4;
        string name = str.first.substr(1, str.first.length() - 1);
        mipsCode.emplace_back(name + ": .asciiz \"" + str.second + "\"");
        memoryPointer += space;
    }
    enter();
}


void generateFunctionMIPS(function &fn) {
    mipsCode.emplace_back(fn.getName() + ":");
    enter();
    vector<string> inter = intermediateCodeInner.at(++pos);
    string fnName = getRealName(inter.at(1), '(');
    while (!functions.contains(fnName)) {//若包含说明读到下一个函数
        if (inter.at(0) == "scan") {//读语句
            generateScanMIPS(fn, inter);
        } else if (inter.at(0) == "print") {//写语句
            generatePrintMIPS(fn, inter);
        } else {
            generateStatement(fn, inter);
        }
        if (pos + 1 < intermediateCodeInner.size())
            inter = intermediateCodeInner.at(++pos);
        else {
            pos++;
            break;
        }
    }
    globalVariableLoad(fn);
}

void variableArrangeRegisterIm(function &fn, string &varName, enum varType type) {
    var2reg.insert(pair<string, Reg>(varName, Reg(regPointer)));
    reg2var.insert(pair<Reg, string>(Reg(regPointer), varName));
    usedRegister[regPointer] = true;
    switch (type) {
        case LOCAL_VAR: {
            fn.getToken(varName).allocateRegister(Reg(regPointer));
            break;
        }
        case GLOBAL_VAR: {
            globalTable.getToken(varName).allocateRegister(Reg(regPointer));
            break;
        }
        case TEMP_VAR:
        default:
            break;
    }
}

void variableArrangeRegister(function &fn, string &putInVar, enum varType type) {
    if (var2reg.count(putInVar) != 0)
        return;
    if (!usedRegister[regPointer]) {//若寄存器未满
        variableArrangeRegisterIm(fn, putInVar, type);
    } else {
        //两个步骤：
        //1.将寄存器头指针处的变量存入内存，并将该变量从寄存器中删除
        // 1.1 若内存中未存储过该变量，则新开辟一片空间将变量存入并记录在var2mem中
        // 1.2 若内存中存储过该变量，则将内存存入该地址中
        string pickOutVar = reg2var.at(regPointer);
        if (var2mem.count(pickOutVar) != 0) {
            int memAddress = var2mem.at(pickOutVar);
            stringstream stream;
            stream << "0x" << hex << memAddress;
            mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
        } else {
            int memAddress = DATA_BASE_ADDRESS + memoryPointer;
            stringstream stream;
            stream << "0x" << hex << memAddress;
            mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
            auto ty = whatIsThisShit(pickOutVar, fn);
            switch (ty) {
                case LOCAL_VAR: {
                    fn.getToken(pickOutVar).setMemory(to_string(DATA_BASE_ADDRESS + memoryPointer));
                    memoryPointer += fn.getToken(pickOutVar).getSize();
                    fn.getToken(pickOutVar).removeRegister();
                    break;
                }
                case GLOBAL_VAR: {
                    globalTable.getToken(pickOutVar).setMemory(to_string(DATA_BASE_ADDRESS + memoryPointer));
                    memoryPointer += globalTable.getToken(pickOutVar).getSize();
                    globalTable.getToken(pickOutVar).removeRegister();
                    break;
                }
                case TEMP_VAR:
                default:
                    break;
            }
            var2mem.insert(pair<string, int>(pickOutVar, memAddress));
            mem2var.insert(pair<int, string>(memAddress, pickOutVar));
        }
        mipsCode.emplace_back("add $t9 $0 " + string(registers[int(regPointer)]));
        mipsCode.emplace_back("sw $t9 ($t8)");
        reg2var.erase(regPointer);
        var2reg.erase(pickOutVar);
        //2.将寄存器分配给变量
        // 2.1 若内存中没有此变量，则直接分配寄存器
        // 2.2 若内存中存有此变量，则将内存中的值取出并存于寄存器中
        if (var2mem.count(putInVar) == 0)
            variableArrangeRegisterIm(fn, putInVar, type);
        else {
            int memAddress = var2mem.at(putInVar);
            stringstream stream;
            stream << "0x" << hex << memAddress;
            mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
            variableArrangeRegisterIm(fn, putInVar, type);
            mipsCode.emplace_back("lw " + string(registers[var2reg.at(putInVar)]) + " ($t8)");
        }
    }
    if (regPointer < $s7) {
        int next = int(regPointer);
        next++;
        regPointer = Reg(next);
    } else
        regPointer = $t0;
}

void generateScanMIPS(function &fn, vector<string> &inter) {
    string varName = inter.at(1);
    auto type = whatIsThisShit(varName, fn);
    if (type == LOCAL_VAR) {
        Identifier var = fn.getToken(varName);
        if (var.isEqualType(INTTK)) {
            int service = syscallService[READ_INT];
            mipsCode.emplace_back("li $v0 " + to_string(service));
        } else {
            int service = syscallService[READ_CHARACTER];
            mipsCode.emplace_back("li $v0 " + to_string(service));
        }
    } else {
        Identifier var = globalTable.getToken(varName);
        if (var.isEqualType(INTTK)) {
            int service = syscallService[READ_INT];
            mipsCode.emplace_back("li $v0 " + to_string(service));
        } else {
            int service = syscallService[READ_CHARACTER];
            mipsCode.emplace_back("li $v0 " + to_string(service));
        }
    }
    mipsCode.emplace_back("syscall");
    if (var2reg.count(varName) == 0 || var2mem.count(varName) == 0) {
        variableArrangeRegister(fn, varName, static_cast<varType>(type));
        string vt = registers[var2reg.at(varName)];
        mipsCode.emplace_back("add " + vt + " $0 $v0");
    } else if (var2reg.count(varName) != 0) {
        string vt = registers[var2reg.at(varName)];
        mipsCode.emplace_back("add " + vt + " $0 $v0");
    } else {
        stringstream stream;
        stream << "0x" << hex << var2mem.at(varName);
        mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
        mipsCode.emplace_back("add $t9 $0 $v0");
        mipsCode.emplace_back("sw $t9 ($t8)");
    }
    makeChanged(varName, fn);
}

void generatePrintMIPS(function &fn, vector<string> &inter) {
    string varName0 = inter.at(1);
    string varName1 = getRealName(varName0, '[');
    switch (whatIsThisShit(varName1, fn)) {
        case TEMP_VAR: {
            int service = syscallService[PRINT_INT];
            mipsCode.emplace_back("li $v0 " + to_string(service));
            if (var2reg.count(varName1) != 0) {
                mipsCode.emplace_back("addu $a0 $0 " + string(registers[var2reg.at(varName1)]));
            } else {
                stringstream stream;
                stream << "0x" << hex << var2mem.at(varName1);
                mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
                mipsCode.emplace_back("lw $a0, ($t8)");
            }
            mipsCode.emplace_back("syscall");
            break;
        }
        case STR_VAR: {
            int service = syscallService[PRINT_STR];
            mipsCode.emplace_back("li $v0 " + to_string(service));
            mipsCode.emplace_back("la $a0 " + varName1.substr(1, varName1.length() - 1));
            mipsCode.emplace_back("syscall");
            if (inter.size() > 2) {
                string varName2 = inter.at(2);
                string varName3 = getRealName(varName2, '[');
                int type = whatIsThisShit(varName3, fn);
                if (type == TEMP_VAR)
                    service = syscallService[PRINT_INT];
                else
                    service = syscallService[PRINT_CHAR];
                mipsCode.emplace_back("li $v0 " + to_string(service));
                mipsCode.emplace_back("addu $a0 $0 " + getValueOfVar(varName2, fn));
                mipsCode.emplace_back("syscall");
            }
            break;
        }
        case LOCAL_VAR:
        case GLOBAL_VAR: {
            int service = syscallService[PRINT_CHAR];
            mipsCode.emplace_back("li $v0 " + to_string(service));
            mipsCode.emplace_back("addu $a0 $0 " + getValueOfVar(varName0, fn));
            mipsCode.emplace_back("syscall");
            break;
        }
        default:
            break;
    }
    generatePrintEnter();
}

void globalVariableLoad(function &fn) {

}

void generateVariableStatementInner(function &fn, identityTable &table, string &vt2, string &vt3, string &varName1,
                                    enum varType type) {
    string vn;
    tokenCategory ty = table.getToken(varName1).getTokenCategory();
    if (ty == INTTK)
        vn = to_string(table.getToken(varName1).getIntValue(0, 0));
    else {
        stringstream stream;
        stream << table.getToken(varName1).getCharValue(0, 0);
        vn = "\'" + stream.str() + "\'";
    }
    if (vt2.empty()) {
        vt2 = vn;
        mipsCode.emplace_back("li $t8 " + vn);
        vt2 = string(registers[int($t8)]);
    } else
        vt3 = vn;
}

void generateStatement(function &fn, vector<string> &inter) {
    string code;
    string opStr;
    string vt1;
    string vt2;
    string vt3;
    bool hasOp = false;
    bool hasChar = false;
    bool hasInt = false;
    bool hasEqual = false;
//    enum mipsOp op;
    for (auto &varName0:inter) {
        string varName1 = getRealName(varName0, '[');
        auto type = varType(whatIsThisShit(varName1, fn));
        if (type == LOCAL_VAR || type == GLOBAL_VAR || type == TEMP_VAR) {
            if (!vt1.empty()) {//对于右式，若为未改变值的初始化变量或常量
                bool fnContains = fn.contains(varName1);
                bool gtContains = globalTable.contains(varName1);
                if (fnContains || gtContains) {
                    if (fnContains) {
                        if (fn.getToken(varName1).initialized
                            && !fn.getToken(varName1).changed) {
                            identityTable fnTable = fn.getTokenTable();
                            generateVariableStatementInner(fn, fnTable, vt2, vt3, varName1, LOCAL_VAR);
                            continue;
                        }
                    } else {
                        if (globalTable.getToken(varName1).initialized
                            && !globalTable.getToken(varName1).changed) {
                            generateVariableStatementInner(fn, globalTable, vt2, vt3, varName1, GLOBAL_VAR);
                            continue;
                        }
                    }
                }
            }
            variableArrangeRegister(fn, varName1, type);
            if (vt1.empty()) {
                makeChanged(varName1, fn);//变量被赋值
                if (var2reg.count(varName1) != 0) {
                    vt1 = registers[var2reg.at(varName1)];
                } else {
                    stringstream stream;
                    stream << "0x" << hex << var2mem.at(varName1);
                    mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
                    mipsCode.emplace_back("lw $t8 ($t8)");
                    vt1 = registers[int($t8)];
                }
            } else if (vt2.empty()) {
                if (var2reg.count(varName1) != 0) {
                    vt2 = registers[var2reg.at(varName1)];
                } else {
                    stringstream stream;
                    stream << "0x" << hex << var2mem.at(varName1);
                    mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
                    mipsCode.emplace_back("lw $t8 ($t8)");
                    vt2 = registers[int($t8)];
                }
            } else {
                if (var2reg.count(varName1) != 0) {
                    vt3 = registers[var2reg.at(varName1)];
                } else {
                    stringstream stream;
                    stream << "0x" << hex << var2mem.at(varName1);
                    mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
                    mipsCode.emplace_back("lw $t9 ($t9)");
                    vt3 = registers[int($t9)];
                }
            }
        } else if (type == OP) {
            opStr = varName1;
            hasOp = true;
        } else if (type == INTEGER) {
            if (vt2.empty()) {
                vt2 = varName1;
            } else
                vt3 = varName1;
            hasInt = true;
        } else if (type == CHARACTER) {
            string vtn;
            if (hasOp)
                vtn = to_string(varName1.at(1) - 'a' + 97);
            else {
                vtn = varName1;
                hasChar = true;
            }
            if (vt2.empty())
                vt2 = vtn;
            else
                vt3 = vtn;
        } else
            hasEqual = true;
    }
    if (hasEqual) {
        if (!opStr.empty()) {//有操作符
            if (!vt3.empty()) {
                switch (opStr.at(0)) {
                    case '+': {
                        code += "add ";
                        break;
                    }
                    case '-': {
                        code += "sub ";
                        break;
                    }
                    case '/': {
                        code += "div ";
                        break;
                    }
                    case '*': {
                        code += "mul ";
                        break;
                    }
                    default:
                        break;
                }
                if (whatIsThisShit(vt2, fn) == INTEGER) {
                    // 若为数字，应首先将数字赋予寄存器
                    mipsCode.emplace_back("li $t8 " + vt2);
                    vt2 = registers[int($t8)];
                }
                code += (vt1 + " " + vt2 + " " + vt3);
            } else {
                switch (opStr.at(0)) {
                    case '+': {
                        code += ("la " + vt1 + " (" + vt2 + ")");
                        break;
                    }
                    case '-': {
                        code += ("sub " + vt1 + " $0" + vt2);
                        break;
                    }
                    default:
                        break;
                }
            }
        } else if (hasInt || hasChar) {//单个字符赋值|单个数值赋值
            code = "li " + vt1 + " " + vt2;
        } else {//单个标识符赋值
            code = "la " + vt1 + " (" + vt2 + ")";
        }
    }
    mipsCode.emplace_back(code);
}

int whatIsThisShit(string &varName, function &fn) {
    if (varName.at(0) == '#') {//字符串
        return STR_VAR;
    } else if (fn.contains(varName)) {//局部变量
        return LOCAL_VAR;
    } else if (globalTable.contains(varName)) {//全局变量
        return GLOBAL_VAR;
    } else if (varName.at(0) == '$') {//临时变量
        return TEMP_VAR;
    } else if (varName == "=") {
        return EQUAL;
    } else if (varName.at(0) == '\'') {
        return CHARACTER;
    } else if ((varName.size() > 1 && isdigit(varName.at(1))) ||
               (varName.size() == 1 && isdigit(varName.at(0)))) {
        return INTEGER;
    } else {
        return OP;
    }
}

void generatePrintEnter() {
    mipsCode.emplace_back("addiu $v0 $0 11");
    mipsCode.emplace_back("addiu $a0 $0 10");
    mipsCode.emplace_back("syscall");
}

string getValueOfVar(string &varName, function &fn) {
    string varName1 = getRealName(varName, '[');
    bool fnContains = fn.contains(varName1);
    bool gtContains = globalTable.contains(varName1);
    if (fnContains || gtContains) {//若写语句的第二项是标识符，则必为char类型
        if (fnContains) {
            if (fn.getToken(varName1).initialized
                && !fn.getToken(varName1).changed) {
                stringstream stream;
                stream << fn.getToken(varName1).getCharValue(0, 0);
                return "\'" + stream.str() + "\'";
            }
        } else {
            if (globalTable.getToken(varName1).initialized
                && !globalTable.getToken(varName1).changed) {
                stringstream stream;
                stream << globalTable.getToken(varName1).getCharValue(0, 0);
                return "\'" + stream.str() + "\'";
            }
        }
    }
    //必被赋过值
    if (var2reg.count(varName1)) {
        return registers[var2reg.at(varName1)];
    } else {
        stringstream stream;
        stream << "0x" << hex << var2mem.at(varName1);
        mipsCode.emplace_back("addiu $t8 $0 " + stream.str());
        mipsCode.emplace_back("lw $t8 ($t8)");
        return registers[int($t8)];
    }
}

string getRealName(string varName, char end) {
    string ret;
    for (auto &ch:varName) {
        if (ch == end)
            break;
        ret += ch;
    }
    return ret;
}

void makeChanged(string varName, function &fn) {
    switch (whatIsThisShit(varName, fn)) {
        case GLOBAL_VAR: {
            globalTable.varChange(varName);
            break;
        }
        case LOCAL_VAR: {
            fn.varChange(varName);
            break;
        }

    }

}