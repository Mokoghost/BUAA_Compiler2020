cmake_minimum_required(VERSION 3.17)
project(BUAA_Compiler2020_Clion_Edition)

set(CMAKE_CXX_STANDARD 14)

#include_directories(SYSTEM "/foo/bar")
add_executable(BUAA_Compiler2020_Clion_Edition main.cpp lexicalAnalysis.cpp syntaxAnalysis.cpp function.cpp Identifier.cpp compiler.h identityTable.cpp tokenType.cpp functionTable.cpp GenerateIntermediateCode.cpp GenerateMIPSAssembly.cpp CopyPropagation.cpp multiSimplifier.cpp changeSameName.cpp allocateRegisterForGlobalVar.cpp tempVarCounter.cpp)