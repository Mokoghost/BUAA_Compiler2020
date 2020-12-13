#include "compiler.h"

vector<string> mipsCode;
map<string, string> strings;
int strNum;
const map<Reg, string> reg2str = {{$zero, "$zero"},/*NOLINT*/
                                  {$at,   "$at"},
                                  {$v0,   "$v0"},
                                  {$v1,   "$v1"},
                                  {$a0,   "$a0"},
                                  {$a1,   "$a1"},
                                  {$a2,   "$a2"},
                                  {$a3,   "$a3"},
                                  {$t0,   "$t0"},
                                  {$t1,   "$t1"},
                                  {$t2,   "$t2"},
                                  {$t3,   "$t3"},
                                  {$t4,   "$t4"},
                                  {$t5,   "$t5"},
                                  {$t6,   "$t6"},
                                  {$t7,   "$t7"},
                                  {$s0,   "$s0"},
                                  {$s1,   "$s1"},
                                  {$s2,   "$s2"},
                                  {$s3,   "$s3"},
                                  {$s4,   "$s4"},
                                  {$s5,   "$s5"},
                                  {$s6,   "$s6"},
                                  {$s7,   "$s7"},
                                  {$t8,   "$t8"},
                                  {$t9,   "$t9"},
                                  {$gp,   "$gp"},
                                  {$sp,   "$sp"},
                                  {$fp,   "$fp"},
                                  {$ra,   "$ra"}};
const int syscallService[6] = {1,
                               4,
                               5,
                               10,
                               11,
                               12};
const string branch[6] = {"beq", "bne", "blt", "bge", "j", "jal"};/*NOLINT*/
map<string, int> var2mem;//全局变量位置
map<int, string> mem2var;//内存位置存储的变量
bool usedRegister[32];
bool v1, s7;//作为临时存储
int pos;
enum Reg regPointer = $t0;//寄存器循环指针，永远指向下一个要用的寄存器
int memoryPointer = DATA_BASE_ADDRESS;//内存使用量
int sraNum;
int pushNum;

void enter() {
    mipsCode.emplace_back("");
}

void generateMIPSAssembly() {
    allocateMemory();
    mipsCode.emplace_back(".text");
    generateInitializeMemory();
    enter();
    mipsCode.emplace_back("j " + functions.getToken("main").getName());
    enter();
    while (pos < intermediateCodeInner.size()) {
        if (intermediateCode.at(pos).empty()) {
            pos++;
            continue;
        }
        vector<string> inter = intermediateCodeInner.at(pos);
        string fnName = inter.at(0);
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
        var2mem.insert(pair<string, int>(token.second.getName(), memoryPointer));
        mem2var.insert(pair<int, string>(memoryPointer, token.second.getName()));
        memoryPointer += space;
    }
    for (auto &str:strings) {
        int space = int(ceil(double(str.second.size()) / 4)) * 4;//字符串声明要取整
        string name = str.first.substr(1, str.first.length() - 1);
        mipsCode.emplace_back(name + ": .asciiz \"" + str.second + "\"");
        memoryPointer += space;
    }
    enter();
}

void generateInitializeMemory() {
    for (auto &token : globalTable.getTokens()) {
        if (token.second.initialized) {
            int memAddress = var2mem.at(token.second.getName());
            for (int i = 0; i < token.second.getSize() / 4; i++) {
                stringstream stream;
                stream << "0x" << hex << memAddress + i * 4;
                string output = "la $t8 ";
                if (token.second.getTokenCategory() == CHARTK)
                    mipsCode.emplace_back(output + "\'" + token.second.getCharValue(i) + "\'");
                else
                    mipsCode.emplace_back(output + to_string(token.second.getIntValue(i)));
                mipsCode.emplace_back("sw $t8 " + stream.str());
            }
        }
    }
}

void generateFunctionMIPS(function &fn) {
    mipsCode.emplace_back(fn.getName() + ":");
    enter();
    allocateParas(fn);
    generateAllocateStack(fn);
    if (pos + 1 >= intermediateCodeInner.size()) {
        pos++;
        return;
    }
    vector<string> inter = intermediateCodeInner.at(++pos);
    string fnName = inter.at(0);
    enum branchType brType;
    bool last = false;
    while (!functions.contains(fnName)) {//若包含说明读到下一个函数
        if (inter.at(0) == "@scan") {//读语句
            generateScanMIPS(fn, inter);
            last = false;
        } else if (inter.at(0) == "@print") {//写语句
            generatePrintMIPS(fn, inter);
            last = false;
        } else if (inter.at(0).find("@Label") != string::npos) {//标签/*NOLINT*/
            generateLabel(inter);
            last = false;
        } else if (inter.at(0) == "@call") {//函数调用语句
            generateFunctionCall(inter, fn);
            last = false;
        } else if (inter.at(0) == "@para") {//入口参数
            pos += (fn.getArgumentNum() - 1);
            last = false;
        } else if (inter.at(0) == "@push") {//入口参数
            generatePushStatement(inter, fn);
            last = false;
        } else if (inter.at(0) == "@call_start") {//函数调用语句入口，提示要将变量压栈
            fn.generateSaveBeforeCall();
            pushNum = 0;
            last = false;
        } else if (brType = getBranchType(inter.at(0)), brType != NOT_A_FUCKING_BRANCH) {//跳转指令
            generateBranchIns(brType, inter, fn);
            last = false;
        } else if (inter.at(0) == "return") {//返回指令
            generateReturnStatement(inter, fn);
            last = true;
        } else if (inter.at(0).empty()) {//空行
        } else {//运算赋值相关语句
            generateCalculateStatement(fn, inter);
            last = false;
        }
        if (pos + 1 < intermediateCodeInner.size()) {//若未到中间代码结尾
            inter = intermediateCodeInner.at(++pos);
            fnName = inter.at(0);
        } else {
            pos += 2;
            break;
        }
    }
    if (!last)
        fn.functionEnding();
    else
        fn.eraseRegister();/*NOLINT*/
}

void generateAllocateStack(function &fn) {
    for (auto &token:fn.getTokens()) {
        if (token.second.initialized || token.second.isArray)
            fn.setStackToVar(token.second.getName());
        if (token.second.initialized) {
            for (int i = 0; i < token.second.getSize() / 4; i++) {
                string output = "la $t8 ";
                int base = fn.getOffSet(token.second.getName());
                if (token.second.getTokenCategory() == CHARTK)
                    mipsCode.emplace_back(output + "\'" + token.second.getCharValue(i) + "\'");
                else
                    mipsCode.emplace_back(output + to_string(token.second.getIntValue(i)));
                mipsCode.emplace_back("sw $t8 " + to_string(base - i * 4) + "($sp)");
            }
        }
    }
    for (auto &varName:fn.getGlobalVarUsed()) {
        string name = varName;
        variableArrangeSpace(fn, name, GLOBAL_VAR);
    }
}

void variableArrangeRegisterIm(function &fn, string &varName, enum varType type) {
    fn.setRegToVar(varName, Reg(regPointer));
    usedRegister[regPointer] = true;
    if (type == GLOBAL_VAR) {
        int memAddress = var2mem.at(varName);
        stringstream stream;
        stream << "0x" << hex << memAddress;
        mipsCode.emplace_back("lw " + reg2str.at(regPointer) + " " + stream.str());
    }
}

void variableArrangeSpace(function &fn, string &putInVar, enum varType type) {//数组变量不分配寄存器
    bool blank = false;
    for (int i = int($t0); i <= int($s6); i++) {
        if (!usedRegister[i]) {
            regPointer = Reg(i);
            blank = true;
            break;
        }
    }
    if (type != LOCAL_VAR && type != GLOBAL_VAR && type != TEMP_VAR)
        return;
    if (fn.varHasReg(putInVar))
        return;
    if (blank) {
        /**若寄存器未满，直接将寄存器分配给变量b，若为全局变量，需要将其值从内存中读出存入分配的寄存器。**/
        variableArrangeRegisterIm(fn, putInVar, type);
    } else { /**
              *     不能在函数中改变$sp值，因此如果给局部/临时寄存器分配栈空间只能将其分配于$sp之下
              *当函数跳转时$sp值再一次性减去它们
              * **/
        if (type == GLOBAL_VAR)
            return;
        fn.setStackToTemp(putInVar);
    }
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
    if (!fn.varHasReg(varName) && !fn.stackHasVar(varName)) {//寄存器或栈里都没有此变量
        variableArrangeSpace(fn, varName, static_cast<varType>(type));
        if (fn.varHasReg(varName)) {
            const string &vt = reg2str.at(fn.getRegOfVar(varName, false));
            mipsCode.emplace_back("addu " + vt + " $0 $v0");
        } else {
            string vt = to_string(fn.getOffsetOfTemp(varName, false)) + "($sp)";
            mipsCode.emplace_back("sw $v0 " + vt);
        }
    } else if (fn.varHasReg(varName)) {//寄存器里有此变量
        const string &vt = reg2str.at(fn.getRegOfVar(varName, false));
        mipsCode.emplace_back("addu " + vt + " $0 $v0");
    } else {//只有栈里有此变量
        int offset = fn.getOffSet(varName);
        mipsCode.emplace_back("sw $v0 " + to_string(offset) + "($sp)");
    }
}

void generatePrintMIPSIml(function &fn, string &varName) {
    string varName1 = getRealName(varName, '[');
    varType type = whatIsThisShit(varName1, fn);
    int service;
    switch (type) {
        case TEMP_VAR: {
            if (charTypeTempVar.count(varName) != 0)
                service = syscallService[PRINT_CHAR];
            else
                service = syscallService[PRINT_INT];
            mipsCode.emplace_back("li $v0 " + to_string(service));
            if (fn.varHasReg(varName1)) {//寄存器里有值
                mipsCode.emplace_back("addu $a0 $0 " + reg2str.at(fn.getRegOfVar(varName1, true)));
            } else {//栈里有值
                int offset = fn.getOffsetOfTemp(varName1, true);
                mipsCode.emplace_back("lw $a0 " + to_string(offset) + "($sp)");
            }
            mipsCode.emplace_back("syscall");
            break;
        }
        case STR_VAR: {
            service = syscallService[PRINT_STR];
            mipsCode.emplace_back("li $v0 " + to_string(service));
            mipsCode.emplace_back("la $a0 " + varName1.substr(1, varName1.length() - 1));
            mipsCode.emplace_back("syscall");
            break;
        }
        case LOCAL_VAR:
        case GLOBAL_VAR: {
            if (type == LOCAL_VAR) {
                if (fn.getToken(varName1).getTokenCategory() == INTTK)
                    service = syscallService[PRINT_INT];
                else
                    service = service = syscallService[PRINT_CHAR];
            } else {
                if (globalTable.getToken(varName1).getTokenCategory() == INTTK)
                    service = syscallService[PRINT_INT];
                else
                    service = service = syscallService[PRINT_CHAR];
            }
            mipsCode.emplace_back("li $v0 " + to_string(service));
            mipsCode.emplace_back("addu $a0 $0 " + getValueOfVar(varName, fn));
            mipsCode.emplace_back("syscall");
            break;
        }
        case RET_VAL: {
            if (functions.getToken(varName.substr(5, varName.size() - 5)).getTokenCategory() == INTTK)
                service = syscallService[PRINT_INT];
            else
                service = service = syscallService[PRINT_CHAR];
            mipsCode.emplace_back("addu $a0 $0 $v0");
            mipsCode.emplace_back("li $v0 " + to_string(service));
            mipsCode.emplace_back("syscall");
            break;
        }
        case INTEGER: {
            service = syscallService[PRINT_INT];
            mipsCode.emplace_back("addu $a0 $0 " + varName);
            mipsCode.emplace_back("li $v0 " + to_string(service));
            mipsCode.emplace_back("syscall");
            break;
        }
        case CHARACTER: {
            service = syscallService[PRINT_CHAR];
            mipsCode.emplace_back("addu $a0 $0 " + varName);
            mipsCode.emplace_back("li $v0 " + to_string(service));
            mipsCode.emplace_back("syscall");
            break;
        }
        default:
            break;
    }
}

void generatePrintMIPS(function &fn, vector<string> &inter) {
    generatePrintMIPSIml(fn, inter.at(1));
    if (inter.size() > 2) {
        generatePrintMIPSIml(fn, inter.at(2));
    }
    generatePrintEnter();
}

void setValueOfVarArray(string &varName, string &varName1, function &fn, Identifier var, enum varType type) {/*NOLINT*/
    ////数字地址的下标始终用$t8存储
    stringstream stream;
    vector<string> elements = boomVec(varName);
    if (elements.size() == 1) {//一维数组
        if (isdigit(elements.at(0).at(0))) {//数字下标
            int d1 = stoi(elements.at(0));
            mipsCode.emplace_back("li $t8 " + to_string(d1));
            mipsCode.emplace_back("sll $t8 $t8 2");
            if (type == LOCAL_VAR) {//局部数组变量存于栈中
                int base = fn.getOffSet(varName1);
                mipsCode.emplace_back("li $t9 " + to_string(base));
                mipsCode.emplace_back("sub $t8 $t9 $t8");
                mipsCode.emplace_back("addu $t8 $t8 $sp");
            } else {//全局数组变量存于堆中
                stringstream stream2;
                int base = var2mem.at(varName1);
                stream2 << "0x" << hex << base;
                mipsCode.emplace_back("li $t9 " + stream2.str());
                mipsCode.emplace_back("addu $t8 $t9 $t8");
            }
        } else {//变量下标
            string index = elements.at(0);
            string realName = getRealName(index, '[');
            if (whatIsThisShit(realName, fn) == TEMP_VAR) {
                if (fn.varHasReg(index))
                    mipsCode.emplace_back("sll $t8 " +
                                          reg2str.at(fn.getRegOfVar(index, true)) + " 2");
                else {
                    mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index, true)) + "($sp)");
                    mipsCode.emplace_back("sll $t8 $t8 2");
                }
            } else {
                string vn = getValueOfVar(index, fn);
                if (whatIsThisShit(vn, fn) == TEMP_VAR)//$t9
                    mipsCode.emplace_back("sll $t8 " + vn + " 2");
                else
                    mipsCode.emplace_back("li $t8 " + to_string(stoi(vn) * 4));
            }
            if (type == LOCAL_VAR) {//局部数组变量存于栈中
                int base = fn.getOffSet(varName1);
                mipsCode.emplace_back("li $t9 " + to_string(base));
                mipsCode.emplace_back("sub $t8 $t9 $t8");
                mipsCode.emplace_back("addu $t8 $t8 $sp");
            } else {//全局数组变量存于堆中
                stringstream stream2;
                int base = var2mem.at(varName1);
                stream2 << "0x" << hex << base;
                mipsCode.emplace_back("li $t9 " + stream2.str());
                mipsCode.emplace_back("addu $t8 $t9 $t8");
            }
        }
    } else {//二维数组
        if (isdigit(elements.at(0).at(0))
            && isdigit(elements.at(1).at(0))) {//两个都是数字下标
            int d1 = stoi(elements.at(0));
            int d2 = stoi(elements.at(1));
            int index = d1 * var.getDimension(2) + d2;
            mipsCode.emplace_back("li $t8 " + to_string(index));
            mipsCode.emplace_back("sll $t8 $t8 2");
            if (type == LOCAL_VAR) {//局部数组变量存于栈中
                int base = fn.getOffSet(varName1);
                mipsCode.emplace_back("li $t9 " + to_string(base));
                mipsCode.emplace_back("sub $t8 $t9 $t8");
                mipsCode.emplace_back("addu $t8 $t8 $sp");
            } else {//全局数组变量存于堆中
                stringstream stream2;
                int base = var2mem.at(varName1);
                stream2 << "0x" << hex << base;
                mipsCode.emplace_back("li $t9 " + stream2.str());
                mipsCode.emplace_back("addu $t8 $t9 $t8");
            }
        } else if (isdigit(elements.at(0).at(0))
                   && !isdigit(elements.at(1).at(0))) {//第一维是数字下标，第二维不是
            int d1 = stoi(elements.at(0));
            string index = elements.at(1);
            string realName = getRealName(index, '[');
            if (whatIsThisShit(realName, fn) == TEMP_VAR) {
                if (fn.varHasReg(index))
                    mipsCode.emplace_back("sll $t8 " +
                                          reg2str.at(fn.getRegOfVar(index, true)) + " 2");
                else {
                    mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index, true)) + "($sp)");
                    mipsCode.emplace_back("sll $t8 $t8 2");
                }
            } else {
                string vn = getValueOfVar(index, fn);
                if (whatIsThisShit(vn, fn) == TEMP_VAR)
                    mipsCode.emplace_back("sll $t8 " + vn + " 2");
                else
                    mipsCode.emplace_back("li $t8 " + to_string(stoi(vn) * 4));
            }
            if (type == LOCAL_VAR) {//局部数组变量存于栈中
                int base = fn.getOffSet(varName1) - d1 * var.getDimension(2) * 4;
                mipsCode.emplace_back("li $t9 " + to_string(base));
                mipsCode.emplace_back("sub $t8 $t9 $t8");
                mipsCode.emplace_back("addu $t8 $t8 $sp");
            } else {//全局数组变量存于堆中
                stringstream stream2;
                int base = var2mem.at(varName1) + d1 * var.getDimension(2) * 4;
                stream2 << "0x" << hex << base;
                mipsCode.emplace_back("li $t9 " + stream2.str());
                mipsCode.emplace_back("addu $t8 $t9 $t8");
            }
        } else if (!isdigit(elements.at(0).at(0))
                   && isdigit(elements.at(1).at(0))) {//第一维不是数字下标，第二维是
            int d2 = stoi(elements.at(1)) * 4;
            string index = elements.at(0);
            string realName = getRealName(index, '[');
            if (whatIsThisShit(realName, fn) == TEMP_VAR) {
                if (fn.varHasReg(index))
                    mipsCode.emplace_back("mul $t8 " +
                                          reg2str.at(fn.getRegOfVar(index, true)) + " " +
                                          to_string(var.getDimension(2) * 4));
                else {
                    mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index, true)) + "($sp)");
                    mipsCode.emplace_back("mul $t8 $t8 " + to_string(var.getDimension(2) * 4));
                }
            } else {
                string vn = getValueOfVar(index, fn);
                if (whatIsThisShit(vn, fn) == TEMP_VAR)
                    mipsCode.emplace_back("mul $t8 " + vn + " " +
                                          to_string(var.getDimension(2) * 4));
                else
                    mipsCode.emplace_back(
                            "li $t8 " + to_string(stoi(vn) * var.getDimension(2) * 4));
            }
            mipsCode.emplace_back("addi $t8 $t8 " + to_string(d2));
            if (type == LOCAL_VAR) {//局部数组变量存于栈中
                int base = fn.getOffSet(varName1);
                mipsCode.emplace_back("li $t9 " + to_string(base));
                mipsCode.emplace_back("sub $t8 $t9 $t8");
                mipsCode.emplace_back("addu $t8 $t8 $sp");
            } else {//全局数组变量存于堆中
                stringstream stream2;
                int base = var2mem.at(varName1);
                stream2 << "0x" << hex << base;
                mipsCode.emplace_back("li $t9 " + stream2.str());
                mipsCode.emplace_back("addu $t8 $t9 $t8");
            }
        } else {//第一维和第二维都不是数字下标
            string index1 = elements.at(0);
            string index2 = elements.at(1);
            string realName1 = getRealName(index1, '[');
            string realName2 = getRealName(index2, '[');
            varType ty1 = whatIsThisShit(realName1, fn);
            varType ty2 = whatIsThisShit(realName2, fn);
            if (ty1 == TEMP_VAR) {
                if (fn.varHasReg(index1))
                    mipsCode.emplace_back("mul $t8 " +
                                          reg2str.at(fn.getRegOfVar(index1, true)) + " " +
                                          to_string(var.getDimension(2)));
                else {
                    mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index1, true)) + "($sp)");
                    mipsCode.emplace_back("mul $t8 $t8 " + to_string(var.getDimension(2)));
                }
            } else {
                string vn = getValueOfVar(index1, fn);
                if (whatIsThisShit(vn, fn) == TEMP_VAR)
                    mipsCode.emplace_back("mul $t8 " + vn + " " +
                                          to_string(var.getDimension(2)));
                else
                    mipsCode.emplace_back(
                            "li $t8 " + to_string(stoi(vn) * var.getDimension(2)));
            }
            if (ty2 == TEMP_VAR) {
                if (fn.varHasReg(index1))
                    mipsCode.emplace_back("addu $t8 $t8 " + reg2str.at(fn.getRegOfVar(index2, true)));
                else {
                    mipsCode.emplace_back("lw $t9 " + to_string(fn.getOffsetOfTemp(index2, true)) + "($sp)");
                    mipsCode.emplace_back("addu $t8 $t8 $t9");
                }
            } else {
                string choose;
                if (!v1) {
                    mipsCode.emplace_back("la $v1 ($t8)");
                    choose = "v1";
                    v1 = true;
                } else {
                    mipsCode.emplace_back("la $s7 ($t8)");
                    choose = "s7";
                    s7 = true;
                }
                string vn = getValueOfVar(index2, fn);
                if (choose == "v1") {
                    mipsCode.emplace_back("la $t8 ($v1)");
                    v1 = false;
                } else {
                    mipsCode.emplace_back("la $t8 ($s7)");
                    s7 = false;
                }
                if (whatIsThisShit(vn, fn) == TEMP_VAR)//$t9
                    mipsCode.emplace_back("addu $t8 $t8 " + vn);
                else
                    mipsCode.emplace_back("addi $t8 $t8 " + to_string(stoi(vn)));
            }
            mipsCode.emplace_back("sll $t8 $t8 2");
            if (type == LOCAL_VAR) {//局部数组变量存于栈中
                int base = fn.getOffSet(varName1);
                mipsCode.emplace_back("li $t9 " + to_string(base));
                mipsCode.emplace_back("sub $t8 $t9 $t8");
                mipsCode.emplace_back("addu $t8 $t8 $sp");
            } else {//全局数组变量存于堆中
                stringstream stream2;
                int base = var2mem.at(varName1);
                stream2 << "0x" << hex << base;
                mipsCode.emplace_back("li $t9 " + stream2.str());
                mipsCode.emplace_back("addu $t8 $t9 $t8");
            }
        }
    }
}

void generateCalculateStatement(function &fn, vector<string> &inter) {
    string code;
    string opStr;
    int pointer = 0;
    string vt[3];
    bool hasOp = false;
    bool hasChar = false;
    bool hasInt = false;
    bool hasEqual = false;
    bool isArray = false;
    for (auto &varName0:inter) {
        string varName1 = getRealName(varName0, '[');
        auto type = varType(whatIsThisShit(varName1, fn));
        if (type == LOCAL_VAR || type == GLOBAL_VAR || type == TEMP_VAR) {
            if (pointer >= 1) { //若为右式，可直接采用getValueOfVarIml
                if (type == LOCAL_VAR)
                    vt[pointer++] = getValueOfVarIml(fn, varName0, varName1, fn.getToken(varName1), LOCAL_VAR);
                else if (type == GLOBAL_VAR)
                    vt[pointer++] = getValueOfVarIml(fn, varName0, varName1, globalTable.getToken(varName1),
                                                     GLOBAL_VAR);
                else {
                    if (fn.varHasReg(varName1))
                        vt[pointer++] = reg2str.at(fn.getRegOfVar(varName1, true));
                    else
                        vt[pointer++] = to_string(fn.getOffsetOfTemp(varName1, true)) + "($sp)";
                }
                if (vt[1] == "$t9" && pointer == 2 && inter.size() == 5) {
                    string varName2 = getRealName(inter.at(4), '[');
                    if (fn.stackHasVar(varName2) || var2mem.count(varName2) != 0) {//后面可能会覆盖$t9
                        if (!v1) {
                            mipsCode.emplace_back("la $v1 ($t9)");
                            v1 = true;
                            vt[1] = "$v1";

                        } else {
                            mipsCode.emplace_back("la $s7 ($t9)");
                            s7 = true;
                            vt[1] = "$s7";
                        }
                    }
                }
            } else {
                if (varName1 == varName0 && !fn.stackHasVar(varName1)) {//不是数组或初始化局部变量
                    variableArrangeSpace(fn, varName1, type);
                    if (fn.varHasReg(varName1))
                        vt[pointer++] = reg2str.at(fn.getRegOfVar(varName1, false));
                    else
                        vt[pointer++] = to_string(fn.getOffsetOfTemp(varName1, false)) + "($sp)";
                } else {
                    if (type == LOCAL_VAR) {//若为局部变量
                        if (fn.getToken(varName1).isArray)//数组
                            setValueOfVarArray(varName0, varName1, fn, fn.getToken(varName1), LOCAL_VAR);
                        else {//初始化变量
                            vt[0] = to_string(fn.getOffSet(varName1));
                        }
                    } else {//若为全局变量
                        if (globalTable.getToken(varName1).isArray)
                            setValueOfVarArray(varName0, varName1, fn, globalTable.getToken(varName1), GLOBAL_VAR);
                    }
                    if (inter.size() == 3 && inter.at(2).find('[') != string::npos) {//是数组赋值且等式右端也是数组
                        //$t8存的左式地址，因此会被覆盖
                        if (!v1) {
                            mipsCode.emplace_back("la $v1 ($t8)");
                            v1 = true;
                            vt[0] = "$v1";
                        } else {
                            mipsCode.emplace_back("la $s7 ($t8)");
                            s7 = true;
                            vt[0] = "$s7";
                        }
                    }
                    isArray = true;
                    pointer++;
                }
            }
        } else if (type == RET_VAL) {
            vt[pointer++] = "$v0";
        } else if (type == OP) {
            opStr = varName1;
            hasOp = true;
        } else if (type == INTEGER) {
            vt[pointer++] = varName1;
            hasInt = true;
        } else if (type == CHARACTER) {
            string vtn;
            if (hasOp)
                vtn = to_string(varName1.at(1) - 'a' + 97);
            else {
                vtn = varName1;
                hasChar = true;
            }
            vt[pointer++] = vtn;
        } else
            hasEqual = true;
    }
    if (isArray) {
        if (hasOp) {//必为for循环中的步长
            code = "sw $t9 " + vt[0] + "($sp)";
            if (opStr == "+") {
                mipsCode.emplace_back("addi $t9 $t9 " + vt[2]);
            } else {
                mipsCode.emplace_back("subi $t9 $t9 " + vt[2]);
            }
        } else {//不是临时变量右侧都不会是表达式
            varType ty = whatIsThisShit(vt[1], fn);
            if (ty == INTEGER || ty == CHARACTER) {
                mipsCode.emplace_back("li $t9 " + vt[1]);
                vt[1] = reg2str.at($t9);
            }
            if (!vt[0].empty() && vt[0] != "$v1" && vt[0] != "$s7") {
                if (vt[0].size() > 1 && vt[0].at(1) == 'x')//全局数组变量
                    code = "sw " + vt[1] + " " + vt[0];
                else//局部变量数组
                    code = "sw " + vt[1] + " " + vt[0] + "($sp)";
            } else if (vt[0] == "$v1" || vt[0] == "$s7") {
                if (vt[0] == "$v1") {
                    code = "sw " + vt[1] + " ($v1)";
                    v1 = false;
                } else {
                    code = "sw " + vt[1] + " ($s7)";
                    s7 = false;
                }
            } else {
                code = "sw " + vt[1] + " ($t8)";
            }
        }
    } else {
        if (hasEqual) {
            if (!opStr.empty()) {//有操作符
                if (whatIsThisShit(vt[1], fn) == CHARACTER)
                    vt[1] = to_string(vt[1].at(1));
                if (!vt[2].empty() && whatIsThisShit(vt[2], fn) == CHARACTER)
                    vt[2] = to_string(vt[2].at(1));
                if (pointer == 3) {
                    switch (opStr.at(0)) {
                        case '+': {
                            code += "addu ";
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
                    if (opStr == "<<") {
                        if (whatIsThisShit(vt[1], fn) == INTEGER) {
                            int ele1 = stoi(vt[1]);
                            int ele2 = stoi(vt[2]);
                            string ans = to_string(vt[2].at(0) == '-' ? -(ele1 << abs(ele2)) : ele1 << ele2);/*NOLINT*/
                            if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                                mipsCode.emplace_back("li $t8 " + ans);
                                code = "sw $t8 " + vt[0];
                            } else {
                                code = "li " + vt[0] + " " + ans;
                            }
                        } else {
                            if (stoi(vt[2]) >= 0 && vt[2].at(0) != '-') {
                                vt[2] = to_string(stoi(vt[2]));
                                if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                        mipsCode.emplace_back("sll $t9 $t8 " + vt[2]);
                                        code = "sw $t9 " + vt[0];
                                    } else {
                                        mipsCode.emplace_back("sll $t8 " + vt[1] + " " + vt[2]);
                                        code = "sw $t8 " + vt[0];
                                    }
                                } else {
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                        code = "sll " + vt[0] + " $t8 " + vt[2];
                                    } else
                                        code = "sll " + vt[0] + " " + vt[1] + " " + vt[2];
                                }
                            } else {
                                vt[2] = to_string(-stoi(vt[2]));
                                if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                        mipsCode.emplace_back("sll $t9 $t8 " + vt[2]);
                                        mipsCode.emplace_back("sub $t9 $0 $t9");
                                        code = "sw $t9 " + vt[0];
                                    } else {
                                        mipsCode.emplace_back("sll $t8 " + vt[1] + " " + vt[2]);
                                        mipsCode.emplace_back("sub $t8 $0 $t8");
                                        code = "sw $t8 " + vt[0];
                                    }
                                } else {
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                        mipsCode.emplace_back("sll " + vt[0] + " $t8 " + vt[2]);
                                        code = "sub " + vt[0] + " $0 " + vt[0];
                                    } else {
                                        mipsCode.emplace_back("sll " + vt[0] + " " + vt[1] + " " + vt[2]);
                                        code = "sub " + vt[0] + " $0 " + vt[0];
                                    }
                                }
                            }
                        }
                    } else if (opStr == ">>") {
                        if (whatIsThisShit(vt[1], fn) == INTEGER) {
                            int ele1 = stoi(vt[1]);
                            int ele2 = stoi(vt[2]);
                            int ans = 0;
                            if (ele1 < 0) {
                                if (ele2 > 0) {
                                    ans = (-ele1) >> ele2;/*NOLINT*/
                                    ans = -ans;
                                } else
                                    ans = (-ele1) >> (-ele2);/*NOLINT*/
                            } else {
                                if (ele2 > 0) {
                                    ans = ele1 >> ele2;/*NOLINT*/
                                } else
                                    ans = -(ele1 >> (-ele2));/*NOLINT*/
                            }
                            code = "li " + vt[0] + " " + to_string(ans);
                        } else {
                            if (stoi(vt[2]) >= 0) {
                                if ((vt[1].find("($sp)") != string::npos)/*NOLINT*/
                                    || (vt[0].find("($sp)") != string::npos)) {/*NOLINT*/
                                    string vt0;
                                    string vt1;
                                    if (vt[0] != "$t8") {
                                        vt0 = "$t8";
                                        vt1 = "$t9";
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                    } else {
                                        vt0 = "$t9";
                                        vt1 = "$t8";
                                        mipsCode.emplace_back("lw $t9 " + vt[1]);
                                    }
                                    mipsCode.emplace_back(
                                            "bge " + vt1 + " $0 Label_div_bigger_than_zero" + to_string(++sraNum));
                                    mipsCode.emplace_back("neg " + vt1 + " " + vt1);
                                    mipsCode.emplace_back("sra " + vt0 + " " + vt1 + " " + vt[2]);
                                    mipsCode.emplace_back("neg " + vt0 + " " + vt0);
                                    if (vt[0].find("($sp)") != string::npos)/*NOLINT*/
                                        mipsCode.emplace_back("sw " + vt0 + vt[0]);
                                    else
                                        mipsCode.emplace_back("la " + vt[0] + " (" + vt0 + ")");
                                    mipsCode.emplace_back("j Label_div_end" + to_string(sraNum));
                                    mipsCode.emplace_back("Label_div_bigger_than_zero" + to_string(sraNum) + ":");
                                    mipsCode.emplace_back("sra " + vt0 + " " + vt1 + " " + vt[2]);
                                    if (vt[0].find("($sp)") != string::npos)/*NOLINT*/
                                        mipsCode.emplace_back("sw " + vt0 + vt[0]);
                                    else
                                        mipsCode.emplace_back("la " + vt[0] + " (" + vt0 + ")");
                                    code = "Label_div_end" + to_string(sraNum) + ":";
                                } else {
                                    mipsCode.emplace_back(
                                            "bge " + vt[1] + " $0 Label_div_bigger_than_zero" + to_string(++sraNum));
                                    mipsCode.emplace_back("neg " + vt[1] + " " + vt[1]);
                                    mipsCode.emplace_back("sra " + vt[0] + " " + vt[1] + " " + vt[2]);
                                    mipsCode.emplace_back("neg " + vt[0] + " " + vt[0]);
                                    mipsCode.emplace_back("j Label_div_end" + to_string(sraNum));
                                    mipsCode.emplace_back("Label_div_bigger_than_zero" + to_string(sraNum) + ":");
                                    mipsCode.emplace_back("sra " + vt[0] + " " + vt[1] + " " + vt[2]);
                                    code = "Label_div_end" + to_string(sraNum) + ":";
                                }
                            } else {
                                vt[2] = to_string(-stoi(vt[2]));
                                if ((vt[1].find("($sp)") != string::npos)/*NOLINT*/
                                    || (vt[0].find("($sp)") != string::npos)) {/*NOLINT*/
                                    string vt0;
                                    string vt1;
                                    if (vt[0] != "$t8") {
                                        vt0 = "$t8";
                                        vt1 = "$t9";
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                    } else {
                                        vt0 = "$t9";
                                        vt1 = "$t8";
                                        mipsCode.emplace_back("lw $t9 " + vt[1]);
                                    }
                                    mipsCode.emplace_back(
                                            "bge " + vt1 + " $0 Label_div_bigger_than_zero" + to_string(++sraNum));
                                    mipsCode.emplace_back("neg " + vt1 + " " + vt1);
                                    mipsCode.emplace_back("sra " + vt0 + " " + vt1 + " " + vt[2]);
                                    mipsCode.emplace_back("neg " + vt0 + " " + vt0);
                                    mipsCode.emplace_back("j Label_div_end" + to_string(sraNum));
                                    mipsCode.emplace_back("Label_div_bigger_than_zero" + to_string(sraNum) + ":");
                                    mipsCode.emplace_back("sra " + vt0 + " " + vt1 + " " + vt[2]);
                                    code = "Label_div_end" + to_string(sraNum) + ":";
                                    code = "sub " + vt0 + " $0 " + vt0;
                                    if (vt[0].find("($sp)") != string::npos)/*NOLINT*/
                                        mipsCode.emplace_back("sw " + vt0 + vt[0]);
                                    else
                                        mipsCode.emplace_back("la " + vt[0] + " (" + vt0 + ")");
                                } else {
                                    mipsCode.emplace_back(
                                            "bge " + vt[1] + " $0 Label_div_bigger_than_zero" + to_string(++sraNum));
                                    mipsCode.emplace_back("neg " + vt[1] + " " + vt[1]);
                                    mipsCode.emplace_back("sra " + vt[0] + " " + vt[1] + " " + vt[2]);
                                    mipsCode.emplace_back("neg " + vt[0] + " " + vt[0]);
                                    mipsCode.emplace_back("j Label_div_end" + to_string(sraNum));
                                    mipsCode.emplace_back("Label_div_bigger_than_zero" + to_string(sraNum) + ":");
                                    mipsCode.emplace_back("sra " + vt[0] + " " + vt[1] + " " + vt[2]);
                                    mipsCode.emplace_back("Label_div_end" + to_string(sraNum) + ":");
                                    code = "sub " + vt[0] + " $0 " + vt[0];
                                }
                            }
                        }
                    } else {
                        varType ty1 = whatIsThisShit(vt[1], fn);
                        varType ty2 = whatIsThisShit(vt[2], fn);
                        if (ty1 == INTEGER) {
                            // 若为数字，应首先将数字赋予寄存器
                            mipsCode.emplace_back("li $t8 " + vt[1]);
                            vt[1] = reg2str.at($t8);
                        } else if (ty1 == CHARACTER) {
                            mipsCode.emplace_back("li $t8 " + to_string(vt[1].at(1)));
                            vt[1] = reg2str.at($t8);
                        } else if (ty1 == SP) {
                            mipsCode.emplace_back("lw $t8 " + vt[1]);
                            vt[1] = reg2str.at($t8);
                        }
                        if (ty2 == CHARACTER)
                            vt[2] = to_string(vt[2].at(1));
                        else if (ty2 == SP) {
                            mipsCode.emplace_back("lw $v1 " + vt[2]);
                            vt[2] = "$v1";
                        }
                        if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                            mipsCode.emplace_back(code + "$t8 " + vt[1] + " " + vt[2]);
                            code = "sw $t8 " + vt[0];
                        } else
                            code += (vt[0] + " " + vt[1] + " " + vt[2]);
                    }
                } else {
                    varType ty3 = whatIsThisShit(vt[1], fn);
                    switch (opStr.at(0)) {
                        case '+': {
                            if (ty3 == CHARACTER || ty3 == INTEGER) {
                                if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                                    mipsCode.emplace_back("li $t8 " + vt[1]);
                                    code = "sw $t8 " + vt[0];
                                } else
                                    code = "li " + vt[0] + " " + vt[1];
                            } else {
                                if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                        code = "sw $t8 " + vt[0];
                                    } else
                                        code = "sw " + vt[1] + " " + vt[0];
                                } else {
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        code = "lw " + vt[0] + " " + vt[1];
                                    } else
                                        code = "la " + vt[0] + " (" + vt[1] + ")";
                                }
                            }
                            break;
                        }
                        case '-': {
                            if (ty3 == CHARACTER || ty3 == INTEGER) {
                                if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                                    mipsCode.emplace_back("sub $t8 $0 " + vt[1]);
                                    code = "sw $t8 " + vt[0];
                                } else
                                    code += ("sub " + vt[0] + " $0 " + vt[1]);
                            } else {
                                if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                        mipsCode.emplace_back("sub $t8 $0 $t8");
                                        code = "sw $t8 " + vt[0];
                                    } else {
                                        mipsCode.emplace_back("sub $t8 $0 " + vt[1]);
                                        code = "sw $t8 " + vt[0];
                                    }
                                } else {
                                    if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                                        mipsCode.emplace_back("lw $t8 " + vt[1]);
                                        mipsCode.emplace_back("sub $t8 $0 $t8");
                                        code = "la " + vt[0] + " ($t8)";
                                    } else {
                                        mipsCode.emplace_back("sub $t8 $0 " + vt[1]);
                                        code = "la " + vt[0] + " ($t8)";
                                    }
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            } else if (hasInt || hasChar) {//单个字符赋值|单个数值赋值
                if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                    mipsCode.emplace_back("li $t8 " + vt[1]);
                    code = "sw $t8 " + vt[0];
                } else
                    code = "li " + vt[0] + " " + vt[1];
            } else {//单个标识符赋值
                varType ty3 = whatIsThisShit(vt[1], fn);
                if (ty3 == CHARACTER || ty3 == INTEGER) {
                    if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                        mipsCode.emplace_back("li $t8 " + vt[1]);
                        code = "sw $t8 " + vt[0];
                    } else
                        code = "li " + vt[0] + " " + vt[1];
                } else {
                    if (vt[0].find("($sp)") != string::npos) {/*NOLINT*/
                        if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                            mipsCode.emplace_back("lw $t8 " + vt[1]);
                            code = "sw $t8 " + vt[0];
                        } else
                            code = "sw " + vt[1] + " " + vt[0];
                    } else {
                        if (vt[1].find("($sp)") != string::npos) {/*NOLINT*/
                            code = "lw " + vt[0] + " " + vt[1];
                        } else
                            code = "la " + vt[0] + " (" + vt[1] + ")";
                    }
                }
            }
        }
    }
    v1 = false;
    s7 = false;
    mipsCode.emplace_back(code);
}

enum varType whatIsThisShit(string &varName, function &fn) {
    if (varName.at(0) == '#') {//字符串
        return STR_VAR;
    } else if (fn.contains(varName)) {//局部变量
        return LOCAL_VAR;
    } else if (globalTable.contains(varName)) {//全局变量
        return GLOBAL_VAR;
    } else if (varName.at(0) == '$') {//临时变量
        return TEMP_VAR;
    } else if (varName == "=") {
        return ASS;
    } else if (varName.at(0) == '\'') {
        return CHARACTER;
    } else if ((isdigit(varName.at(0))
                || (varName.size() > 1
                    && (varName.at(0) == '+' || varName.at(0) == '-')
                    && isdigit(varName.at(1)))) && isdigit(varName.at(varName.size() - 1))) {
        return INTEGER;
    } else if (varName.find("@RET") != string::npos) {/*NOLINT*/
        return RET_VAL;
    } else if (varName.find("($sp)") != string::npos) {/*NOLINT*/
        return SP;
    } else {
        return OP;
    }
}

void generatePrintEnter() {
    mipsCode.emplace_back("addiu $v0 $0 11");
    mipsCode.emplace_back("addiu $a0 $0 10");
    mipsCode.emplace_back("syscall");
}

string getValueOfVarIml(function &fn, string &varName, string &varName1, Identifier var, enum varType type) {
    stringstream stream;
    if (varName.find('[') == string::npos) {//普通标识符
        if (var.initialized && var.getIdCategory() == CONST) {//是否是初始化变量
            if (var.getTokenCategory() == CHARTK) {
                stream << var.getCharValue(0);
                return "\'" + stream.str() + "\'";
            } else {
                stream << var.getIntValue(0);
                return stream.str();
            }
        } else {
            if (fn.varHasReg(varName1)) {//寄存器中存在
                return reg2str.at(fn.getRegOfVar(varName1, true));
            } else if (fn.stackHasVar(varName1) || var2mem.count(varName1) != 0) {//堆或栈中存在
                int offset;
                if (fn.stackHasVar(varName1)) {
                    offset = fn.getOffSet(varName1);
                    mipsCode.emplace_back("lw $t9 " + to_string(offset) + "($sp)");
                } else {
                    stringstream stream2;
                    offset = var2mem.at(varName);
                    stream2 << "0x" << hex << offset;
                    mipsCode.emplace_back("lw $t9 " + stream2.str());
                }
                return reg2str.at($t9);
            } else {//分配
                variableArrangeSpace(fn, varName1, whatIsThisShit(varName1, fn));
                if (fn.varHasReg(varName1))
                    return reg2str.at(fn.getRegOfVar(varName1, false));
                else {
                    mipsCode.emplace_back("lw $t9 " + to_string(fn.getOffSet(varName1)) + "($sp)");
                    return reg2str.at($t9);
                }
            }
        }
    } else {
        vector<string> elements = boomVec(varName);
        if (elements.size() == 1) {//一维数组
            if (isdigit(elements.at(0).at(0))) {//数字下标
                int d1 = stoi(elements.at(0));
                mipsCode.emplace_back("li $t8 " + to_string(d1));
                mipsCode.emplace_back("sll $t8 $t8 2");
                if (type == LOCAL_VAR) {//局部数组变量存于栈中
                    int base = fn.getOffSet(varName1);
                    mipsCode.emplace_back("li $t9 " + to_string(base));
                    mipsCode.emplace_back("sub $t8 $t9 $t8");
                    mipsCode.emplace_back("addu $t8 $t8 $sp");
                } else {//全局数组变量存于堆中
                    stringstream stream2;
                    int base = var2mem.at(varName1);
                    stream2 << "0x" << hex << base;
                    mipsCode.emplace_back("li $t9 " + stream2.str());
                    mipsCode.emplace_back("addu $t8 $t9 $t8");
                }
                mipsCode.emplace_back("lw $t9 ($t8)");
                return reg2str.at($t9);
            } else {//变量下标
                string index = elements.at(0);
                string realName = getRealName(index, '[');
                if (whatIsThisShit(realName, fn) == TEMP_VAR) {
                    if (fn.varHasReg(index))
                        mipsCode.emplace_back("sll $t8 " +
                                              reg2str.at(fn.getRegOfVar(index, true)) + " 2");
                    else {
                        mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index, true)) + "($sp)");
                        mipsCode.emplace_back("sll $t8 $t8 2");
                    }
                } else {
                    string vn = getValueOfVar(index, fn);
                    if (whatIsThisShit(vn, fn) == TEMP_VAR)//$t9
                        mipsCode.emplace_back("sll $t8 " + vn + " 2");
                    else//变量转数字
                        mipsCode.emplace_back("li $t8 " + to_string(stoi(vn) * 4));
                }
                if (type == LOCAL_VAR) {//局部数组变量存于栈中
                    int base = fn.getOffSet(varName1);
                    mipsCode.emplace_back("li $t9 " + to_string(base));
                    mipsCode.emplace_back("sub $t8 $t9 $t8");
                    mipsCode.emplace_back("addu $t8 $t8 $sp");
                } else {//全局数组变量存于堆中
                    stringstream stream2;
                    int base = var2mem.at(varName1);
                    stream2 << "0x" << hex << base;
                    mipsCode.emplace_back("li $t9 " + stream2.str());
                    mipsCode.emplace_back("addu $t8 $t9 $t8");
                }
                mipsCode.emplace_back("lw $t9 ($t8)");
                return reg2str.at($t9);
            }
        } else {//二维数组
            if (isdigit(elements.at(0).at(0))
                && isdigit(elements.at(1).at(0))) {//两个都是数字下标
                int d1 = stoi(elements.at(0));
                int d2 = stoi(elements.at(1));
                int index = d1 * var.getDimension(2) + d2;
                mipsCode.emplace_back("li $t8 " + to_string(index));
                mipsCode.emplace_back("sll $t8 $t8 2");
                if (type == LOCAL_VAR) {//局部数组变量存于栈中
                    int base = fn.getOffSet(varName1);
                    mipsCode.emplace_back("li $t9 " + to_string(base));
                    mipsCode.emplace_back("sub $t8 $t9 $t8");
                    mipsCode.emplace_back("addu $t8 $t8 $sp");
                } else {//全局数组变量存于堆中
                    stringstream stream2;
                    int base = var2mem.at(varName1);
                    stream2 << "0x" << hex << base;
                    mipsCode.emplace_back("li $t9 " + stream2.str());
                    mipsCode.emplace_back("addu $t8 $t9 $t8");
                }
                mipsCode.emplace_back("lw $t9 ($t8)");
                return reg2str.at($t9);
            } else if (isdigit(elements.at(0).at(0))
                       && !isdigit(elements.at(1).at(0))) {//第一维是数字下标，第二维不是
                int d1 = stoi(elements.at(0));
                string index = elements.at(1);
                string realName = getRealName(index, '[');
                if (whatIsThisShit(realName, fn) == TEMP_VAR) {
                    if (fn.varHasReg(index))
                        mipsCode.emplace_back("sll $t8 " +
                                              reg2str.at(fn.getRegOfVar(index, true)) + " 2");
                    else {
                        mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index, true)) + "($sp)");
                        mipsCode.emplace_back("sll $t8 $t8 2");
                    }
                } else {
                    string vn = getValueOfVar(index, fn);
                    if (whatIsThisShit(vn, fn) == TEMP_VAR)//$t9
                        mipsCode.emplace_back("sll $t8 " + vn + " 2");
                    else
                        mipsCode.emplace_back("li $t8 " + to_string(stoi(vn) * 4));
                }
                if (type == LOCAL_VAR) {//局部数组变量存于栈中
                    int base = fn.getOffSet(varName1) - d1 * var.getDimension(2) * 4;
                    mipsCode.emplace_back("li $t9 " + to_string(base));
                    mipsCode.emplace_back("sub $t8 $t9 $t8");
                    mipsCode.emplace_back("addu $t8 $t8 $sp");
                } else {//全局数组变量存于堆中
                    stringstream stream2;
                    int base = var2mem.at(varName1) + d1 * var.getDimension(2) * 4;
                    stream2 << "0x" << hex << base;
                    mipsCode.emplace_back("li $t9 " + stream2.str());
                    mipsCode.emplace_back("addu $t8 $t9 $t8");
                }
                mipsCode.emplace_back("lw $t9 ($t8)");
                return reg2str.at($t9);
            } else if (!isdigit(elements.at(0).at(0))
                       && isdigit(elements.at(1).at(0))) {//第一维不是数字下标，第二维是
                int d2 = stoi(elements.at(1)) * 4;
                string index = elements.at(0);
                string realName = getRealName(index, '[');
                if (whatIsThisShit(realName, fn) == TEMP_VAR) {
                    if (fn.varHasReg(index))
                        mipsCode.emplace_back("mul $t8 " +
                                              reg2str.at(fn.getRegOfVar(index, true)) + " " +
                                              to_string(var.getDimension(2) * 4));
                    else {
                        mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index, true)) + "($sp)");
                        mipsCode.emplace_back("mul $t8 $t8 " + to_string(var.getDimension(2) * 4));
                    }
                } else {
                    string vn = getValueOfVar(index, fn);
                    if (whatIsThisShit(vn, fn) == TEMP_VAR)//$t9
                        mipsCode.emplace_back("mul $t8 " + vn + " " +
                                              to_string(var.getDimension(2) * 4));
                    else
                        mipsCode.emplace_back(
                                "li $t8 " + to_string(stoi(vn) * var.getDimension(2) * 4));
                }
                mipsCode.emplace_back("addi $t8 $t8 " + to_string(d2));
                if (type == LOCAL_VAR) {//局部数组变量存于栈中
                    int base = fn.getOffSet(varName1);
                    mipsCode.emplace_back("li $t9 " + to_string(base));
                    mipsCode.emplace_back("sub $t8 $t9 $t8");
                    mipsCode.emplace_back("addu $t8 $t8 $sp");
                } else {//全局数组变量存于堆中
                    stringstream stream2;
                    int base = var2mem.at(varName1);
                    stream2 << "0x" << hex << base;
                    mipsCode.emplace_back("li $t9 " + stream2.str());
                    mipsCode.emplace_back("addu $t8 $t9 $t8");
                }
                mipsCode.emplace_back("lw $t9 ($t8)");
                return reg2str.at($t9);
            } else {//第一维和第二维都不是数字下标
                string index1 = elements.at(0);
                string index2 = elements.at(1);
                string realName1 = getRealName(index1, '[');
                string realName2 = getRealName(index2, '[');
                varType ty1 = whatIsThisShit(realName1, fn);
                varType ty2 = whatIsThisShit(realName2, fn);
                if (ty1 == TEMP_VAR) {
                    if (fn.varHasReg(index1))
                        mipsCode.emplace_back("mul $t8 " +
                                              reg2str.at(fn.getRegOfVar(index1, true)) + " " +
                                              to_string(var.getDimension(2)));
                    else {
                        mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(index1, true)) + "($sp)");
                        mipsCode.emplace_back("mul $t8 $t8 " + to_string(var.getDimension(2)));
                    }
                } else {
                    string vn = getValueOfVar(index1, fn);
                    if (whatIsThisShit(vn, fn) == TEMP_VAR)//$t9
                        mipsCode.emplace_back("mul $t8 " + vn + " " +
                                              to_string(var.getDimension(2)));
                    else
                        mipsCode.emplace_back(
                                "li $t8 " + to_string(stoi(vn) * var.getDimension(2)));
                }
                if (ty2 == TEMP_VAR) {
                    if (fn.varHasReg(index2))
                        mipsCode.emplace_back("addu $t8 $t8 " + reg2str.at(fn.getRegOfVar(index2, true)));
                    else {
                        mipsCode.emplace_back("lw $t9 " + to_string(fn.getOffsetOfTemp(index2, true)) + "($sp)");
                        mipsCode.emplace_back("addu $t8 $t8 $t9");
                    }
                } else {
                    string choose;
                    if (!v1) {
                        mipsCode.emplace_back("la $v1 ($t8)");
                        choose = "v1";
                        v1 = true;
                    } else {
                        mipsCode.emplace_back("la $s7 ($t8)");
                        choose = "s7";
                        s7 = true;
                    }
                    string vn = getValueOfVar(index2, fn);
                    if (choose == "v1") {
                        mipsCode.emplace_back("la $t8 ($v1)");
                        v1 = false;
                    } else {
                        mipsCode.emplace_back("la $t8 ($s7)");
                        s7 = false;
                    }
                    if (whatIsThisShit(vn, fn) == TEMP_VAR)//$t9
                        mipsCode.emplace_back("addu $t8 $t8 " + vn);
                    else
                        mipsCode.emplace_back("addi $t8 $t8 " + to_string(stoi(vn)));
                }
                mipsCode.emplace_back("sll $t8 $t8 2");
                if (type == LOCAL_VAR) {//局部数组变量存于栈中
                    int base = fn.getOffSet(varName1);
                    mipsCode.emplace_back("li $t9 " + to_string(base));
                    mipsCode.emplace_back("sub $t8 $t9 $t8");
                    mipsCode.emplace_back("addu $t8 $t8 $sp");
                } else {//全局数组变量存于堆中
                    stringstream stream2;
                    int base = var2mem.at(varName1);
                    stream2 << "0x" << hex << base;
                    mipsCode.emplace_back("li $t9 " + stream2.str());
                    mipsCode.emplace_back("addu $t8 $t9 $t8");
                }
                mipsCode.emplace_back("lw $t9 ($t8)");
                return reg2str.at($t9);
            }
        }
    }
}

string getValueOfVar(string &varName, function &fn) {
    string varName1 = getRealName(varName, '[');
    bool fnContains = fn.contains(varName1);
    bool gtContains = globalTable.contains(varName1);
    if (fnContains || gtContains) {
        if (fnContains) {
            return getValueOfVarIml(fn, varName, varName1, fn.getToken(varName1), LOCAL_VAR);
        } else {
            return getValueOfVarIml(fn, varName, varName1, globalTable.getToken(varName1), GLOBAL_VAR);
        }
    }
    return "Wrong!!!";
}

string getRealName(const string &varName, char end) {
    string ret;
    for (auto &ch:varName) {
        if (ch == end)
            break;
        ret += ch;
    }
    return ret;
}

void generateLabel(vector<string> &inter) {
    string label = inter.at(0).substr(1, inter.at(0).size() - 1);
    mipsCode.emplace_back(label);
}

enum branchType getBranchType(string &ins) {
    if (ins.find("@bge") != string::npos) {/*NOLINT*/
        return BGE;
    } else if (ins.find("@beq") != string::npos) {/*NOLINT*/
        return BEQ;
    } else if (ins.find("@bne") != string::npos) {/*NOLINT*/
        return BNE;
    } else if (ins.find("@blt") != string::npos) {/*NOLINT*/
        return BLT;
    } else if (ins.find("@j") != string::npos) {/*NOLINT*/
        return J;
    }
    return NOT_A_FUCKING_BRANCH;
}

void generateBranchInsIml(enum branchType type, vector<string> &inter, function &fn) {
    string var1 = getRealName(inter.at(1), '[');
    string var2 = getRealName(inter.at(2), '[');
    varType ty1 = whatIsThisShit(var1, fn);
    varType ty2 = whatIsThisShit(var2, fn);
    string varName1, varName2;
    if (ty1 == INTEGER || ty1 == CHARACTER) {
        mipsCode.emplace_back("li $t9 " + var1);
        varName1 = reg2str.at($t9);
    } else if (ty1 == TEMP_VAR) {
        if (fn.varHasReg(var1))
            varName1 = reg2str.at(fn.getRegOfVar(var1, true));
        else {
            varName1 = to_string(fn.getOffsetOfTemp(var1, true)) + "($sp)";
        }
    } else
        varName1 = getValueOfVar(inter.at(1), fn);
    varType ty1Neo = whatIsThisShit(varName1, fn);
    if (ty1Neo == INTEGER || ty1Neo == CHARACTER) {
        mipsCode.emplace_back("li $t9 " + varName1);
        varName1 = "$t9";
    }
    if (varName1 == "$t9") {
        if (fn.stackHasVar(var2) || var2mem.count(var2) != 0) {//后面可能会覆盖$t9
            if (!v1) {
                mipsCode.emplace_back("la $v1 ($t9)");
                varName1 = "$v1";
                v1 = true;
            } else {
                mipsCode.emplace_back("la $s7 ($t9)");
                varName1 = "$s7";
                s7 = true;
            }
        }
    }
    varName2 = (ty2 == INTEGER || ty2 == CHARACTER) ? var2
                                                    : (ty2 != TEMP_VAR) ? getValueOfVar(inter.at(2), fn)
                                                                        : (fn.varHasReg(var2)) ? reg2str.at(
                            fn.getRegOfVar(var2, true))
                                                                                               :
                                                                          to_string(fn.getOffsetOfTemp(var2, true)) +
                                                                          "($sp)";
    bool find1 = varName1.find("($sp)") != string::npos;
    bool find2 = varName2.find("($sp)") != string::npos;
    if (!find1 && !find2)
        mipsCode.emplace_back(
                branch[type] + " " + varName1 + " " + varName2 + " " + inter.at(3).substr(1, inter.at(3).size() - 1));
    else if (find1 && !find2) {
        mipsCode.emplace_back("lw $v1 " + varName1);
        mipsCode.emplace_back(
                branch[type] + " $v1 " + varName2 + " " + inter.at(3).substr(1, inter.at(3).size() - 1));
    } else if (!find1 && find2) {
        mipsCode.emplace_back("lw $v1 " + varName2);
        mipsCode.emplace_back(
                branch[type] + " " + varName1 + " $v1 " + inter.at(3).substr(1, inter.at(3).size() - 1));
    } else {
        mipsCode.emplace_back("lw $t8 " + varName1);
        mipsCode.emplace_back("lw $t9 " + varName2);
        mipsCode.emplace_back(
                branch[type] + " $t8 $t9 " + inter.at(3).substr(1, inter.at(3).size() - 1));
    }
    v1 = false;
    s7 = false;
}

void generateBranchIns(enum branchType type, vector<string> &inter, function &fn) {
//    fn.functionReturn();
    switch (type) {
        case BGE:
        case BEQ:
        case BNE:
        case BLT: {
            generateBranchInsIml(type, inter, fn);
            break;
        }
        case J: {
            mipsCode.emplace_back(branch[J] + " " + inter.at(1).substr(1, inter.at(1).size() - 1));
            break;
        }
        default:
            break;
    }
}

void generateReturnStatement(vector<string> &inter, function &fn) {
    string name = fn.getName();
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (name == "main") {
        mipsCode.emplace_back("li $v0 10");
        mipsCode.emplace_back("syscall");
        return;
    }
    if (inter.size() > 1) {
        string varName = getRealName(inter.at(1), '[');
        varType ty = whatIsThisShit(varName, fn);
        if (ty == LOCAL_VAR || ty == GLOBAL_VAR) {
            string var = getValueOfVar(inter.at(1), fn);
            varType ty2 = whatIsThisShit(var, fn);
            if (ty2 != INTEGER && ty2 != CHARACTER) {
                mipsCode.emplace_back("la $v0 (" + var + ")");
            } else {
                mipsCode.emplace_back("li $v0 " + var);
            }
        } else if (ty == TEMP_VAR) {
            if (fn.varHasReg(varName)) {
                string var = reg2str.at(fn.getRegOfVar(varName, true));
                mipsCode.emplace_back("la $v0 (" + var + ")");
            } else {
                mipsCode.emplace_back("lw $v0 " + to_string(fn.getOffsetOfTemp(varName, true)) + "($sp)");
            }
        } else {
            mipsCode.emplace_back("li $v0 " + inter.at(1));
        }
    }
    fn.functionReturn();
    mipsCode.emplace_back("jr $ra");
    enter();
}

void allocateParas(function &fn) {
    int argNum = fn.getArgumentNum();
    for (int i = 0; i < argNum && i < 4; i++) {
        fn.setRegToVar(fn.getArg(i).getName(), Reg($a0 + i));
    }
    if (fn.getArgumentNum() > 0) {
        string a0Name = fn.getArg(0).getName();
        fn.deleteVarInReg(a0Name, $a0);
        variableArrangeSpace(fn, a0Name, LOCAL_VAR);
        mipsCode.emplace_back("la " + reg2str.at(fn.getRegOfVar(a0Name, false)) + " ($a0)");
    }
    if (argNum > 4) {
        for (int i = 4; i < argNum; i++) {
            string varName = fn.getArg(i).getName();
            variableArrangeSpace(fn, varName, LOCAL_VAR);
            if (fn.varHasReg(varName))
                mipsCode.emplace_back(
                        "lw " + reg2str.at(fn.getRegOfVar(varName, false)) + " " + to_string((argNum - i) * 4 - 4) +
                        "($sp)");
            else {
                mipsCode.emplace_back("lw $t8 " + to_string((argNum - i) * 4) + "($sp)");
                mipsCode.emplace_back("sw $t8 " + to_string(fn.getOffsetOfTemp(varName, false)) + "($sp)");
            }
        }
        mipsCode.emplace_back("addi $sp $sp " + to_string((argNum - 4) * 4));
        //fn.spUpOff += (argNum - 4) * 4;此时函数还未分配栈空间，因此spOffset不用改变（否则就大于零了）
    }
}

void generateFunctionCall(vector<string> &inter, function &fn) {
    mipsCode.emplace_back("jal " + inter.at(1));
    enter();
    fn.generateLoadAfterCall();
}

void generatePushStatement(vector<string> &inter, function &fn) {
    varType type = whatIsThisShit(inter.at(1), fn);
    string a0;
    if (type == INTEGER || type == CHARACTER) {
        mipsCode.emplace_back("li $t8 " + inter.at(1));
        a0 = reg2str.at($t8);
    } else if (type == TEMP_VAR) {
        if (fn.varHasReg(inter.at(1)))
            a0 = reg2str.at(fn.getRegOfVar(inter.at(1), true));
        else {
            mipsCode.emplace_back("lw $t8 " + to_string(fn.getOffsetOfTemp(inter.at(1), true)) + "($sp)");
            a0 = reg2str.at($t8);
        }
    } else {
        a0 = getValueOfVar(inter.at(1), fn);
        varType type2 = whatIsThisShit(a0, fn);
        if (type2 == INTEGER || type2 == CHARACTER) {
            mipsCode.emplace_back("li $t8 " + a0);
            a0 = reg2str.at($t8);
        }
    }
    if (pushNum >= 4) {
        mipsCode.emplace_back("addi $sp $sp -4");
        mipsCode.emplace_back("sw " + a0 + " 0($sp)");
        fn.spUpOff -= 4;
    } else {
        mipsCode.emplace_back(
                "addu " + reg2str.at(Reg($a0 + pushNum)) + " " + a0 + " $0");
    }
    pushNum++;
}
