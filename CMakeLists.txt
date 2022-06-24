cmake_minimum_required(VERSION 3.13)
project(Compiler)

set(CMAKE_CXX_STANDARD 11)

add_executable(Compiler main.cpp WordAnalyser.h SyntaxAnalyser.h SymbolTable.h ErrorManager.h Expression.h ErrorDetector.h Stm.h Coder.h Optimizer.h ConstAnalyser.h ConflictGraph.h DefiniationAnalyse.h DAG.h)