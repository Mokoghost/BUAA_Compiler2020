#include "compiler.h"

FILE *testfile = fopen("testfile1.txt", "r");/* NOLINT */
ifstream in("testfile.txt");
ofstream out("output.txt");/* NOLINT */
ofstream err("error.txt");/* NOLINT */
ofstream midGross("18373647_xpy_before_optimize.txt");/* NOLINT */
ofstream midOptimize("18373647_xpy_after_optimize.txt");/* NOLINT */
ofstream mips("mips.txt");/* NOLINT */
ofstream mipsTest("mips.asm");/* NOLINT */

int main() {
    synAnalysis();
    copyPropagation();
    copyPropagation();
    multiSimplifier();
    copyPropagation();
    changeSameName();
    counterTempVar();
    allocateRegisterForGlobalVar();
    outputIntermediateCode();
    for (auto &outputAn : outputErr) {
        err << outputAn << endl;
    }
    for (auto &outputAn : intermediateCode) {
        midGross << outputAn << endl;
    }
    generateMIPSAssembly();
    for (auto &outputAn : mipsCode) {
        mips << outputAn << endl;
    }
    for (auto &outputAn : mipsCode) {
        mipsTest << outputAn << endl;
    }
    fclose(testfile);
    out.close();
    err.close();
    midGross.close();
    midOptimize.close();
    mips.close();
    return 0;
}
