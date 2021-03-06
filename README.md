# BUAA_Compiler2020
2020北航编译技术实验部分个人作业

[文法定义](#文法定义)

[词法分析设计文档](#词法分析设计文档)

[语法分析设计文档](#语法分析设计文档)

[错误处理与符号表管理](#错误处理与符号表管理)

[生成目标代码](#生成目标代码)

[优化](#优化)


## 文法定义

```c++
＜加法运算符＞ ::= +｜-         /*测试程序需出现2种符号*/
＜乘法运算符＞  ::= *｜/         /*测试程序需出现2种符号*/
＜关系运算符＞  ::=  <｜<=｜>｜>=｜!=｜==    /*测试程序需出现6种符号*/
＜字母＞   ::= ＿｜a｜．．．｜z｜A｜．．．｜Z   /*测试程序需出现下划线、小写字母、大写字母3种情况*/    
＜数字＞   ::= ０｜１｜．．．｜９                        /*测试程序至少出现1次数字*/
＜字符＞    ::=  '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞'   /*测试程序至少出现4种类型的字符*/
＜字符串＞   ::=  "｛十进制编码为32,33,35-126的ASCII字符｝" //字符串中要求至少有一个字符
		  /*测试程序至少出现1次字符串，不必覆盖所有字符*/
＜程序＞    ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞   
	/*测试程序只需出现有/无常量说明、有/无变量说明、有/无函数定义的情况，不必考虑其排列组合*/
＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}    /*测试程序需出现一个const语句、2个或2个以上const语句2种情况*/
＜常量定义＞   ::=   int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}| char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}   
	/*测试程序需出现int和char 2种类型的常量，需出现1个常量定义、2个或2个以上常量定义2种情况，不必考虑排列组合*/
＜无符号整数＞  ::= ＜数字＞｛＜数字＞｝  /*测试程序需出现1位整数、2位及2位以上整数2种情况*/
＜整数＞        ::= ［＋｜－］＜无符号整数＞  /*测试程序需出现不带+/-号的整数、带+和-号的整数*/
＜标识符＞    ::=  ＜字母＞｛＜字母＞｜＜数字＞｝  
               //标识符和保留字都不区分大小写，比如if和IF均为保留字，不允许出现与保留字相同的标识符
              /*测试程序需出现只有1个字母的标识符，有后续字母或数字2种情况*/
＜声明头部＞   ::=  int＜标识符＞ |char＜标识符＞   /*测试程序需出现int 和 char2种类型的声明头部*/
＜常量＞   ::=  ＜整数＞|＜字符＞    /*测试程序需出现整数和字符2种情况的常量*/
＜变量说明＞  ::= ＜变量定义＞;{＜变量定义＞;}   /*测试程序需出现只有1项变量定义、2项及2项以上变量定义2种情况*/
＜变量定义＞ ::= ＜变量定义无初始化＞|＜变量定义及初始化＞  /*测试程序需出现变量定义无初始化和有初始化2种情况*/
＜变量定义无初始化＞  ::= ＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']')
	{,(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']' )}
            //变量包括简单变量、一维、二维数组，＜无符号整数＞表示数组各维元素的个数，其值需大于0，数组元素按行优先存储
            //变量没有初始化的情况下没有初值 
            /*测试程序需出现一维数组定义、二维数组定义2种情况；需出现一个类型标识符后有1项、2项及2项以上变量2种情况，不必考虑排列组合*/
＜变量定义及初始化＞  ::= ＜类型标识符＞＜标识符＞=＜常量＞|＜类型标识符＞＜标识符＞'['＜无符号整数＞']'='{'＜常量＞{,＜常量＞}'}'
	|＜类型标识符＞＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']'='{''{'＜常量＞{,＜常量＞}'}'{, '{'＜常量＞{,＜常量＞}'}'}'}'
         //简单变量、一维、二维数组均可在声明的时候赋初值，＜无符号整数＞表示数组各维元素的个数，其值需大于0，数组元素按行优先存储，
	//＜常量＞的类型应与＜类型标识符＞完全一致，否则报错；每维初值的个数与该维元素个数一致，否则报错，无缺省值； 
        /*测试程序需出现简单变量初始化、一维数组初始化、二维数组初始化3种情况*/  
＜类型标识符＞      ::=  int | char  /*测试程序需出现int、char 2种类型标识符*/
＜有返回值函数定义＞  ::=  ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'   /*测试程序需出现有返回值的函数定义*/
＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'   /*测试程序需出现无返回值的函数定义*/
＜复合语句＞   ::=  ［＜常量说明＞］［＜变量说明＞］＜语句列＞                
	/*测试程序的复合语句需出现有和无常量说明2种情况、有和无变量说明2种情况，不必考虑其排列组合*/
＜参数表＞    ::=  ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞  
	/*测试程序需出现无参数、1个参数、2个及2个以上参数3种情况*/
＜主函数＞    ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’       		/*每个测试程序有且仅有1个主函数*/
＜表达式＞    ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}   //[+|-]只作用于第一个<项>  
              /*测试程序的表达式需出现项之前无符号、有+和-号的情况；表达式只有1个项、2个及2个以上项2种情况，不必考虑排列组合*/
＜项＞     ::= ＜因子＞{＜乘法运算符＞＜因子＞}     /*测试程序的项需出现只有1个因子、2个及2个以上因子2种情况*/
＜因子＞    ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|＜标识符＞'['＜表达式＞']''['＜表达式＞']'|'('＜表达式＞')'
	｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞         
                //char 类型的变量或常量，用字符的ASCII 码对应的整数参加运算
                //＜标识符＞'['＜表达式＞']'和＜标识符＞'['＜表达式＞']''['＜表达式＞']'中的＜表达式＞只能是整型，下标从0开始
                //单个＜标识符＞不包括数组名，即数组不能整体参加运算，数组元素可以参加运算
               /*测试程序的因子需出现7种情况*/
＜语句＞    ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞;|＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜
	＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}'          /*测试程序需出现11种语句*/
＜赋值语句＞   ::=  ＜标识符＞＝＜表达式＞|＜标识符＞'['＜表达式＞']'=＜表达式＞
		|＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞
                 //＜标识符＞＝＜表达式＞中的<标识符>不能为常量名和数组名
                /*测试程序需出现给简单变量赋值、一维数组元素赋值、二维数组元素赋值3种情况*/
＜条件语句＞  ::= if '('＜条件＞')'＜语句＞［else＜语句＞］   /*测试程序需出现有else和无else 2种形式的条件语句*/
＜条件＞    ::=  ＜表达式＞＜关系运算符＞＜表达式＞           
                  //表达式需均为整数类型才能进行比较
                 /*测试程序中需出现条件*/
＜循环语句＞   ::=  while '('＜条件＞')'＜语句＞
		| for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞     
                 //for语句先进行条件判断，符合条件再进入循环体
               /*测试程序中需出现while和for 2种循环语句，for语句应出现+步长和-步长2种情况*/
＜步长＞::= ＜无符号整数＞  
＜情况语句＞  ::=  switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞‘}’     /*测试程序需出现情况语句*/
＜情况表＞   ::=  ＜情况子语句＞{＜情况子语句＞}      /*测试程序需出现只有1个情况子语句、2个及2个以上情况子语句2种情况*/
＜情况子语句＞  ::=  case＜常量＞：＜语句＞                            /*测试程序中需出现情况子语句*/
＜缺省＞   ::=  default :＜语句＞                                                /*测试程序中需出现缺省语句*/
       //情况语句中，switch后面的表达式和case后面的常量只允许出现int和char类型；
       //每个情况子语句执行完毕后，不继续执行后面的情况子语句
＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'         /*测试程序需出现有返回值的函数调用语句*/
＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'         /*测试程序需出现无返回值的函数调用语句*/
	//函数调用时，只能调用在之前已经定义过的函数，对是否有返回值的函数都是如此
＜值参数表＞   ::= ＜表达式＞{,＜表达式＞}｜＜空＞                   
                //实参的表达式不能是数组名，可以是数组元素
                //实参的计算顺序,要求生成的目标码运行结果与Clang8.0.1 编译器运行的结果一致
               /*测试程序中需出现无实参、1个实参、2个及2个以上实参3种情况*/
＜语句列＞   ::= ｛＜语句＞｝ /*测试程序的语句列需出现无语句、有语句2种情况*/
＜读语句＞    ::=  scanf '('＜标识符＞')' 
               //从标准输入获取<标识符>的值，该标识符不能是常量名和数组名
               //生成的PCODE或MIPS汇编在运行时，对每一个scanf语句，无论标识符的类型是char还是int，末尾均需回车；
	       //在testin.txt文件中的输入数据也是每项在一行
              //生成MIPS汇编时按照syscall指令的用法使用
             /*测试程序中需出现读语句*/
＜写语句＞    ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')' 
               //printf '(' ＜字符串＞,＜表达式＞ ')'输出时，先输出字符串的内容，再输出表达式的值，两者之间无空格
              //表达式为字符型时，输出字符；为整型时输出整数
              //＜字符串＞原样输出（不存在转义）
              //每个printf语句的内容输出到一行，按结尾有换行符\n处理
             /*测试程序中需出现3种形式的写语句*/
＜返回语句＞   ::=  return['('＜表达式＞')']   
              //无返回值的函数中可以没有return语句，也可以有形如return;的语句
             //有返回值的函数只要出现一条带返回值的return语句（表达式带小括号）即可，不用检查每个分支是否有带返回值的return语句
             /*测试程序中需出现有返回语句和无返回语句2种情况，有返回语句时，需出现有表达式和无表达式2种情况*/                           
```

另：关于类型和类型转换的约定：

1. 表达式类型为char型有以下三种情况：

       1）表达式由<标识符>、＜标识符＞'['＜表达式＞']和＜标识符＞'['＜表达式＞']''['＜表达式＞']'构成，
       且<标识符>的类型为char，即char类型的常量和变量、char类型的一维、二维数组元素。
       
       2）表达式仅由一个<字符>构成，即字符字面量。
       
       3）表达式仅由一个有返回值的函数调用构成，且该被调用的函数返回值为char型

  除此之外的所有情况，<表达式>的类型都是int

2. 只在表达式计算中有类型转换，字符型一旦参与运算则转换成整型，包括小括号括起来的字符型，也算参与了运算，例如(‘c’)的结果是整型。
3. 其他情况，例如赋值、函数传参、if/while条件语句中关系比较要求类型完全匹配，并且＜条件＞中的关系比较只能是整型之间比，不能是字符型。



## 词法分析设计文档

### 编码前设计

编码前，老师要求我们完成了《编译技术》书第73页的第三题。第71到72页有对此题十分详细的顶层代码解答。总的来说，程序的逻辑是读入字符后分情况讨论，输出一个类别符`symbol`作为返回值。本题亦是如此，只是类型变得很多，同时对输出有一定要求，大致逻辑是不变的。我首先定义了一个`enum`类型并将`symbol`作为此类型的变量，这样可以直接用`symbol`作为下标输出一个存有类型值的字符串数组。然后再声明各种可能的符号判别函数，并在`getsym`函数中使用，最后一一完成各函数的定义。主函数中循环使用`getsym`函数直到读取到EOF。

```c++
#define TRUE        1
#define FALSE       0
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;
enum category
{
	IDENFR = 0, INTCON, CHARCON, STRCON, CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK,
	ELSETK, SWITCHTK, CASETK, DEFAULTTK, WHILETK, FORTK, SCANFTK, PRINTFTK, RETURNTK, PLUS,
	MINU, MULT, DIV, LSS, LEQ, GRE, GEQ, EQL, NEQ, COLON,
	ASSIGN, SEMICN, COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE
} symbol;
char categoryString[39][10]
= { "IDENFR","INTCON","CHARCON","STRCON","CONSTTK","INTTK","CHARTK","VOIDTK","MAINTK","IFTK",
"ELSETK", "SWITCHTK", "CASETK", "DEFAULTTK","WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK","PLUS",
"MINU","MULT","DIV","LSS","LEQ","GRE","GEQ","EQL","NEQ","COLON",
"ASSIGN","SEMICN","COMMA","LPARENT","RPARENT","LBRACK","RBRACK","LBRACE","RBRACE"
};
char reserves[15][10]
= { "const","int","char","void","main","if","else","switch","case",
	"default","while","for","scanf","printf","return"
};
int  num;
char Char;
char lastChar;
char tokenToLower[128];
char token[128];
int  pointer;
int  line;
string ans;
int getChar(FILE* stream);
void clearToken();
int isSpace();
int isLetter(), isDigit();
int isColon(), isComma(), isSemi(), isQuote(), isApostrophe();
int isEqu(), isGre(), isLss(), isExcla();
int isPlus(), isMinus(), isDivi(), isMult();
int isLpar(), isRpar(), isLbrack(), isRbrack(), isLbrace(), isRbrace();
void catToken();
void retract();
int reserver(), transNum(), isEof();
void error();
int getsym(FILE* stream)；
```

### 编码后设计

在VS19编程环境下发现，使用`fopen`会提醒如下错误：

```c
'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.	LexicalAnalysis	lex.cpp	390	
```

虽然后来在群上同学的提醒下使用`#define _CRT_SECURE_NO_WARNINGS`会避免报错，然而我首先尝试改用C++完成代码。在一个下午的速成学习中，我逐渐学会使用输入输出流来打印字符，但是还是觉得纯粹用`fstream`输入对字符串的控制不够精细，尤其是如果想要通过空格来判断字符串是否结束是十分困难的，输入流会直接吞掉空字符。所以我改回使用`fgetc()`来读字符，用`ofstream`来输出，十分方便。在调试过程中，逐渐习惯了VS19的操作，同时发现了很多细节上的bug，尤其是对词法理解上的问题，不过我的代码比较冗长，因为想使用较为工整的格式，对于很多地方写的不够简洁，足足有四百余行。不过我为报错提供了接口，方便后续添加，同时代码较为易读，后续会延续这种风格。

## 语法分析设计文档

### 编码前设计

首先，由于使用递归子程序法编写语法分析程序，所以必须对所有非终结符编写分析函数（需要输出的部分必须编写，不需要的可以直接判断`Token`成分）：

```C++
int String(int optional);//字符串
int program();//程序
int constDescription(int optional);//常量说明
int constDefine();//常量定义
int unsignedInteger(bool output,int optional);//无符号整数
int integer(int optional);//整数
int statementHead();//声明头部
int Const();//常量
int variableDescription(int optional);//变量说明
int variableDefine(int optional);//变量定义
int variableDefineWithoutInitial();//变量定义无初始化
int variableDefineWithInitial();//变量定义及初始化
int functionDefineWithReturn(int optional);//有返回值函数定义
int functionDefineWithoutReturn(int optional);//无返回值函数定义
int compoundStatement();//复合语句
int parametersTable();//参数表
int mainFunction();//主函数
int expression();//表达式
int term();//项
int factor();//因子
int statement(int optional);//语句
int assignmentStatement(int optional);//赋值语句
int conditionStatement(int optional);//条件语句
int condition();//条件
int loopStatement(int optional);//循环语句
int stride();//步长
int switchStatement(int optional);//情况语句
int caseTable();//情况表
int caseStatement(int optional);//情况子语句
int defaultStatement();//缺省
int functionCallStatementWithReturn(int optional);//有返回值函数调用语句
int functionCallStatementWithoutReturn(int optional);//无返回值函数调用语句
int valueParameterTable();//值参数表
int statementList();//语句列
int readStatement(int optional);//读语句
int printStatement(int optional);//写语句
int returnStatement(int optional);//返回语句
```

同时确立三个原则以方便编码：

1. 只要是可选的，必须有空串的判断（也即传入一个参数，若是可选的，则允许为空；若是必选的，则不允许为空）。`optional`为1说明可选，若为0说明必选。可选也即代表在某个位置可以有此部分，也可以没有，例如：
   ＜程序＞  ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞）
   那么常量说明、变量说明、有返回值函数定义和无返回值函数定义都是可选的。

2. 所有函数调用前必须首先`getToken()`。

3. 所有函数结束调用时不要`getToken()`。

为了方便后续加入异常处理和符号号管理的内容，我会对可能出现异常的部分都添加处理异常的接口。对于输出，我选择将准备输出的字符串压入`vector<string> outputAns`中方便修改。

### 编码后设计

编码过程中和debug时都遇到了很多问题，主要是加入了异常处理的情况使得问题变得极为复杂。尤其对于可选情况，如果直接进入递归那么如果不满足就会报错，而且就算在使用`option`的情况下，不满足也不会报错（仅返回0），但是如果每一种情况都遍历一次复杂度相当高，而且往往会出现诸多bug难以调试，因此添加了专门用于判断该走入哪种情况的函数，原理相同，就是预先读入`Token`，判断符合哪种条件就进入该递归子程序（预读后需要退回对应的词）。

在编码中还采用了一种十分重要的手段，就是将读入的`Token`和其对应的testfile中的字符压入栈中，在需要进行回退时改变栈顶指针，这样就可以进行预读，并且不会在第一次读入`Token`时就自动输出，可以保证输出顺序的正确性。

同时，对于函数的调用，需要存储函数对应的类型，因此建立了函数符号表方便查找。对于数组各维的大小对应也可以使用输入栈来判断赋值或初始化是否异常。

```C++
extern vector<enum tokenCategory> formerTokens;//token栈
extern vector<string> formerAns;//输入栈
extern vector<string> outputAns;//输出栈
extern map<string, enum tokenCategory> functionTable;//函数符号表
```

最后对于代码的风格，我将每个`.cpp`文件的函数、全局变量声明放入了对应的头文件中，再在`.cpp`源文件中定义，同时定义了大量的宏以便于阅读。

## 错误处理与符号表管理

### 编码前设计

对于符号表，增加以下类：

Token类：

1. 有两个类变量：名字、类型。
2. 可以比较类型是否相等。

标识符类：

1. 继承自Token类。
2. 有每一维的维数信息，若不为数组则两个维度的值为0。
3. 有一个变量记录是`const`、`var`还是`arg`。

函数类：

1. 继承自Token类。
2. 有一个符号表。
3. 有一个参数栈。

标识符符号表：

1. 每读入一个函数，则创建一个符号表。对于一个函数内部，采用栈式符号表，保存函数内（或全局）定义的所有的常量、变量，函数中的符号表应当保存其后定义的。

函数符号表：

1. 程序只有一个函数符号表。

在分析器中：保存一个函数符号表，将所有函数（包括main）都存放进去，同时保存一个全局变量或常量符号表。

程序中符号表的结构：

```mermaid
graph TD
    analy(分析器) == 拥有 ==> fs(函数符号表)
    analy== 拥有 ==>tm(全局标识符符号表)
    fs == 拥有 ==> f1(函数1)
    fs == 拥有 ==> ff(...)
    fs == 拥有 ==> fn(main函数)
    f1== 拥有 ==>t1(标识符符号表1)
    t1== 拥有 ==>tk1(形参*)
    t1== 拥有 ==>tk2(常量*)
    t1== 拥有 ==>tk3(变量*)
    tm== 拥有 ==>tkm1(形参*)
    tm== 拥有 ==>tkm2(常量*)
    tm== 拥有 ==>tkm3(变量*)
    ff== 拥有 ==> tkf(...)
    fn== 拥有 ==> tkn(...)
```

一个可能的程序：

```c
const int c1=1;//常量说明
int v1;//无初始化变量定义
int v2=1;//初始化变量定义
int f1(int arg){//有返回值函数定义
    const int c2=1;//常量说明
    int v3;//变量说明
    int v4=1;//变量说明
    for(...){
        ...
    }
    while(...){
        ...
    }
}
void main(){
    const int c2=1;//常量说明
    int v3;//变量说明
    int v4=1;//变量说明
    for(...){
        ...
    }
    while(...){
        ...
    }
}
```

可以看出：能定义常量或变量的地方只有程序开始处、定义函数开始处和主函数开始处，因此无需“层数”这一信息，只需要考虑该变量或常量是否属于某一函数或者是全局变量即可。

也即，在函数定义时检查是否重复定义，在函数复合语句的部分检查是否使用未定义的符号。

报错时传入错误类型给error函数，错误行数则直接使用错误的token所在的行，此时需要改进上次的输入栈，当存入某个token时应同时存入其所在的行数。

同时在答疑中助教表明不会出现函数未定义的情况，因此可以不将后定义的函数存入先定义的函数的符号表中。

下面的代码为各种报错类型：

```c++
//a:非法符号或不符合词法
//b:名字重定义
//c:未定义的名字
if (!fn.contains(formerAns.at(formerAns.size() - 1 - tokenPointer).first)) {//IDENFR
        error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " c");
}
//函数参数个数不匹配
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " d");
//函数参数类型不匹配
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " e");
//条件判断中出现不合法的类型
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " f");
//无返回值的函数存在不匹配的return语句
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " g");
//有返回值的函数缺少return语句或存在不匹配的return语句
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " h");
//数组元素的下标只能是整型表达式
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " i");
//不能改变常量的值
error_syntax(to_string(formerAns.at(formerAns.size() - 2 - tokenPointer).second) + " j");
//应为分号
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " k");
//应为右小括号’)’
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " l");
//应为右中括号’]’
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " m");
//数组初始化个数不匹配
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " n");
//<常量>类型不一致
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " o");
//缺少缺省语句
error_syntax(to_string(formerAns.at(formerAns.size() - 1 - tokenPointer).second) + " p");
```

### 编码后设计

最终将所有的函数定义放在了头文件`compiler.h`中，并分别在各自的`.cpp`文件中加以实现。

```c++
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
```

需要解决的一个困难是如何判断一个表达式的类型是`int`还是`char`，文法说明中提出了三点：

1. 表达式由<标识符>、＜标识符＞'['＜表达式＞']和＜标识符＞'['＜表达式＞']''['＜表达式＞']'构成，且<标识符>的类型为char，即char类型的常量和变量、char类型的一维、二维数组元素。
2. 表达式仅由一个<字符>构成，即字符字面量。
3. 表达式仅由一个有返回值的函数调用构成，且该被调用的函数返回值为char型

除此之外的所有情况，<表达式>的类型都是int。再看看表达式及其组成的范式：

```c
＜表达式＞::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}   //[+|-]只作用于第一个<项>  
＜项＞::= ＜因子＞{＜乘法运算符＞＜因子＞} 
＜因子＞::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|＜标识符＞'['＜表达式＞']''['＜表达式＞']'|'('＜表达式＞')'｜＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
```

所以一个可行的方法是，直接对这三种情况进行特判，然后把类型作为返回值。

在编码完后还发现了数不胜数的bug，其中最有意义的在于：C++函数调用是传值，也即复制参数而并非像Java一样传入参数的引用（的复制），因此如果不手动传引用或指针作为形参，那么经过函数后将不会对参数产生改变。

## 生成目标代码

### 编码前设计

#### 中间代码生成

生成四元式语法规则：

```cpp
int foo( int a, int b, int c, int d)
/*
int foo()
@para int a
@para int b
@para int c
@para int d
*/
```

```cpp
i = tar(x,y)
/*
@call_start
@push x
@push y
@call tar
i = RET
*/
```

```cpp
return (x)
/*
return x
*/
```

```cpp
int i, j;//作业一
/*
在中间代码阶段，无需显式地声明变量
在MIPS汇编代码阶段：
对于非全局变量，在使用直接分配寄存器（或内存栈）即可
对于全局变量，在程序开头分配内存，在使用时对寄存器进行赋值最后存入内存
*/
```

```cpp
const int c = 10//作业一
/*
无必要，在使用时直接调用即可（存放在符号表中）
*/
```

```cpp
k=+r/10000*i-a*(b+j[1-1*1][1])+3-'c';//作业一
/*
$t0 = r / 10000
$t1 = $t0 * i
$t2 = + $t1
$t3 =  b
$t4 =  1
$t5 = 1 * 1
$t6 = $t4 - $t5
$t7 =  1
$t8 = $t3 + j[$t6][$t7]
$t9 = a * $t8
$t10 = $t2 - $t9
$t11 = $t10 + 3
$t12 = $t11 - 'c'
k = $t12
为避免字符名字与临时变量名重回，临时变量命名规则为$ti
*/
```

```cpp
x == y
/*
x == y
*/
```

```cpp
if($t1>$t2){
    ...
} else{
    ...
}
/*
@bge $t2 $t1 Label_condition_end1
 ...
Label_condition_end1:
 else中的内容
*/
if($t1<$t2){
    ...
}
/*
@bge $t1 $t2 Label_condition_end1
*/
if($t1==$t2){
    ...
}
/*
@bne $t1 $t2 Label_condition_end1
*/
if($t1!=$t2){
    ...
}
/*
@beq $t1 $t2 Label_condition_end1
*/
if($t1>=$t2){
    ...
}
/*
@blt $t1 $t2 Label_condition_end1
*/
if($t1<=$t2){
    ...
}
/*
@blt $t2 $t1 Label_condition_end1
*/
/*
Label_condition_end1:
*/
```

```cpp
for(identifier=expression;condition;identifier=identifier(+|-)stride){
    ...
}
/*
令identifier为i，令stride为$t0，令初始条件的右值为$s0
Label_for_start1:
i = $s0
比较语句（同上）-> Label_for_end1:
Label_for_entry1:
循环体
i=i(+ | -)$t0
比较语句（同上）-> Label_for_entry1:
Label_for_end1:
*/
```

```cpp
while(condition){
    ...
}
/*
Label_while_start1:
比较语句（同上）-> Label_while_end1:
Label_while_entry1:
循环体
比较语句（同上）-> Label_while_entry1:
Label_while_end1 :
*/
```

```cpp
switch(expression){
    case Const1:
        statement;
    case Const2:
        statement;
    ...
    default:
        statement;
}
/*
 * 令判断的变量为$t0
Label_switch_start1 :
expression
@bne $t0 const1 Label_switch1_case1_end
语句块
@j Label_switch_end1
Label_switch1_case1_end:
@bne $t0 const2 Label_switch1_case2_end
语句块
@j Label_switch_end1
Label_switch1_case2_end:
default语句块
Label_switch_end1:
*/
```

```cpp
printf(String/expression/String,expression)//作业一
/*
@print tn
*/
```

```cpp
scanf(identifier)//作业一
/*
@scan identifier
*/
```

中间代码生成应在语法分析的同时进行。

#### 目标代码生成

##### 主要逻辑

在程序开始时，输出`.data`并给全局变量分配空间，同时用`.asciiz`初始化每个字符串。

输出`.text`。

在中间代码中每读到一个函数名则在函数表中寻找此函数，当使用到全局变量、局部变量或临时变量时分配寄存器，若寄存器不够，则压栈。在函数结尾全局变量使用后存入内存，。编译器中需要记录哪些寄存器可以销毁（在不使用优化时，同一个函数的使用的寄存器都不用销毁）。当读到常量或未被改变值的初始化变量时，可直接用数值填入汇编代码。

##### 内存分配

首先，全局变量和字符串都会分配于静态区（static）中（从.data开始的地址）。

当寄存器分配不够时压入栈中，对于数组，将其视为维数多个单独的普通变量，改变其值时使用寄存器或内存，全局数组变量存于相应的空间中。

函数调用时使用`$sp`寄存器和栈（stack）空间。多于四个入口参数将从栈中`$sp`寄存器位置取出，也即压栈时顺序为：
$$
$ra寄存器压栈\rightarrow 各临时变量寄存器压栈\rightarrow 入口参数寄存器压栈\rightarrow 入口参数寄存器赋值\rightarrow四个之后的入口参数压栈
$$

其中相对于栈的位移，规定先存的位于更低的地址（例如：`sw $ra 0($sp)`），此后依次累加。

<img src="img\空间分配.png" alt="空间分配" style="zoom:60%;" />

上图为内存区域分配示意图，下图为编译器中对于内存分配的操作方式。将变量压栈后可通过变量与其相对`$sp`寄存器的偏移和偏移与寄存器的关系恢复变量与寄存器的关系从而实现从栈中恢复变量值的操作：

<img src="img\内存读取示意图.jpg" style="zoom: 20%;" />

图中的数据结构都应是函数类型的属性，因为无论是寄存器还是栈的分配都不会跨越函数。

注意：存入堆时从`.data`起点开始，存入栈时先减去偏移再从偏移为0开始存入，因此`$sp`初始位置是不存值的。

##### 函数声明及调用

函数调用的步骤：

1. 将需要保存的值压入栈（入口参数为表达式时产生的中间变量都可以不压栈）
2. 如果需要的话，指定入口参数
3. jal 调用
4. 从栈中恢复相关的值

调用过程中的规则：

1. 通过 jal 指令调用，使用一个 jr $ra 指令返回
2. 4个入口参数 ：$a0，\$a1，\$a2， \$a3 (多于四个存在栈里)
3. 返回值通常在 \$v0 中（如果需要，可以使用\$v1）
4. 必须遵守寄存器使用规范（即使是在那些只有你自己调用的函数中!）

```c
int sumSquare(int x, int y){
	return mult(x,x)+ y;
}
```

```assembly
sumSquare: 
#push
 addi 	$sp,	$sp,	-8 	# space on stack
 sw 	$ra, 	0($sp) 	# save ret addr
 sw 	$a1, 	4($sp) 	# save y
 
 add 	$a1,	$a0,	$zero 	# prepare args: 2nd x 
 jal 	mult 	# call mult

#pop
 lw 	$a1, 	4($sp) 	# restore y
 add 	$v0,	$v0,	$a1 	# mult()+y 
 lw 	$ra, 	0($sp) 	# get ret addr
 addi 	$sp,	$sp,	8 	# restore stack
 jr 	$ra
mult:
```

##### 数组处理

全局变量在程序开头开对应大小的空间于内存，局部变量在函数开头开对应大小的空间于栈。存储全局变量时较小下标的元素占据较低的堆内存，存储局部变量时较小下标的元素占据较高位置的栈内存。

##### 分配栈空间

分配栈空间只可能出现在一个函数内，每个函数结束后栈空间应该清空。函数开始时首先将初始化后的数组存在栈里。当寄存器不够用时，要分配栈空间只可能分给局部变量、零时变量，若此局部变量是一个未初始化的数组的某一个元素，应将其视为一个普通的局部变量来处理。

##### 同名变量处理

方案一：重命名→需要在`syntaxAnalysis`中对所有的原变量改变赋值，或在function中做一个hash表相互映射。

##### 参考内容

MIPS寄存器：

| REGISTER | NAME     | USAGE                    |
| -------- | -------- | ------------------------ |
| $0       | $zero    | 常量0                    |
| $1       | $at      | 保留给汇编器             |
| \$2-$3   | \$v0-$v1 | 函数调用返回值           |
| \$4-$7   | \$a0-$a3 | 函数调用参数             |
| \$8-$15  | \$t0-$t7 | 临时寄存器               |
| \$16-$23 | \$s0-$s7 | 全局寄存器               |
| \$24-$25 | \$t8-$t9 | 临时寄存器               |
| $28      | $gp      | 全局指针(Global Pointer) |
| $29      | $sp      | 堆栈指针(Stack Pointer)  |
| $30      | $fp      | 帧指针(Frame Pointer)    |
| $31      | $ra      | 返回地址(return address) |

MIPS内存地址：

选择`Default`。

| 区域                                    | 地址       |
| --------------------------------------- | ---------- |
| .text base address                      | 0x00400000 |
| .data base address                      | 0x10010000 |
| $sp（并不指向栈顶，应该是MARS设计如此） | 0x7fffeffc |

MIPS指令：

| 含义                                 | 指令    | 类型 |
| ------------------------------------ | ------- | ---- |
| 符号加                               | add     | R    |
| 符号加立即数                         | addi    | I    |
| 无符号加立即数                       | addiu   | I    |
| 无符号加                             | addu    | R    |
| 符号减                               | sub     | R    |
| 无符号减                             | subu    | R    |
| 与                                   | and     | R    |
| 与立即数                             | andi    | I    |
| 或非                                 | nor     | R    |
| 或                                   | or      | R    |
| 或立即数                             | ori     | I    |
| 异或                                 | xor     | R    |
| 异或立即数                           | xori    | I    |
| 逻辑左移                             | sll     | R    |
| 逻辑右移                             | srl     | R    |
| 算术右移                             | sra     | R    |
| 逻辑可变左移                         | sllv    | R    |
| 逻辑可变右移                         | srlv    | R    |
| 算术可变右移                         | srav    | R    |
| 小于置 1(有符号)                     | slt     | R    |
| 小于立即数置 1(有符号)               | slti    | I    |
| 小于立即数置 1(无符号)               | sltiu   | I    |
| 小于置 1(无符号)                     | sltu    | R    |
| 相等时转移（1条alu+1条跳转/1条跳转） | beq     | I    |
| 不等于时转移（1+1/1）                | bne     | I    |
| 小于时跳转（1+1/1+1）                | blt     | R    |
| 大于时跳转（2+1/1+1）                | bgt     | R    |
| 小于 0 时转移（2+1/1+1）             | ble     | R    |
| 小于等于0时转移                      | blez    | I    |
| 大于等于时转移（1+1/1+1）            | bge     | R    |
| 大于等于0时转移                      | bgez    | I    |
| 跳转                                 | j       | J    |
| 跳转并链接(推荐使用)                 | jal     | J    |
| 跳转至寄存器                         | jr      | R    |
| 跳转并链接                           | jalr    | R    |
| Ｍove                                | move    | R    |
| 读 CP0 寄存器                        | mfc0    | R    |
| 读 HI 寄存器                         | mfho    | R    |
| 读 LO 寄存器                         | mflo    | R    |
| 写 CP0 寄存器                        | mtc0    | R    |
| 写 HI 寄存器                         | mthi    | R    |
| 写 LO 寄存器                         | mtlo    | R    |
| 符号乘                               | mult    | R    |
| 无符号乘                             | multu   | R    |
| 符号除                               | div     | R    |
| 无符号除                             | divu    | R    |
| 加载字节                             | lb      | I    |
| 加载无符号字节                       | lbu     | I    |
| 加载半字                             | lh      | I    |
| 加载无符号半字                       | lhu     | I    |
| 立即数加载至高位                     | lui     | I    |
| 加载字                               | lw      | I    |
| 加载立即数                           | li      | I    |
| 加载至地址                           | la      | I    |
| 存储字节                             | sb      | I    |
| 存储半字节                           | sh      | I    |
| 存储字                               | sw      | I    |
| 系统调用                             | syscall |      |
| 异常返回                             | eret    |      |
| 断点                                 | break   |      |

三种类型指令的格式：

| 类型 | 格式                         |
| ---- | ---------------------------- |
| R    | op rd rs rt                  |
| I    | op rt rs im 或 op rt imm(rs) |
| J    | op target                    |

名词解释：

```assembly
.data       #Subsequent items stored in Data segment at next available address
.space      #Reserve the next specified number of bytes in Data segment
.text       #Subsequent items (instructions) stored in Text segment at next available address
.asciiz    	#Store the string in the Data segment and add null terminator
```

Syscall：

```assembly
li  $v0, 1           # service 1 is print integer
add $a0, $t0, $zero  # load desired value into argument register $a0, using pseudo-op
syscall
```

| Service                    | Code in $v0 | Arguments                                        | Result                      |
| -------------------------- | ----------- | ------------------------------------------------ | --------------------------- |
| print integer              | 1           | $a0 = integer to print                           |                             |
| print string               | 4           | $a0 = address of null-terminated string to print |                             |
| read integer               | 5           |                                                  | $v0 contains integer read   |
| exit (terminate execution) | 10          |                                                  |                             |
| print character            | 11          | $a0 = character to print                         |                             |
| read character             | 12          |                                                  | $v0 contains character read |

### 编码后设计

对于中间代码生成，难点在于表达式的处理。可以利用递归下降算法，对于每一个`expression`，`term`，`factor`都传入一个字符串指针作为其四元式左边的符号，在`expression`和`term`中生成新的非终结符传入其子元素：

```c++
 	//以下为expression核心代码
	string op0;
    if (Token == PLUS || Token == MINU) {
        op0 = ...
        getToken();
    }
    string vt0;
    ret -= term(fn, vt0);
    string vt1 =...//新建一个非终结符
    addQuaternion(vector<string>{vt1, "=", op0, vt0});//增加一条四元式
    getToken();
    while (Token == PLUS || Token == MINU) {
        string op = ...
        getToken();
        string vt2;
        term(fn, vt2);
        string vtn...//新建一个非终结符
        addQuaternion(vector<string>{vtn, "=", vt1, op, vt2});//增加一条四元式
        vt1 = vtn;
        getToken();
    }
    vn = vt1;
```

在生成中间代码时，注意非终结符命名不可与变量名重合。

对于生成目标代码的部分，遇到了众多的难点，其中最主要的是寄存器的分配。人为规定使用`$t0`到`$s7`十六个寄存器，并有一指针指向下一个即将被分配的寄存器，寄存器分配遵守如下规则：

1. 若寄存器未满，直接将寄存器分配给变量$b$，若为全局变量，需要将其值从内存中读出存入分配的寄存器。
2. 若寄存器已满，有两个步骤：
   1. 将寄存器头指针处的变量$a$压入栈内，并将该变量从寄存器中清除  
      1. 若栈中未存储过变量$a$，则新开辟一片空间将变量存入并记录在`var2off`（变量与偏移的键值对）中（先`addi $sp $sp -4`再`sw`）
      2. 若栈中存储过变量$a$，则将寄存器中的值存入该地址中(`sw`)
   2. 将寄存器分配给变量$b$
      1. 若栈中没有变量$b$，则直接分配寄存器，若为全局变量，需要将其值从内存中读出存入分配的寄存器。
      2. 若栈中存有变量$b$，则将栈中的值取出存于寄存器中

————————————————————————分割线———————————————————————

之前错的离谱，直接导致👴从12月4号到12月13号累计debug了六七十小时，重构了大部分代码。

总的来说是错在寄存器和栈内存的交换上，这根本不可行！！！

如果存在一个跨越基本块的`$sp`指针变换，那它必错。因为当再次利用原偏移访问变量时，`$sp`指针的值已经变了，那么将无法访问到原地址。

因此，除了一开始给变量分配栈内存空间和函数跳转时需要压栈，在函数内不要改变`$sp`的值。

我首先给所有全局变量在堆中分配空间，同时扫描中间代码，找到在一个函数中使用的所有非数组全局变量，并为其分配寄存器。对于初始化的局部变量和局部变量数组，在函数开始时分配栈空间，对于未初始化的局部变量和临时变量分配寄存器。对于寄存器的分配，不区分全局寄存器和临时寄存器，从`$t0`到`$s6`全部等效使用。对于分配给全局或局部变量的寄存器，不应该替换，对于临时寄存器，则可以扫描后计数，当确定后面不会使用时可以将其替换。

对于栈空间，由于不能改变`$sp`的值，所以可以将新分配的变量存在`$sp`的下面，在函数跳转时再移动`$sp`到这些空间的下面。（所以总的来说，我的编译器并不是严格按照标准来分配寄存器的）。

## 优化

### 循环结构改造

![](img\if.png)

<img src="img\while.png" style="zoom: 33%;" />

将while和for循环改造成上图所示的结构可以在每次循环过程中省去一次跳转。

### 复写传播

扫描中间代码，若左式为临时变量而右式是数字、标识符、其他临时变量等时，将其删除并在后续使用到此变量的代码中用右式替换它。

### 乘除化简

两数乘除法直接计算出结果，一个乘（除）数为2的幂次大小时可以用左移、右移来替代直接的乘除。