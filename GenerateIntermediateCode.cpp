#include "compiler.h"

void addQuaternion(vector<string> strings) {
    string ans;
    for (auto &str:strings) {
        ans += (str + " ");
    }
    ans.erase(ans.length() - 1, 1);
    intermediateCode.push_back(ans);
}

