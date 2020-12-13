#include "compiler.h"

void changeSameName() {
    function fn;
    for (auto &inter:intermediateCodeInner) {
        if (functions.contains(inter.at(0))) {
            fn = functions.getToken(inter.at(0));
        } else if (fn.getName().empty()) {
            continue;
        } else {
            for (int i = 0; i < inter.size(); i++) {
                string element = inter.at(i);
                string real = getRealName(element, '[');
                if (fn.hasSameName(real)) {
                    vector<string> replace;
                    int size = inter.size();
                    for (int j = 0; j < size - i; j++) {
                        replace.push_back(inter.at(size - 1 - j));
                        inter.pop_back();
                    }
                    inter.push_back(fn.getReplaceName(real));
                    size = replace.size();
                    for (int j = 1; j < size; j++) {
                        inter.push_back(replace.at(size - 1 - j));
                    }
                }
            }
        }
    }
    functions.changeSameNames();
}