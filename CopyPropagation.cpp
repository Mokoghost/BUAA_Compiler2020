#include "compiler.h"

map<string, string> consts;
set<string> charTypeTempVar;

void copyPropagation() {
/**
 * 初次优化针对常数传播
 */
    for (auto iter = intermediateCodeInner.begin(); iter != intermediateCodeInner.end();) {
        vector<string> inter = *iter;
        if (inter.size() == 3) {
            function f0;
            varType ty0 = whatIsThisShit(inter.at(0), f0);
            varType ty2 = whatIsThisShit(inter.at(2), f0);
            if (inter.at(1) == "=" && ty0 == TEMP_VAR &&
                (ty2 == INTEGER || ty2 == CHARACTER || ty2 == TEMP_VAR
                 || inter.at(2).find('[') == string::npos) && ty2 != RET_VAL) {
                consts.insert(pair<string, string>(inter.at(0), inter.at(2)));
                iter = intermediateCodeInner.erase(iter);
            } else if (inter.at(1) == "=" && ty0 == TEMP_VAR
                       && inter.at(2).find('[') != string::npos) {
                vector<string> elements = boomVec(inter.at(2));
                if (isdigit(elements.at(0).at(0))) {
                    if (elements.size() == 2) {
                        if (isdigit(elements.at(1).at(0))) {
                            consts.insert(pair<string, string>(inter.at(0), inter.at(2)));
                            iter = intermediateCodeInner.erase(iter);
                        } else
                            iter++;
                    } else {
                        consts.insert(pair<string, string>(inter.at(0), inter.at(2)));
                        iter = intermediateCodeInner.erase(iter);
                    }
                } else
                    iter++;
            } else if (inter.at(1) == "=" && ty0 == TEMP_VAR &&
                       (ty2 == INTEGER || ty2 == CHARACTER || ty2 == TEMP_VAR
                        || inter.at(2).find('[') == string::npos) && ty2 == RET_VAL) {
                if (functions.getToken(inter.at(2).substr(5, inter.at(2).size() - 5)).getTokenCategory() ==
                    CHARTK) {//若是字符型返回值
                    charTypeTempVar.insert(inter.at(0));
                }
                iter++;
            } else
                iter++;
        } else
            iter++;
    }
    for (auto &inter:intermediateCodeInner) {
        for (auto &str:inter) {
            if (str.find('[') != string::npos) {
                vector<string> elements = boomVec(str);
                if (consts.count(elements.at(0)) != 0) {
                    str.replace(str.find(elements.at(0)),
                                elements.at(0).size(),
                                consts.at(elements.at(0)));
                }
                if (elements.size() == 2) {
                    if (consts.count(elements.at(1)) != 0) {
                        str.replace(str.find(elements.at(1)),
                                    elements.at(1).size(),
                                    consts.at(elements.at(1)));
                    }
                }
            } else if (consts.count(str) != 0) {
                str.replace(0, str.size(), consts.at(str));
            } else if (inter.size() == 4 && inter.at(2) == "+") {
                string vn3 = inter.at(3);
                inter.pop_back();
                inter.pop_back();
                inter.push_back(vn3);
            }
        }
    }
}

vector<string> boomVec(string array) {
    vector<string> ans;
    int rb1 = 0, lb1 = array.find('['), rb2 = 0, lb2 = 0;
    int lbNum = 1;
    int i = lb1;
    while (lbNum) {
        i++;
        if (array.at(i) == '[') {
            lbNum++;
        } else if (array.at(i) == ']') {
            lbNum--;
        }
    }
    rb1 = i;
    lb2 = rb1 + 1;
    string d1 = array.substr(lb1 + 1, rb1 - lb1 - 1);
    ans.push_back(d1);
    if (rb1 == array.size() - 1) {
        return ans;
    } else {
        i = lb2;
        lbNum = 1;
        while (lbNum) {
            i++;
            if (array.at(i) == '[') {
                lbNum++;
            } else if (array.at(i) == ']') {
                lbNum--;
            }
        }
        rb2 = i;
        string d2 = array.substr(lb2 + 1, rb2 - lb2 - 1);
        ans.push_back(d2);
        return ans;
    }
}