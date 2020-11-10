#include "compiler.h"

vector<string> identityType{"int", "char"};/* NOLINT */
vector<string> intermediateCode;//中间代码
vector<vector<string>> intermediateCodeInner;//从中间代码转MIPS时实际使用的

void addQuaternion(const vector<string>& quaternion) {
    vector<string> ansInner;
    string ans;
    for (auto &str:quaternion) {
        if (!str.empty()) {
            ans += (str + " ");
            ansInner.emplace_back(str);
        }
    }
    ans.erase(ans.length() - 1, 1);
    intermediateCode.push_back(ans);
    intermediateCodeInner.emplace_back(ansInner);
}

