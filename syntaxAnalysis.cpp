#include "compiler.h"

functionTable functions;//函数符号表
identityTable globalTable;/* NOLINT */
int num;
//char syntaxCategoryString[37][40]
//        = {
//                "<字符串>", "<程序>", "<常量说明>", "<常量定义>", "<无符号整数>", "<整数>", "<声明头部>", "<常量>",
//                "<变量说明>", "<变量定义>", "<变量定义无初始化>", "<变量定义及初始化>", "<有返回值函数定义>", "<无返回值函数定义>",
//                "<复合语句>", "<参数表>", "<主函数>", "<表达式>", "<项>", "<因子>", "<语句>", "<赋值语句>", "<条件语句>",
//                "<条件>", "<循环语句>", "<步长>", "<情况语句>", "<情况表>", "<情况子语句>", "<缺省>", "<有返回值函数调用语句>",
//                "<无返回值函数调用语句>", "<值参数表>", "<语句列>", "<读语句>", "<写语句>", "<返回语句>"};

int synAnalysis() {
    getChar();
    getToken();
    program();
    return 0;
}

int String(int optional) {//字符串
    if (optional)
        if (Token != STRCON)
            return 0;
    if (Token == STRCON) {

        //outputAns.emplace_back(syntaxCategoryString[0]);
        return 1;
    }
    return 0;
}

int program() {//程序
    function f0;
    if (constDescription(1, f0))
        getToken();
    if (variableDescription(1, f0))
        getToken();
    while (functionDefineWithReturn(1) || functionDefineWithoutReturn(1))
        getToken();
    mainFunction();
    //outputAns.emplace_back(syntaxCategoryString[1]);
    return 1;
}

int constDescription(int optional, function &fn) {//常量说明
    if (optional)
        if (Token != CONSTTK)
            return 0;
    //Token == CONSTTK
    getToken();
    constDefine(fn);
    getToken();
    if (Token != SEMICN) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " k");
        retractTokens(1);
    }
    getToken();
    while (Token == CONSTTK) {
        getToken();
        constDefine(fn);
        getToken();
        if (Token != SEMICN) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " k");
            retractTokens(1);
        }
        getToken();
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[2]);
    return 1;
}

int constDefineInt(function &fn) {
    getToken();//IDENFR
    int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    string id = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    getToken();//ASSIGN
    getToken();
    integer(0, 1);
    Identifier con(id, INTTK);
    con.setValue(num);
    con.setDimension(1, 1);
    con.setDimension(2, 1);
    con.setIdentifierCategory(CONST);
    if (fn.getName().empty())
        globalTable.addToken(con, l);
    else
        fn.addToken(con, l);
    getToken();
    return 1;
}

int constDefineChar(function &fn) {
    getToken();//IDENFR
    int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    string id = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    getToken();//ASSIGN
    getToken();//CHARCON
    Identifier con(id, CHARTK);
    con.setValue(formerAns.at(formerAns.size() - 1 - tokenPointer).first[0]);
    con.setDimension(1, 1);
    con.setDimension(2, 1);
    con.setIdentifierCategory(CONST);
    if (fn.getName().empty())
        globalTable.addToken(con, l);
    else
        fn.addToken(con, l);
    getToken();
    return 1;
}

int constDefine(function &fn) {//常量定义
    if (Token == INTTK) {
        constDefineInt(fn);
        while (Token == COMMA) {
            constDefineInt(fn);
        }
    } else if (Token == CHARTK) {
        constDefineChar(fn);
        while (Token == COMMA) {
            constDefineChar(fn);
        }
    } else {}//impossible to reach
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[3]);
    return 1;
}

int unsignedInteger(bool output, int optional, int context) {//无符号整数
    if (optional)
        if (Token != INTCON)
            return 0;
    if (!output)
        if (Token == INTCON)
            return 1;
    if (Token == INTCON) {
        //outputAns.emplace_back(syntaxCategoryString[4]);
        return 1;
    }
    if (context == 1)
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " o");
    else
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " i");
    return 0;
}

int integer(int optional, int context) {//整数
    if (optional)
        if (!(Token == PLUS || Token == MINU || unsignedInteger(false, 1, context)))
            return 0;
    int sym = 1;
    if (Token == PLUS || Token == MINU) {
        sym = (Token == PLUS) ? 1 : -1;
        getToken();
    }
    unsignedInteger(true, 0, context);
    //outputAns.emplace_back(syntaxCategoryString[5]);
    num = stoi(formerAns.at(formerAns.size() - 1 - tokenPointer).first) * sym;
    return 1;
}

int statementHead(function &fn) {//声明头部
    //Token == INTTK || Token == CHARTK
    enum tokenCategory type = Token;
    getToken();
    string fName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    fn.iniTokenType(fName, type);
    //outputAns.emplace_back(syntaxCategoryString[6]);
    return 1;
}

int Const() {//常量
    if (integer(1, 1)) {
        //outputAns.emplace_back(syntaxCategoryString[7]);
        return 1;//INTTK
    } else if (isCharacter()) {
        //outputAns.emplace_back(syntaxCategoryString[7]);
        return 2;//CHARTK
    }
    return 0;
}

int variableDescription(int optional, function &fn) {//变量说明
    if (optional)
        if (!isTypeIdentifier())
            return 0;
    int judge = judgeSection();
    if (!(judge == VARIABLE_DESCRIPTION_WITHOUT_INITIAL ||
          judge == VARIABLE_DESCRIPTION_WITH_INITIAL))
        return 0;
    variableDefine(0, fn);
    getToken();
    while (Token == SEMICN) {
        getToken();
        if (!variableDefine(1, fn)) {
            break;
        }
        getToken();
    }
    retractTokens(1);
    if (Token != SEMICN)
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " k");
    //outputAns.emplace_back(syntaxCategoryString[8]);
    return 1;
}

int variableDefine(int optional, function &fn) {//变量定义
    if (optional) {
        int judge = judgeSection();
        if (!(judge == VARIABLE_DESCRIPTION_WITHOUT_INITIAL ||
              judge == VARIABLE_DESCRIPTION_WITH_INITIAL))
            return 0;
    }
    if (variableDefineWithoutInitial(fn) || variableDefineWithInitial(fn)) {
        //outputAns.emplace_back(syntaxCategoryString[9]);
        return 1;
    }
    return 0;
}

int variableDefineWithoutInitialIm(enum tokenCategory ty, function &fn) {
    string name = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    Identifier var(name, ty);
    getToken();
    if (Token == LBRACK) {//＜标识符＞'['＜无符号整数＞']'
        getToken();
        unsignedInteger(true, 0, 2);
        int d1 = stoi(formerAns.at(formerAns.size() - 1 - tokenPointer).first);//第一维数
        var.setDimension(1, d1);
        getToken();
        if (Token != RBRACK) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
            retractTokens(1);
        }
        getToken();
        if (Token == LBRACK) {
            getToken();
            unsignedInteger(true, 0, 2);
            int d2 = stoi(formerAns.at(formerAns.size() - 1 - tokenPointer).first);//第二维数
            var.setDimension(2, d2);
            getToken();
            if (Token != RBRACK) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
                retractTokens(1);
            }
            getToken();
        } else
            var.setDimension(2, 1);
    } else {//＜标识符＞
        var.setDimension(1, 1);
        var.setDimension(2, 1);
    }
    var.setIdentifierCategory(VAR);
    if (fn.getName().empty()) {
        globalTable.addToken(var, l);
    } else
        fn.addToken(var, l);
    return 1;
}

int variableDefineWithoutInitial(function &fn) {//变量定义无初始化
    if (judgeSection() != VARIABLE_DESCRIPTION_WITHOUT_INITIAL)
        return 0;
    //isTypeIdentifier()
    enum tokenCategory ty = Token;
    getToken();
    while (Token == IDENFR) {
        variableDefineWithoutInitialIm(ty, fn);
        if (Token == COMMA)
            getToken();
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[10]);
    return 1;
}

int variableDefineWithInitialIm2(enum tokenCategory type, int d1, Identifier &var) {
    getToken();
    int ty = 0;//1: o 2: n
    for (int i = 0; i < d1; i++) {
        int constRet = Const();
        if (!constRet) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " n");
            ty = 2;
            break;
        }
        if (!equalType(type, constRet) && ty == 0) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " o");
            ty = 1;
        }
        if (type == INTTK)
            var.setValue(num);
        else
            var.setValue(formerAns.at(formerAns.size() - 1 - tokenPointer).first[0]);
        getToken();//COMMA
        getToken();
    }
    if (!ty) {
        if (Const()) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " n");
            getToken();
            while (Token == COMMA) {
                getToken();
                getToken();//COMMA OR RBRACE
            }
        } else
            retractTokens(1);
    } else {
        retractTokens(1);
    }
    return 1;
}

int variableDefineWithInitialIm(enum tokenCategory type, function &fn) {
    string name = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    Identifier var(name, type);
    getToken();
    if (Token == LBRACK) {//若为数组
        getToken();
        unsignedInteger(true, 0, 1);
        int d1 = stoi(formerAns.at(formerAns.size() - 1 - tokenPointer).first);//第一维数
        var.setDimension(1, d1);
        getToken();
        if (Token != RBRACK) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
            retractTokens(1);
        }
        getToken();
        if (Token == LBRACK) {//若为二维数组
            getToken();
            unsignedInteger(true, 0, 1);
            int d2 = stoi(formerAns.at(formerAns.size() - 1 - tokenPointer).first);//第二维数
            var.setDimension(2, d2);
            getToken();
            if (Token != RBRACK) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
                retractTokens(1);
            }
            getToken();//ASSIGN
            int counter1 = 0;
            getToken();//LBRACE
            getToken();
            while (Token == LBRACE) {
                counter1++;
                variableDefineWithInitialIm2(type, d2, var);
                if (counter1 == d1) {
                    break;
                }
                getToken();//COMMA
                getToken();
            }
            if (counter1 != d1) {
                retractTokens(1);
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " n");
            }
            getToken();
            if (Token != RBRACE) {
                getToken();
                while (Token == LBRACE) {
                    variableDefineWithInitialIm2(type, d2, var);
                    getToken();//COMMA
                    getToken();
                }
                retractTokens(1);
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " n");
            }
        } else if (Token == ASSIGN) {//若为一维数组
            var.setDimension(2, 1);
            getToken();//LBRACE
            variableDefineWithInitialIm2(type, d1, var);
        }
    } else if (Token == ASSIGN) {//若为简单变量
        getToken();
        var.setDimension(1, 1);
        var.setDimension(2, 1);
        int constRet = Const();
        if (constRet) {
            if (!equalType(type, constRet)) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " o");
            }
        }
        if (type == INTTK)
            var.setValue(num);
        else
            var.setValue(formerAns.at(formerAns.size() - 1 - tokenPointer).first[0]);
    } else {}//impossible to reach
    var.setIdentifierCategory(VAR);
    if (fn.getName().empty()) {
        globalTable.addToken(var, l);
    } else
        fn.addToken(var, l);
    getToken();
    return 1;
}


int variableDefineWithInitial(function &fn) {//变量定义及初始化
    if (judgeSection() != VARIABLE_DESCRIPTION_WITH_INITIAL)
        return 0;
    //isTypeIdentifier()
    enum tokenCategory type = Token;//标识符类型(int,char)
    getToken();
    while (Token == IDENFR) {
        variableDefineWithInitialIm(type, fn);
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[11]);
    return 1;
}

int functionDefineWithReturn(int optional) {//有返回值函数定义
    if (optional)
        if (!isTypeIdentifier())
            return 0;
    if (judgeSection() != FUNCTION_DEFINE_WITH_RETURN)
        return 0;
    function fn;
    getToken();
    int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    retractTokens(1);
    statementHead(fn);
    getToken();//LPARENT
    getToken();
    parametersTable(fn);
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    getToken();//LBRACE
    getToken();
    compoundStatement(fn);
    getToken();//RBRACE
    if (!fn.returnNum)
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " h");
    functions.addToken(fn, l);
    //outputAns.emplace_back(syntaxCategoryString[12]);
    return 1;
}

int functionDefineWithoutReturn(int optional) {//无返回值函数定义
    if (optional)
        if (Token != VOIDTK)
            return 0;
    if (judgeSection() != FUNCTION_DEFINE_WITHOUT_RETURN)
        return 0;
    //Token==VOIDTK
    getToken();// IDENFR
    int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    string fName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    function fn(fName, VOIDTK);
    getToken();//LPARENT
    getToken();
    parametersTable(fn);
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    getToken();//LBRACE
    getToken();
    compoundStatement(fn);
    getToken();//RBRACE
    functions.addToken(fn, l);
    //outputAns.emplace_back(syntaxCategoryString[13]);
    return 1;
}

int compoundStatement(function &fn) {//复合语句
    if (constDescription(1, fn))
        getToken();
    if (variableDescription(1, fn))
        getToken();
    int retNum = statementList(fn);
    //outputAns.emplace_back(syntaxCategoryString[14]);
    return retNum;
}

int parametersTable(function &fn) {//参数表
    while (isTypeIdentifier()) {
        enum tokenCategory ty = Token;
        getToken();//IDENFR
        int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
        Identifier arg(formerAns.at(formerAns.size() - 1 - tokenPointer).first, ty);
        arg.setDimension(1, 1);
        arg.setDimension(2, 1);
        arg.setIdentifierCategory(ARG);
        fn.setArgument(arg);
        fn.addToken(arg, l);
        getToken();
        if (Token != COMMA) {
            break;
        }
        getToken();
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[15]);
    return 1;
}

int mainFunction() {//主函数
    //Token == VOIDTK
    getToken();//MAINTK
    function mainFunc(formerAns.at(formerAns.size() - 1 - tokenPointer).first, VOIDTK);
    int l = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    getToken();//LPARENT
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    addQuaternion(vector<string>{"void", "main()"});
    getToken();//LBRACE
    getToken();
    compoundStatement(mainFunc);
    getToken();//RBRACE
    functions.addToken(mainFunc, l);
    //outputAns.emplace_back(syntaxCategoryString[16]);
    return 1;
}

int expression(function &fn, string &vn) {//表达式
    int ret = 0;
    if (isCharExp(fn, vn)) {
        return 2;
    }
    string op0;
    if (Token == PLUS || Token == MINU) {
        op0 = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
        getToken();
    }
    string vt0;
    ret -= term(fn, vt0);
    string vt1 = "$t" + to_string(fn.tempNum++);
    addQuaternion(vector<string>{vt1, "=", op0, vt0});
    getToken();
    while (Token == PLUS || Token == MINU) {
        string op = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
        getToken();
        string vt2;
        ret -= term(fn, vt2);
        string vtn = "$t" + to_string(fn.tempNum++);
        addQuaternion(vector<string>{vtn, "=", vt1, op, vt2});
        vt1 = vtn;
        getToken();
    }
    vn = vt1;
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[17]);
    return ret;
}

int term(function &fn, string &vn) {//项
    string vt1;
    int ret = factor(fn, vt1);
    vn = vt1;
    getToken();
    while (Token == MULT || Token == DIV) {
        string op = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
        getToken();
        string vt2;
        ret += factor(fn, vt2);
        string vtn = "$t" + to_string(fn.tempNum++);
        addQuaternion(vector<string>{vtn, "=", vt1, op, vt2});
        vt1 = vtn;
        vn = vtn;
        getToken();
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[18]);
    return ret;
}

int factor(function &fn, string &vn) {//因子
    int ret = 0;
    if (Token == IDENFR) {//＜标识符＞
        string idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
        if (!fn.contains(idName) && !globalTable.contains(idName) && !functions.contains(idName) &&
            fn.getName() != idName) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " c");
            ret = 'c' - 'a' + 1;
        }
        getToken();
        if (Token == LBRACK) {//＜标识符＞'['＜表达式＞']'
            getToken();
            string vt1;
            if (expression(fn, vt1) == 2) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " i");
                ret = 'i' - 'a' + 1;
            }
            getToken();
            if (Token != RBRACK) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
                retractTokens(1);
                ret = 'm' - 'a' + 1;
            }
            getToken();
            if (Token == LBRACK) {//＜标识符＞'['＜表达式＞']''['＜表达式＞']'
                getToken();
                string vt2;
                if (expression(fn, vt2) == 2) {
                    error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " i");
                    ret = 'i' - 'a' + 1;
                }
                getToken();
                if (Token != RBRACK) {
                    error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
                    retractTokens(1);
                    ret = 'm' - 'a' + 1;
                }
                vn = idName + "[" + vt1 + "]" + "[" + vt2 + "]";
            } else {
                retractTokens(1);
                vn = idName + "[" + vt1 + "]";
            }
        } else if (Token == LPARENT) {//＜有返回值函数调用语句＞
            retractTokens(1);
            functionCallStatementWithReturn(fn);
        } else {
            retractTokens(1);
            vn = idName;
        }
    } else if (Token == LPARENT) {//'('＜表达式＞')'
        getToken();
        expression(fn, vn);
        getToken();
        if (Token != RPARENT) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
            retractTokens(1);
            ret = 'l' - 'a' + 1;
        }
    } else if (isCharacter()) {//＜字符＞
        vn = "\'" + formerAns.at(formerAns.size() - 1 - tokenPointer).first + "\'";
    } else if (integer(1, 2)) {//＜整数＞
        vn = to_string(num);
    }
    //outputAns.emplace_back(syntaxCategoryString[19]);
    return ret;
}

int statement(int optional, function &fn) {//语句
    if (optional)
        if (!isStatement())
            return 0;
    if (statementsWithSemi(fn)) {
        getToken();
        if (Token != SEMICN) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 2 - tokenPointer).second) + " k");
            retractTokens(1);
        }
    } else if (Token == LBRACE) {
        getToken();
        statementList(fn);
        getToken();//RBRACE
    } else if (Token == SEMICN);
    else {
        if (statementsWithoutSemi(fn));
        else {
            error_syntax("statement 4");
            return 0;
        }
    }
    //outputAns.emplace_back(syntaxCategoryString[20]);
    return 1;
}

int assignmentStatement(int optional, function &fn) {//赋值语句
    if (optional)
        if (Token != IDENFR)
            return 0;
    string idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    int idLine = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    int fnContains = fn.contains(idName);
    int gtContains = globalTable.contains(idName);
    if (!fnContains && !gtContains) {//IDENFR
        error_syntax(to_string(idLine) + " c");
    }
    if ((fnContains && fn.getToken(idName).getIdCategory() == CONST) ||
        (gtContains && globalTable.getToken(idName).getIdCategory() == CONST)) {
        error_syntax(to_string(idLine) + " j");
    }
    getToken();
    if (Token == ASSIGN) {//＝＜表达式＞
        getToken();
        string vt1;
        expression(fn, vt1);
        addQuaternion(vector<string>{idName, "=", vt1});
    } else if (Token == LBRACK) {
        getToken();
        string vt1;
        expression(fn, vt1);
        getToken();
        if (Token != RBRACK) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
            retractTokens(1);
        }
        getToken();
        if (Token == ASSIGN) {//'['＜表达式＞']'=＜表达式＞
            getToken();
            string vt2;
            if (expression(fn, vt2) == 2) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " i");
            }
            addQuaternion(vector<string>{idName, "[" + vt1 + "]", "=", vt2});
        } else if (Token == LBRACK) {//'['＜表达式＞']''['＜表达式＞']' =＜表达式＞
            getToken();
            string vt2;
            if (expression(fn, vt2) == 2) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " i");
            }
            getToken();
            if (Token != RBRACK) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
                retractTokens(1);
            }
            getToken();//ASSIGN
            getToken();
            string vt3;
            expression(fn, vt3);
            addQuaternion(vector<string>{idName, "[" + vt1 + "]", "[" + vt2 + "]", "=", vt3});
        }
    } else {}//impossible to reach
    //outputAns.emplace_back(syntaxCategoryString[21]);
    return 1;
}

int conditionStatement(int optional, function &fn) {//条件语句
    if (optional)
        if (Token != IFTK)
            return 0;
    //Token == IFTK
    getToken();//LPARENT
    getToken();
    condition(fn);
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    getToken();
    statement(0, fn);
    getToken();
    if (Token == ELSETK) {
        getToken();
        statement(0, fn);
    } else retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[22]);
    return 1;
}

int condition(function &fn) {//条件
    string vt1;
    if (expressTypeEqual(expression(fn, vt1), CHARTK)) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " f");
    }
    getToken();//isRelation();
    getToken();
    string vt2;
    if (expressTypeEqual(expression(fn, vt2), CHARTK)) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " f");
    }
    //outputAns.emplace_back(syntaxCategoryString[23]);
    return 1;
}

int loopStatement(int optional, function &fn) {//循环语句
    if (optional)
        if (Token != WHILETK && Token != FORTK)
            return 0;
    if (Token == WHILETK) {
        getToken();//LPARENT
        getToken();
        condition(fn);
        getToken();
        if (Token != RPARENT) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
            retractTokens(1);
        }
        getToken();
        statement(0, fn);
    } else if (Token == FORTK) {
        getToken();//LPARENT
        getToken();
        string idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
        if (!fn.contains(idName) && !globalTable.contains(idName)) {//IDENFR
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " c");
        }
        getToken();//ASSIGN
        getToken();
        string vt1;
        expression(fn, vt1);
        getToken();
        if (Token != SEMICN) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 2 - tokenPointer).second) + " k");//应为分号
            retractTokens(1);
        }
        getToken();
        condition(fn);
        getToken();
        if (Token != SEMICN) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 2 - tokenPointer).second) + " k");//应为分号
            retractTokens(1);
        }
        getToken();
        idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
        if (!fn.contains(idName) && !globalTable.contains(idName)) {//IDENFR
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " c");
        }
        getToken();//ASSIGN
        getToken();
        idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
        if (!fn.contains(idName) && !globalTable.contains(idName)) {//IDENFR
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " c");
        }
        getToken();//Token == PLUS || Token == MINU
        getToken();
        stride();
        getToken();
        if (Token != RPARENT) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
            retractTokens(1);
        }
        getToken();
        statement(0, fn);
    } else {}//impossible to reach
    //outputAns.emplace_back(syntaxCategoryString[24]);
    return 1;
}

int stride() {//步长
    unsignedInteger(true, 0, 1);
    //outputAns.emplace_back(syntaxCategoryString[25]);
    return 1;
}

int switchStatement(function &fn) {//情况语句
    int ret;
    //Token == SWITCHTK
    getToken();//LPARENT
    getToken();
    enum tokenCategory type;
    string vt1;
    ret = expression(fn, vt1);
    if (ret == 2)
        type = CHARTK;
    else
        type = INTTK;
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    getToken();//LBRACE
    getToken();
    caseTable(fn, type);
    getToken();
    defaultStatement(fn);
    getToken();//RBRACE
    //outputAns.emplace_back(syntaxCategoryString[26]);
    return 1;
}

int caseTable(function &fn, enum tokenCategory type) {//情况表
    caseStatement(0, fn, type);
    getToken();
    while (caseStatement(1, fn, type)) {
        getToken();
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[27]);
    return 1;
}

int caseStatement(int optional, function &fn, enum tokenCategory type) {//情况子语句
    if (optional)
        if (Token != CASETK)
            return 0;
    //Token == CASETK
    getToken();
    int constRet = Const();
    if (constRet) {
        if (!equalType(type, constRet)) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " o");
        }
    }
    getToken();//COLON
    getToken();
    statement(0, fn);
    //outputAns.emplace_back(syntaxCategoryString[28]);
    return 1;
}

int defaultStatement(function &fn) {//缺省
    if (Token != DEFAULTTK) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " p");
        retractTokens(1);
        return 0;
    }
    getToken();//COLON
    getToken();
    statement(0, fn);
    //outputAns.emplace_back(syntaxCategoryString[29]);
    return 1;
}

void findRparent(int lackP) {//寻找右括号
    while (lackP) {
        getToken();
        if (Token == RPARENT)
            lackP--;
        else if (Token == LPARENT)
            lackP++;
    }
}

int functionCallStatementWithReturn(function &caller) {//有返回值函数调用语句
    string idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    if (!functions.contains(idName) && caller.getName() != idName) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " c");
        getToken();//LPARENT
        findRparent(1);
        return 0;
    }
    function fn;
    if (caller.getName() != idName)
        fn = functions.getToken(formerAns.at(formerAns.size() - 1 - tokenPointer).first);
    else
        fn = caller;
    getToken();//LPARENT
    getToken();
    valueParameterTable(fn, caller);
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    //outputAns.emplace_back(syntaxCategoryString[30]);
    return 1;
}

int functionCallStatementWithoutReturn(function &caller) {//无返回值函数调用语句
    if (!functions.contains(formerAns.at(formerAns.size() - 1 - tokenPointer).first)) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " c");
        getToken();//LPARENT
        findRparent(1);
        return 0;
    }
    function fn = functions.getToken(formerAns.at(formerAns.size() - 1 - tokenPointer).first);
    getToken();//LPARENT
    getToken();
    valueParameterTable(fn, caller);
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    //outputAns.emplace_back(syntaxCategoryString[31]);
    return 1;
}

int valueParameterTable(function &fn, function &caller) {//值参数表
    int argNum = fn.getArgumentNum();
    if (Token == SEMICN && !argNum) {
        retractTokens(1);
        //outputAns.emplace_back(syntaxCategoryString[32]);
        return 0;
    }
    if ((Token == RPARENT && argNum) || (Token != RPARENT && !argNum)) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " d");
        retractTokens(1);
        findRparent(1);
        retractTokens(1);
        //outputAns.emplace_back(syntaxCategoryString[32]);
        return 0;
    }
    if (Token == RPARENT && !argNum) {
        retractTokens(1);
        //outputAns.emplace_back(syntaxCategoryString[32]);
        return 0;
    }
    int count = 0;
    string vt1;
    int expRet = expression(caller, vt1);
    if (expRet >= 0) {
        if (!expressTypeEqual(expRet, fn.getArg(0).getTokenCategory())) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " e");
        }
    }
    count++;
    getToken();
    while (Token == COMMA) {
        getToken();
        string vt2;
        expRet = expression(caller, vt2);
        if (expRet >= 0) {
            if (!expressTypeEqual(expRet, fn.getArg(0).getTokenCategory())) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " e");
            }
        }
        getToken();
        count++;
    }
    if (count != argNum) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " d");
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[32]);
    return 1;
}

int statementList(function &fn) {//语句列
    while (statement(1, fn)) {
//        formerAns.at(formerAns.size() - 1 - tokenPointer).second;
        getToken();
    }
    retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[33]);
    return 1;
}

int readStatement(int optional, function &fn) {//读语句
    if (optional)
        if (Token != SCANFTK)
            return 0;
    //Token == SCANFTK
    getToken();//LPARENT
    getToken();//IDENFR
    string idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
    int idLine = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
    int fnContains = fn.contains(idName);
    int gtContains = globalTable.contains(idName);
    if (!fnContains && !gtContains) {
        error_syntax(to_string(idLine) + " c");
    }
    if ((fnContains && fn.getToken(idName).getIdCategory() == CONST) ||
        (gtContains && globalTable.getToken(idName).getIdCategory() == CONST)) {
        error_syntax(to_string(idLine) + " j");
    }
    addQuaternion(vector<string>{"scan", idName});
    getToken();
    if (Token != RPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
        retractTokens(1);
    }
    //outputAns.emplace_back(syntaxCategoryString[34]);
    return 1;
}

int printStatement(int optional, function &fn) {//写语句
    if (optional)
        if (Token != PRINTFTK)
            return 0;
    //Token == PRINTFTK
    getToken();//LPARENT
    getToken();
    if (String(1)) {
        string strIndex = "#str" + to_string(strNum++);
        strings.insert(pair<string, string>(strIndex,
                                            formerAns.at(formerAns.size() - 1 - tokenPointer).first));
        string str = strIndex;
        getToken();
        if (Token == RPARENT) {
            addQuaternion(vector<string>{"print", str});
        } else if (Token == COMMA) {
            getToken();
            string vt1;
            expression(fn, vt1);
            getToken();
            if (Token != RPARENT) {
                error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
                retractTokens(1);
            }
            addQuaternion(vector<string>{"print", str, vt1});
        }
    } else {
        string vt1;
        expression(fn, vt1);
        getToken();
        if (Token != RPARENT) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
            retractTokens(1);
        }
        addQuaternion(vector<string>{"print", vt1});
    }
    //outputAns.emplace_back(syntaxCategoryString[35]);
    return 1;
}

int returnStatement(function &fn) {//返回语句
    getToken();
    if (fn.isEqualType(VOIDTK) && Token == LPARENT) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " g");
        while (Token != RPARENT)
            getToken();
        return 0;
    }
    if (!fn.isEqualType(VOIDTK) && Token == SEMICN) {
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " h");
        retractTokens(1);
        fn.returnNum++;
        return 0;
    }
    if (Token == LPARENT) {
        getToken();
        if (Token == RPARENT) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " h");
            fn.returnNum++;
            //outputAns.emplace_back(syntaxCategoryString[36]);
            return 1;
        }
        string vt1;
        if (!expressTypeEqual(expression(fn, vt1), fn.getTokenCategory()))
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " h");
        fn.returnNum++;
        getToken();
        if (Token != RPARENT) {
            error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
            retractTokens(1);
        }
    } else retractTokens(1);
    //outputAns.emplace_back(syntaxCategoryString[36]);
    return 1;
}

int isTypeIdentifier() {
    return (Token == INTTK || Token == CHARTK) ? TRUE : FALSE;
}

int equalType(enum tokenCategory type, int constRet) {
    if (constRet && (((type == INTTK) && (constRet == 1)) || ((type == CHARTK) && (constRet == 2))))
        return 1;
    return 0;
}

//int isRelation() {
//    return (Token == LSS || Token == LEQ ||
//            Token == GRE || Token == GEQ ||
//            Token == EQL || Token == NEQ) ? TRUE : FALSE;
//}

void error_syntax(const string &errorInfo) {
    outputErr.push_back(errorInfo);
}

int judgeSection() {
    int pos = int(formerAns.size() - 1 - tokenPointer);
    if (Token == VOIDTK) {
        getToken();
        if (Token == MAINTK) {
            retractTokens(1);
            return MAIN;
        } else {
            retractTokens(1);
            return FUNCTION_DEFINE_WITHOUT_RETURN;
        }
    } else if (Token == SCANFTK) {
        return READ_STATEMENT;
    } else if (Token == PRINTFTK) {
        return WRITE_STATEMENT;
    } else if (Token == RETURNTK) {
        return RETURN_STATEMENT;
    } else if (Token == FORTK || Token == WHILETK) {
        return LOOP_STATEMENT;
    } else if (Token == SWITCHTK) {
        return SWITCH_STATEMENT;
    } else if (Token == IFTK) {
        return CONDITION_STATEMENT;
    } else if (Token == LBRACE) {
        return STATEMENT_LIST;
    } else if (Token == SEMICN) {
        return BLOCK_STATEMENT;
    } else if (Token == IDENFR) {
        getToken();
        if (Token == ASSIGN || Token == LBRACK) {
            retractTokens(1);
            return ASSIGNMENT_STATEMENT;
        } else if (Token == LPARENT) {
            if (!functions.contains(formerAns.at(formerAns.size() - 2 - tokenPointer).first)) {
                retractTokens(1);
                return FUNCTION_CALL_STATEMENT_WITH_RETURN;
            }
            if (functions.getToken(
                    formerAns.at(formerAns.size() - 2 - tokenPointer).first).isEqualType(VOIDTK)) {
                retractTokens(1);
                return FUNCTION_CALL_STATEMENT_WITHOUT_RETURN;
            } else {
                retractTokens(1);
                return FUNCTION_CALL_STATEMENT_WITH_RETURN;
            }
        } else {}//impossible to reach
    } else if (isTypeIdentifier()) {
        getTokens(2);
        if (Token == SEMICN || Token == COMMA) {
            retractTokens(2);
            return VARIABLE_DESCRIPTION_WITHOUT_INITIAL;
        } else if (Token == LPARENT) {
            retractTokens(2);
            return FUNCTION_DEFINE_WITH_RETURN;
        } else if (Token == ASSIGN) {
            retractTokens(2);
            return VARIABLE_DESCRIPTION_WITH_INITIAL;
        } else {
            int posLine = formerAns.at(formerAns.size() - 1 - tokenPointer).second;
            while (formerAns.at(formerAns.size() - 1 - tokenPointer).second == posLine) {
                getToken();
                if (Token == ASSIGN) {
                    retractToToken(pos);
                    return VARIABLE_DESCRIPTION_WITH_INITIAL;
                }
            }
            retractToToken(pos);
            return VARIABLE_DESCRIPTION_WITHOUT_INITIAL;
        }
    } else {
        return -1;
    }
    return -1;//impossible to reach
}

bool isStatement() {
    if (Token == FORTK || Token == WHILETK || Token == IFTK
        || Token == SCANFTK || Token == PRINTFTK || Token == SWITCHTK
        || Token == SEMICN || Token == RETURNTK || Token == LBRACE || Token == IDENFR)
        return true;
    return false;
}

int statementsWithSemi(function &fn) {
    int judge = judgeSection();
    if (judge == FUNCTION_CALL_STATEMENT_WITH_RETURN) {
        functionCallStatementWithReturn(fn);
        return 1;
    } else if (judge == FUNCTION_CALL_STATEMENT_WITHOUT_RETURN) {
        functionCallStatementWithoutReturn(fn);
        return 1;
    } else if (judge == READ_STATEMENT) {
        readStatement(0, fn);
        return 1;
    } else if (judge == WRITE_STATEMENT) {
        printStatement(0, fn);
        return 1;
    } else if (judge == RETURN_STATEMENT) {
        returnStatement(fn);
        return 1;
    } else if (judge == ASSIGNMENT_STATEMENT) {
        assignmentStatement(0, fn);
        return 1;
    }
    return 0;
}

int statementsWithoutSemi(function &fn) {
    int judge = judgeSection();
    if (judge == LOOP_STATEMENT) {
        loopStatement(0, fn);
        return 1;
    } else if (judge == CONDITION_STATEMENT) {
        conditionStatement(0, fn);
        return 1;
    } else if (judge == SWITCH_STATEMENT) {
        switchStatement(fn);
        return 1;
    }
    return 0;
}

int isCharacter() {
    return (Token == CHARCON) ? TRUE : FALSE;
}

bool isCharExp(function &fn, string &vn) {
    int pos = int(formerAns.size() - 1 - tokenPointer);
    if (isCharacter()) {//若为字符字面量
        getToken();
        if (!isOperation()) {
            vn = "\'" + formerAns.at(formerAns.size() - 2 - tokenPointer).first + "\'";
            retractTokens(1);
            return true;
        }
    } else if (Token == IDENFR) {
        if (functions.contains(formerAns.at(pos).first)) {
            if (functions.getToken(formerAns.at(pos).first).isEqualType(CHARTK)) {//若为char型有返回值的函数
                getToken();
                findRparent(1);
                getToken();
                if (!isOperation()) {
                    retractToToken(pos);
                    function fn2 = functions.getToken(formerAns.at(pos).first);
                    functionCallStatementWithReturn(fn2);
                    vn = "RET_" + fn2.getName();
                    // retractTokens(1);
                    return true;
                }
            }
        } else {
            string idName = formerAns.at(formerAns.size() - 1 - tokenPointer).first;
            int fnContains = fn.contains(idName);
            int gtContains = globalTable.contains(idName);
            enum tokenCategory ty;
            if (fnContains)
                ty = fn.getToken(idName).getTokenCategory();
            else if (gtContains)
                ty = globalTable.getToken(idName).getTokenCategory();
            if (fnContains || gtContains) {
                if (ty == CHARTK) {
                    getToken();
                    if (isOperation()) {
                        retractToToken(pos);
                        return false;
                    }
                    if (Token == LBRACK) {//char型一维数组
                        getToken();
                        string vt1;
                        expression(fn, vt1);
                        getToken();//RBRACK
                        getToken();
                        if (!isOperation() && Token != LBRACK) {
                            retractTokens(1);
                            vn = idName + "[" + vt1 + "]";
                            return true;
                        }
                        if (Token == LBRACK) {//char型二维数组
                            getToken();
                            string vt2;
                            expression(fn, vt2);
                            getToken();//RBRACK
                            getToken();
                            if (!isOperation()) {
                                retractTokens(1);
                                vn = idName + "[" + vt1 + "]" + "[" + vt2 + "]";
                                return true;
                            }
                        }
                    } else {//char型标识符
                        retractTokens(1);
                        vn = idName;
                        return true;
                    }
                }
            }

        }
    }
    retractToToken(pos);
    return false;
}

bool expressTypeEqual(int expressType, enum tokenCategory tc) {
    if (expressType == 2)
        return tc == CHARTK;
    else
        return tc == INTTK;
}

void retractTokens(int numOfTokens) {
    while (numOfTokens--) {
        tokenPointer++;
        Token = formerTokens.at(formerTokens.size() - 1 - tokenPointer);
        outputAns.pop_back();
    }
}

void retractToToken(int index) {
    int difference = int(formerTokens.size() - 1 - tokenPointer - index);
    while (difference--) {
        tokenPointer++;
        Token = formerTokens.at(formerTokens.size() - 1 - tokenPointer);
        outputAns.pop_back();
    }
}

void getTokens(int numOfTokens) {
    while (numOfTokens--)
        getToken();
}

int isOperation() {
    if (Token == PLUS || Token == MINU || Token == MULT || Token == DIV)
        return 1;
    return 0;
}