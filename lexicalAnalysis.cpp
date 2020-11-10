#include "compiler.h"

char Char;
char lastChar;
char tokenToLower[128];
char token[128];
int charPointer;
int tokenPointer;
int line = 1;
string ans;
enum tokenCategory Token;
vector<enum tokenCategory> formerTokens;
vector<pair<string, int>> formerAns;
vector<string> outputAns;
vector<string> outputErr;
const  char tokenCategoryString[39][10]
        = {"IDENFR", "INTCON", "CHARCON", "STRCON", "CONSTTK", "INTTK", "CHARTK", "VOIDTK", "MAINTK", "IFTK",
           "ELSETK", "SWITCHTK", "CASETK", "DEFAULTTK", "WHILETK", "FORTK", "SCANFTK", "PRINTFTK", "RETURNTK", "PLUS",
           "MINU", "MULT", "DIV", "LSS", "LEQ", "GRE", "GEQ", "EQL", "NEQ", "COLON",
           "ASSIGN", "SEMICN", "COMMA", "LPARENT", "RPARENT", "LBRACK", "RBRACK", "LBRACE", "RBRACE"
        };
const char reserves[15][10]
        = {"const", "int", "char", "void", "main", "if", "else", "switch", "case",
           "default", "while", "for", "scanf", "printf", "return"
        };

int lexAnalysisIm() {
    clearToken();
    while (isSpace())
        getChar();
    if (isLetter()) {
        while (isDigit() || isLetter()) {
            catToken();
            getChar();
        }
        retractChar();
        int resultValue = isReserve();
        if (resultValue == -1)
            Token = IDENFR;
        else {
            resultValue += int(CONSTTK);
            Token = tokenCategory(resultValue);
        }

    } else if (isQuote()) {
        getChar();
        if (isQuote()) {
            error_lexical();
            getChar();
            Token = STRCON;
            ans = string(token, 0, charPointer);/* NOLINT */
            return 0;
        }
        while (!isQuote()) {
            catToken();
            getChar();
            if (!isStringChar() && !isQuote() && !isSpace()) {
                error_lexical();
            }
        }
        Token = STRCON;
        getChar();
    } else if (isApostrophe()) {
        getChar();
        catToken();
        if (isChar()) {
            Token = CHARCON;
        } else {
            error_lexical();
        }
        getChar();
        getChar();
    } else if (isDigit()) {
        while (isDigit()) {
            catToken();
            getChar();
        }
        retractChar();
        Token = INTCON;
    } else if (isColon()) {
        catToken();
        Token = COLON;
        getChar();
    } else if (isLss()) {
        catToken();
        getChar();
        if (isEqu()) {
            Token = LEQ;
            catToken();
            getChar();
        } else {
            Token = LSS;
        }
    } else if (isGre()) {
        catToken();
        getChar();
        if (isEqu()) {
            Token = GEQ;
            catToken();
            getChar();
        } else {
            Token = GRE;
        }
    } else if (isEqu()) {
        catToken();
        getChar();
        if (isEqu()) {
            Token = EQL;
            catToken();
            getChar();
        } else {
            Token = ASSIGN;
        }
    } else if (isExcla()) {
        catToken();
        getChar();
        if (isEqu()) {
            Token = NEQ;
            catToken();
        } else
            error_lexical();
        getChar();
    } else if (isSemi()) {
        catToken();
        Token = SEMICN;
        getChar();
    } else if (isPlus()) {
        catToken();
        Token = PLUS;
        getChar();
    } else if (isMinus()) {
        catToken();
        Token = MINU;
        getChar();
    } else if (isMult()) {
        catToken();
        Token = MULT;
        getChar();
    } else if (isLpar()) {
        catToken();
        Token = LPARENT;
        getChar();
    } else if (isRpar()) {
        catToken();
        Token = RPARENT;
        getChar();
    } else if (isLbrack()) {
        catToken();
        Token = LBRACK;
        getChar();
    } else if (isRbrack()) {
        catToken();
        Token = RBRACK;
        getChar();
    } else if (isLbrace()) {
        catToken();
        Token = LBRACE;
        getChar();
    } else if (isRbrace()) {
        catToken();
        Token = RBRACE;
        getChar();
    } else if (isComma()) {
        catToken();
        Token = COMMA;
        getChar();
    } else if (isDivi()) {
        catToken();
        Token = DIV;
        getChar();
    } else if (isEof())
        return -1;
    else error_lexical();
    ans = string(token, 0, charPointer);/* NOLINT */
    return 0;
}

int getChar() {
    if (feof(testfile)) {
        Char = '\0';
        return -1;
    }
    Char = fgetc(testfile);/* NOLINT */
    lastChar = Char;
    return 0;
}

void clearToken() {
    if (lastChar != -1) {
        Token = IDENFR;
        tokenToLower[0] = '\0';
        token[0] = '\0';
        charPointer = 0;
    } else {
        Char = lastChar;
    }
}

int isSpace() {
    if (Char == '\n') {
        line++;
    }
    return isspace(Char);
}

int isLetter() {

    return isalpha(Char) || (Char == '_');
}

int isDigit() {
    return isdigit(Char);
}

int isChar() {
    return (Char == '+' || Char == '-' ||
            isLetter() || isdigit(Char) ||
            Char == '*' || Char == '/') ? TRUE : FALSE;
}

int isQuote() {
    return (Char == '"') ? TRUE : FALSE;
}

int isApostrophe() {
    return (Char == '\'') ? TRUE : FALSE;
}

int isColon() {
    return (Char == ':') ? TRUE : FALSE;
}

int isSemi() {
    return (Char == ';') ? TRUE : FALSE;
}

int isComma() {
    return (Char == ',') ? TRUE : FALSE;
}

int isEqu() {
    return (Char == '=') ? TRUE : FALSE;
}

int isGre() {
    return (Char == '>') ? TRUE : FALSE;
}

int isLss() {
    return (Char == '<') ? TRUE : FALSE;
}

int isExcla() {
    return (Char == '!') ? TRUE : FALSE;
}

int isPlus() {
    return (Char == '+') ? TRUE : FALSE;
}

int isMinus() {
    return (Char == '-') ? TRUE : FALSE;
}

int isMult() {
    return (Char == '*') ? TRUE : FALSE;
}

int isDivi() {
    return (Char == '/') ? TRUE : FALSE;
}

int isLpar() {
    return (Char == '(') ? TRUE : FALSE;
}

int isRpar() {
    return (Char == ')') ? TRUE : FALSE;
}

int isLbrack() {
    return (Char == '[') ? TRUE : FALSE;
}

int isRbrack() {
    return (Char == ']') ? TRUE : FALSE;
}

int isLbrace() {
    return (Char == '{') ? TRUE : FALSE;
}

int isRbrace() {
    return (Char == '}') ? TRUE : FALSE;
}

void catToken() {
    token[charPointer] = Char;
    tokenToLower[charPointer] = tolower(Char);/* NOLINT */
    charPointer++;
}

void retractChar() {
    Char = lastChar;
}

int isReserve() {
    int i = 0;
    for (; i < 15; i++) {
        string tokenNow(tokenToLower, 0, charPointer);/* NOLINT */
        string ideTk(reserves[i]);
        if (tokenNow == ideTk) {
            return i;
        }
    }
    return FALSE - 1;
}

int isEof() {
    return (Char == -1) ? TRUE : FALSE;
}

void error_lexical() {
    string errorInfo = to_string(line) + " a";
    outputErr.push_back(errorInfo);
//    printf("lexical error happens!\n");
}

int isStringChar() {
    if (Char == 32 || Char == 33 || (Char >= 35 && Char <= 126))
        return 1;
    else
        return 0;
}

int getToken() {
    if (tokenPointer) {
        tokenPointer--;
        Token = formerTokens.at(formerTokens.size() - 1 - tokenPointer);
        outputAns.push_back(
                tokenCategoryString[int(Token)] + string(" ") +
                formerAns.at(formerAns.size() - 1 - tokenPointer).first);
        return 0;
    }
    if (lexAnalysisIm() != -1) {
        outputAns.push_back(
                tokenCategoryString[int(Token)] + string(" ") + ans);
//        out << tokenCategoryString[int(Token)] << " " << ans << endl;
        formerTokens.push_back(Token);
        formerAns.emplace_back(ans, line);
    } else
        return -1;

    return 0;
}

