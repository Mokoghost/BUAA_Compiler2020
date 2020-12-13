#include "compiler.h"

map<string, int> tempVarCounter;

void counterTempVar() {
    for (auto &inter:intermediateCodeInner) {
        for (auto &var:inter) {
            if (!var.empty() && var.find('$') != string::npos) {
                string temp;
                if (var.at(0) == '$')
                    temp = var;
                else {
                    vector<string> elements = boomVec(var);
                    if (elements.at(0).find('$') != string::npos) {
                        temp = elements.at(0);
                    }
                    if (elements.size() > 1 && elements.at(1).find('$') != string::npos) {
                        if (!temp.empty()) {
                            if (tempVarCounter.count(temp) == 0) {
                                tempVarCounter.insert(pair<string, int>(temp, 1));
                            } else {
                                int counter = tempVarCounter.at(temp);
                                tempVarCounter[temp] = counter + 1;
                            }
                        }
                        temp = elements.at(1);
                    }
                }
                if (tempVarCounter.count(temp) == 0) {
                    tempVarCounter.insert(pair<string, int>(temp, 1));
                } else {
                    int counter = tempVarCounter.at(temp);
                    tempVarCounter[temp] = counter + 1;
                }
            }
        }
    }
}