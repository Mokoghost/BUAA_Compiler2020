#define TRUE        1
#define FALSE       0
//#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <utility>
#include <math.h>

enum tokenCategory {
    IDENFR = 0, INTCON, CHARCON, STRCON, CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK,
    ELSETK, SWITCHTK, CASETK, DEFAULTTK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK, PLUS,
    MINU, MULT, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, COLON,
    ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};
enum identifierCategory {
    CONST = 0, VAR, ARG
};
enum Reg {
    $zero = 0, $at, $v0, $v1, $a0, $a1, $a2, $a3, $t0, $t1, $t2, $t3, $t4, $t5, $t6, $t7, $s0,
    $s1, $s2, $s3, $s4, $s5, $s6, $s7, $t8, $t9, $gp, $sp, $fp, $ra, nullReg
};
enum varType {
    STR_VAR = 1, LOCAL_VAR, GLOBAL_VAR, TEMP_VAR, EQUAL, CHARACTER, INTEGER, OP
};
enum syscallType {
    PRINT_INT, PRINT_STR, READ_INT, EXIT, PRINT_CHAR, READ_CHARACTER
};
//enum mipsOp {
//    add, addi, addiu, addu, sub, subu, And, andi, nor, Or, ori, Xor, xori, sll, srl, sra, sllv, srlv, srav, slt, slti,
//    sltiu, sltu, beq, bne, blt, bgt, ble, blez, bge, bgez, j, jal, jr, jalr, move,mfc0,mfho, mflo, mtc0, mthi, mtlo, mult,
//    multu, Div, divu, lb, lbu, lh, lhu, lui, lw, li, la, sb, sh, sw, syscall, eret, Break
//};
using namespace std;
extern FILE *testfile;//
extern ofstream out;//语法分析&词法分析输出
extern ofstream err;//错误处理输出
extern ofstream midGross;//中间代码优化前
extern ofstream midOptimize;//中间代码优化后
extern ofstream mips;//目标代码生成
extern int line;

void error_syntax(const string &errorInfo);

void error_lexical();

class tokenType {
private:
    enum tokenCategory type;
    string name;
public:
    tokenType(string name, enum tokenCategory type);//初始化一个token（构造函数）

    tokenType();

    void iniTokenType(string fName, enum tokenCategory fType);//初始化一个token

    enum tokenCategory getTokenCategory();//获取token类型，对于标识符有int、char两种，对于函数还有void类

    string getName();//获取token名称

    bool isEqualType(tokenCategory type2);//是否相同类型
};

class Identifier : public tokenType {
private:
    Identifier();

    int dimension1;
    int dimension2;
    enum identifierCategory kind;
    vector<char> charValue;
    vector<int> intValue;
    enum Reg reg;
    string memoryAddress;
public:
    bool initialized;
    bool changed;

    Identifier(string name, enum tokenCategory type);//初始化一个标识符

    void setIdentifierCategory(enum identifierCategory kind);//设置标识符种类（Const、Arg、Var）

    enum identifierCategory getIdCategory();//获取标识符种类（Const、Arg、Var）

    void setDimension(int d, int num);//设置某一维大小

    int getDimension(int d);//获取某一维大小

    void setValue(int value);//设置常量或变量的值

    void setValue(char value);//设置常量或变量的值

    int getIntValue(int index1, int index2);//获取常量或变量的值

    char getCharValue(int index1, int index2);//获取常量或变量的值

    void setMemory(string memory);//存入内存

    void allocateRegister(enum Reg reg1);//分配寄存器

    void removeRegister();//从寄存器中取出

    int getSize() const;//获取空间大小

    void change();//标记变量已经改变
};


class identityTable {
private:
    map<string, Identifier> tokens;
public:
    identityTable();

    void addToken(Identifier &token, int l);//添加一个Identifier

    Identifier getToken(const string &name);//以名称为key获取一个Identifier

    bool contains(const string &name);//是否包含一个Identifier

    map<string, Identifier> getTokens();//获取所有变量

    void varChange(string varName);//改变某变量的状态（已被赋值）
};


class function : public tokenType {
private:
    vector<Identifier> args;
    int argNum{};
    identityTable tokenTable;
public:
    int returnNum;
    int tempNum;

    function();

    function(string name, enum tokenCategory type);//构造函数

    void addToken(Identifier &token, int l);//添加一个Identifier

    void setArgument(const Identifier &arg);//添加一个arg

    int getArgumentNum() const;//获取arg数目

    Identifier getArg(int index);//顺序获取一个arg

    Identifier getToken(const string &name);//以名称为key获取一个Identifier

    bool contains(const string &name);//标识符符号表中是否包含一个Identifier

    identityTable getTokenTable();//返回整个函数的符号表

    void varChange(string varName);//改变某变量的状态（已被赋值）
};

class functionTable {
private:
    map<string, function> tokens;
public:
    void addToken(function &fn, int l);//添加一个函数

    function getToken(const string &name);//以名称为key获取一个函数

    bool contains(const string &name);//是否包含某函数
};

extern vector<enum tokenCategory> formerTokens;//token栈
extern vector<pair<string, int>> formerAns;//输入栈(输入的词+词所在行数)
extern vector<string> outputAns;//输出栈
extern vector<string> outputErr;//错误输出栈
extern functionTable functions;//函数符号表
extern identityTable globalTable;//全局标识符符号表


/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////lexicalAnalysis///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
extern int num;
extern int tokenPointer;
extern enum tokenCategory Token;
extern const char tokenCategoryString[39][10];
extern const char reserves[15][10];

int getToken();//词法分析
int lexAnalysisIm();//词法分析实现
int getChar();//读取字符
void clearToken();//清空token
int isSpace();

int isLetter(), isDigit(), isChar();

int isColon(), isComma(), isSemi(), isQuote(), isApostrophe();

int isEqu(), isGre(), isLss(), isExcla();

int isPlus(), isMinus(), isDivi(), isMult();

int isLpar(), isRpar(), isLbrack(), isRbrack(), isLbrace(), isRbrace();

void catToken();//拼接字符
void retractChar();//退回一个字符
int isReserve()/*是否为保留字*/, isEof()/*是否为EOF*/;

int isStringChar();//是否是string中的合法字符


/////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////syntaxAnalysis////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAIN 0
#define VARIABLE_DESCRIPTION_WITH_INITIAL 1
#define VARIABLE_DESCRIPTION_WITHOUT_INITIAL 2
#define FUNCTION_DEFINE_WITH_RETURN 3
#define FUNCTION_DEFINE_WITHOUT_RETURN 4
#define READ_STATEMENT 5
#define WRITE_STATEMENT 6
#define ASSIGNMENT_STATEMENT 7
#define CONDITION_STATEMENT 8
#define SWITCH_STATEMENT 9
#define RETURN_STATEMENT 10
#define LOOP_STATEMENT 11
#define FUNCTION_CALL_STATEMENT_WITH_RETURN 12
#define FUNCTION_CALL_STATEMENT_WITHOUT_RETURN 13
#define STATEMENT_LIST 14
#define BLOCK_STATEMENT 15

//extern char syntaxCategoryString[37][40];

/*只要是可选的，必须有空串的判断（也即传入一个参数，若是可选的，则允许为空；
 * 若是必选的，则不允许为空）。optional为1说明可选，若为0说明必选。
*/
//所有函数调用前必须首先getToken()
//所有函数结束调用时不要getToken()

int String(int optional);//字符串
int program();//程序
int constDescription(int optional, function &fn);//常量说明
int constDefine(function &fn);//常量定义
//无符号整数，context为1说明上文为变量定义及初始化和switch语句，为2时说明是数组元素的下标
int unsignedInteger(bool output, int optional, int context);

int integer(int optional, int context);//整数
int statementHead(function &fn);//声明头部
int Const();//常量
int variableDescription(int optional, function &fn);//变量说明
int variableDefine(int optional, function &fn);//变量定义
int variableDefineWithoutInitial(function &fn);//变量定义无初始化
int variableDefineWithInitial(function &fn);//变量定义及初始化
int functionDefineWithReturn(int optional);//有返回值函数定义
int functionDefineWithoutReturn(int optional);//无返回值函数定义
int compoundStatement(function &fn);//复合语句
int parametersTable(function &fn);//参数表
int mainFunction();//主函数
int expression(function &fn, string &vn);//表达式
int term(function &fn, string &vn);//项
int factor(function &fn, string &vn);//因子
int statement(int optional, function &fn);//语句
int assignmentStatement(int optional, function &fn);//赋值语句
int conditionStatement(int optional, function &fn);//条件语句
int condition(function &fn);//条件
int loopStatement(int optional, function &fn);//循环语句
int stride();//步长
int switchStatement(function &fn);//情况语句
int caseTable(function &fn, enum tokenCategory type);//情况表
int caseStatement(int optional, function &fn, enum tokenCategory type);//情况子语句
int defaultStatement(function &fn);//缺省
int functionCallStatementWithReturn(function &caller);//有返回值函数调用语句
int functionCallStatementWithoutReturn(function &caller);//无返回值函数调用语句
int valueParameterTable(function &fn, function &caller);//值参数表
int statementList(function &fn);//语句列
int readStatement(int optional, function &fn);//读语句
int printStatement(int optional, function &fn);//写语句
int returnStatement(function &fn);//返回语句
int synAnalysis();//语法分析

int isTypeIdentifier();//类型标识符
int equalType(enum tokenCategory type, int constRet);//类型标识符是否相同
//int isRelation();//关系运算符
int judgeSection();//判断是程序的拿一部分
bool isStatement();//判断是否为语句
int statementsWithSemi(function &fn);//以分号结尾的语句
int statementsWithoutSemi(function &fn);//不以分号结尾的语句
int isCharacter();//判断token是否为CHARCON
bool isCharExp(function &fn, string &vn);//判断表达式是否为char类型
bool expressTypeEqual(int expressType, enum tokenCategory tc);//判断expression类型是否和某类型相同
void retractTokens(int numOfTokens);//退回token，你说个数吧，要退多少个
void getTokens(int numOfTokens);//读入token，你说个数吧，要读多少个
void retractToToken(int index);//输入为某个token在栈中的位置
int isOperation();//判断是否为运算符


/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////GenerateIntermediateCode///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
extern vector<string> intermediateCode;//中间代码
extern vector<vector<string>> intermediateCodeInner;//从中间代码转MIPS时实际使用的
extern vector<string> identityType;

void addQuaternion(const vector<string> &quaternion);//增加一条四元式


/////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////GenerateMIPSAssembly/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define DATA_BASE_ADDRESS 0x10010000
extern vector<string> mipsCode;//目标代码
extern map<string, string> strings;//字符串
extern int strNum;//字符串编号
extern const char registers[32][6];//寄存器名
extern const int syscallService[6];//syscall服务
extern bool usedRegister[32];//已使用的寄存器
extern map<string, enum Reg> var2reg;//变量对应的寄存器
extern map<enum Reg, string> reg2var;//寄存器存储的变量
extern map<string, enum tokenCategory> tempType;//临时变量类型
extern map<string, int> var2mem;//变量对应的内存
extern map<int, string> mem2var;//内存地址存储的变量
extern enum Reg regPointer;//寄存器循环指针，永远指向下一个要用的寄存器

void enter();

void generateMIPSAssembly();//目标代码生成
void allocateMemory();//为全局变量分配内存
extern void generateFunctionMIPS(function &fn);//生成函数的MIPS代码
void variableArrangeRegister(function &fn, string &putInVar, enum varType type);//给变量分配寄存器

void generateScanMIPS(function &fn, vector<string> &inter);//生成MIPS读语句

void generatePrintMIPS(function &fn, vector<string> &inter);//生成

void globalVariableLoad(function &fn);//将全局变量存入内存

void generateStatement(function &fn, vector<string> &inter);

int whatIsThisShit(string &varName, function &fn);

void generatePrintEnter();

string getValueOfVar(string &varName, function &fn);

string getRealName(string varName, char end);//消除某符号对变量名称的影响

void makeChanged(string varName,function &fn);//标记变量已经改变