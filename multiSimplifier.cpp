#include "compiler.h"

void multiSimplifier() {
    for (auto &inter:intermediateCodeInner) {
        if (inter.size() == 5) {
            function f0;
            varType ty2 = whatIsThisShit(inter.at(2), f0);
            varType ty4 = whatIsThisShit(inter.at(4), f0);
            if (ty2 == INTEGER && ty4 == INTEGER && inter.at(3).at(0) == '*') {
                int one = stoi(inter.at(2));
                int two = stoi(inter.at(4));
                int ans = one * two;
                inter.pop_back();
                inter.pop_back();
                inter.pop_back();
                inter.emplace_back(to_string(ans));
            } else if (ty2 == INTEGER && ty4 == INTEGER && inter.at(3).at(0) == '/') {
                int one = stoi(inter.at(2));
                int two = stoi(inter.at(4));
                int ans = one / two;
                inter.pop_back();
                inter.pop_back();
                inter.pop_back();
                inter.emplace_back(to_string(ans));
            } else if (((ty2 == INTEGER && stoi(inter.at(2)) == 0)
                        || (ty4 == INTEGER && stoi(inter.at(4)) == 0))
                       && inter.at(3).at(0) == '*') {
                int ans = 0;
                inter.pop_back();
                inter.pop_back();
                inter.pop_back();
                inter.emplace_back(to_string(ans));
            } else if (ty2 == INTEGER && stoi(inter.at(2)) == 0 && inter.at(3).at(0) == '/') {
                int ans = 0;
                inter.pop_back();
                inter.pop_back();
                inter.pop_back();
                inter.emplace_back(to_string(ans));
            } else {
                if (whatIsThisShit(inter.at(4), f0) == INTEGER
                    && inter.at(3).at(0) == '*') {
                    int value = stoi(inter.at(4));
                    if (value >= 0) {
                        if (!(value & (value - 1)) && value != 0) {
                            value = log2(value);
                            inter.pop_back();
                            inter.pop_back();
                            inter.emplace_back("<<");
                            inter.emplace_back(to_string(value));
                        }
                    } else {
                        int v2 = -value;
                        if (!(v2 & (v2 - 1))) {
                            v2 = log2(v2);
                            inter.pop_back();
                            inter.pop_back();
                            inter.emplace_back("<<");
                            inter.emplace_back("-" + to_string(v2));
                        }
                    }
                }
                if (whatIsThisShit(inter.at(4), f0) == INTEGER
                    && inter.at(3).at(0) == '/') {
                    int value = stoi(inter.at(4));
                    if (value > 0) {
                        if (!(value & (value - 1))) {
                            value = log2(value);
                            inter.pop_back();
                            inter.pop_back();
                            inter.emplace_back(">>");
                            inter.emplace_back(to_string(value));
                        }
                    } else {
                        int v2 = -value;
                        if (!(v2 & (v2 - 1))) {
                            v2 = log2(v2);
                            inter.pop_back();
                            inter.pop_back();
                            inter.emplace_back(">>");
                            inter.emplace_back("-" + to_string(v2));
                        }
                    }
                }
            }
        }
    }
}


int log2(int value) {
    int v = value;
    int x = 0;
    while (v > 1) {
        v >>= 1;
        x++;
    }
    return x;
}