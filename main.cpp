#include "compiler.h"

FILE *testfile = fopen("testfile.txt", "r");/* NOLINT */

ofstream out("output.txt");/* NOLINT */
ofstream err("error.txt");/* NOLINT */
ofstream midGross("18373647_谢朋洋_优化前中间代码.txt");/* NOLINT */
ofstream midOptimize("18373647_谢朋洋_优化后中间代码.txt");/* NOLINT */
ofstream mips("");/* NOLINT */

int main() {
    synAnalysis();
    for (auto &outputAn : outputAns) {
        out << outputAn << endl;
    }
    fclose(testfile);
    out.close();
    return 0;
}
