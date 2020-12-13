#include "compiler.h"

void allocateRegisterForGlobalVar() {
    string fnName;
    for (auto &inter:intermediateCodeInner) {
        if (functions.contains(inter.at(0))) {
            fnName = functions.getToken(inter.at(0)).getName();
        } else {
            for (const auto& element : inter) {
                if (globalTable.contains(element)
                    && globalTable.getToken(element).getIdCategory() != CONST) {
                    functions.addGlobalVarUsed(fnName, element);
                }
            }
        }
    }
}
