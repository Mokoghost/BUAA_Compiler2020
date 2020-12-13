#include <utility>

#include "compiler.h"

vector<string> identityType{"int", "char"};/* NOLINT */
vector<string> intermediateCode;//中间代码
vector<vector<string>> intermediateCodeInner;//从中间代码转MIPS时实际使用的

void addQuaternion(const vector<string> &quaternion) {
    vector<string> ansInner;
    for (auto &str:quaternion) {
        if (!str.empty()) {
            string newStr = str;
            trim(newStr);
            ansInner.emplace_back(newStr);
        }
    }
    intermediateCodeInner.emplace_back(ansInner);
}

void addSpace() {
    intermediateCodeInner.emplace_back(vector<string>{""});
}

void addFunction(string fnName) {
    intermediateCodeInner.emplace_back(vector<string>{std::move(fnName)});
}

void addQuaternions(vector<vector<string>> intermediateInner) {
    intermediateCodeInner.insert(intermediateCodeInner.end(), intermediateInner.begin(), intermediateInner.end());
}

void trim(string &s) {
    if (s.empty()) {
        return;
    }
    s.erase(0, s.find_first_not_of(' '));
    s.erase(s.find_last_not_of(' ') + 1);
}

void outputIntermediateCode() {
    for (auto &inter:intermediateCodeInner) {
        string ans;
        if (inter.size() == 1 && functions.contains(inter.at(0))) {
            function fn = functions.getToken(inter.at(0));
            ans = string(reserves[fn.getTokenCategory() - INTTK + 1]) + " " + inter.at(0)+"()";
            intermediateCode.push_back(ans);
        } else if (inter.size() == 1 && inter.at(0).empty()) {
            intermediateCode.push_back(ans);
        } else {
            for (auto &str:inter) {
                if (!str.empty()) {
                    string newStr = str;
                    trim(newStr);
                    ans += (newStr + " ");
                }
            }
            ans.erase(ans.length() - 1, 1);
            intermediateCode.push_back(ans);
        }
    }
}