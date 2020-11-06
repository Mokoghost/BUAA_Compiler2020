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


enum tokenCategory {
    IDENFR = 0, INTCON, CHARCON, STRCON, CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK,
    ELSETK, SWITCHTK, CASETK, DEFAULTTK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK, PLUS,
    MINU, MULT, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, COLON,
    ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
};
enum identifierCategory {
    CONST = 0, VAR, ARG
};
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
public:
    Identifier(string name, enum tokenCategory type);//初始化一个标识符

    void setIdentifierCategory(enum identifierCategory kind);//设置标识符种类（Const、Arg、Var）

    enum identifierCategory getIdCategory();//获取标识符种类（Const、Arg、Var）

    void setDimension(int d, int num);//设置某一维大小

//    int getDimension(int d);//获取某一维大小
};


class identityTable {
private:
    map<string, Identifier> tokens;
public:
    identityTable();

    void addToken(Identifier &token, int l);//添加一个Identifier

    Identifier getToken(const string &name);//以名称为key获取一个Identifier

    bool contains(const string &name);//是否包含一个Identifier
};


class function : public tokenType {
private:
    vector<Identifier> args;
    int argNum{};
    identityTable tokenTable;
public:
    int returnNum;

    function();

    function(string name, enum tokenCategory type);//构造函数

    void addToken(Identifier &token, int l);//添加一个Identifier

    void setArgument(const Identifier &arg);//添加一个arg

    int getArgumentNum() const;//获取arg数目

    Identifier getArg(int index);//顺序获取一个arg

    Identifier getToken(const string &name);//以名称为key获取一个Identifier

    bool contains(string name);//是否包含一个Identifier
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
extern char tokenCategoryString[39][10];
extern char reserves[15][10];

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
int isReserve()/*是否为保留字*/, transNum()/*数字转换*/, isEof()/*是否为EOF*/;

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

extern char syntaxCategoryString[37][40];
extern int Int;

//只要是可选的，必须有空串的判断（也即传入一个参数，若是可选的，则允许为空；若是必选的，则不允许为空）。optional为1说明可选，若为0说明必选。
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
int expression(function &fn);//表达式
int term(function &fn);//项
int factor(function &fn);//因子
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
int isRelation();//关系运算符
int judgeSection();//判断是程序的拿一部分
bool isStatement();//判断是否为语句
int statementsWithSemi(function &fn);//以分号结尾的语句
int statementsWithoutSemi(function &fn);//不以分号结尾的语句
int isCharacter();//判断token是否为CHARCON
bool isCharExp(function &fn);//判断表达式是否为char类型
bool expressTypeEqual(int expressType, enum tokenCategory tc);//判断expression类型是否和某类型相同
void retractTokens(int numOfTokens);//退回token，你说个数吧，要退多少个
void getTokens(int numOfTokens);//读入token，你说个数吧，要读多少个
void retractToToken(int index);//输入为某个token在栈中的位置
int isOperation();//判断是否为运算符


/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////GenerateIntermediateCode///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
extern identityTable consts;//定义的所有const（由于保证程序语法正确，因此不区分作用域）
extern vector<string> intermediateCode;//中间代码
void addQuaternion(vector<string> strings);//增加一条四元式


/////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////GenerateMIPSAssembly/////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
void addMIPSAssembly();//增加一条汇编代码