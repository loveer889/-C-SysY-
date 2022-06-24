#ifndef COMPILER_SYNTAXANALYSER_H
#define COMPILER_SYNTAXANALYSER_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include "sstream"
#include "WordAnalyser.h"
#include <fstream>
#include "ErrorManager.h"
#include "SymbolTable.h"
#include "Expression.h"
#include <memory>
#include "Stm.h"

using namespace std;

class SyntaxAnalyser {

public:
    explicit SyntaxAnalyser(string filePath) {
        WordAnalyser wordAnalyser(filePath);
        wordAnalyser.analyse();
        allWords = wordAnalyser.getAnalyseResult();
    }

    void analyse() {
        CompUnit();
    }

private:
    int i = 0;
    int memoryi = 0;
    vector<tuple<string, string, int> > allWords;  // 词法分析器给出的所有单词
    LabelManager procedureManager = LabelManager();  // 过程记录器，主要记录while的label
    SymbolTable symbolTable = SymbolTable();

    void setMemory() {
        memoryi = i;
    }

    void returnCommon() {
        i = memoryi;
    }

    string getNextWord() {
        return get<0>(allWords[i++]);
    }

    bool isTopWord(string word) {
        return i < allWords.size() && get<0>(allWords[i]) == word;
    }

    bool isTopType(string type) {
        return i < allWords.size() && get<1>(allWords[i]) == type;
    }


    bool isFarWord(string word, int offset) {
        return ((i + offset) < allWords.size()) && get<0>(allWords[i + offset]) == word;
    }

    tuple<bool> CompUnit() {
        while (isTopWord("const") || (isTopWord("int") && !isFarWord("(", 2))) {
            if (isTopWord("const")) {
                ConstDecl();
            } else {
                VarDecl();
            }
        }

        while (isTopWord("void") || isTopWord("int")) {
            if (isTopWord("int") &&
                isFarWord("main", 1)
                && (isFarWord(")", 3) || isFarWord("{", 3))) {
                MainFuncDef();
            } else {
                FuncDef();
            }
        }
        return (true);
    }

    tuple<bool> ConstDecl() {
        getNextWord();  // 'const'
        getNextWord();  // 'int'
        ConstDef();
        while (isTopWord(",")) {
            getNextWord();  // ,
            ConstDef();
        }
        getNextWord();  // ;
        return (true);
    }

    tuple<bool> ConstDef() {
        string IdentName = getNextWord(); // 常量名
        vector<int> dims;  // 直接获取常量维度的字面量
        while (isTopWord("[")) {
            getNextWord();  // [
            dims.emplace_back(get<1>(ConstExp()));
            getNextWord(); // ]
        }
        getNextWord();  // =
        vector<int> numbers = get<1>(ConstInitVal());  //  常量字面量
        symbolTable.addConst(IdentName, dims, numbers);
        shared_ptr<Symbol> ptr = symbolTable.lastPtr;
        ptr->setIndex(Stm::addConst(dims, numbers));  // 告诉符号表，这个是我中间代码里的第几个变量，下次直接查符号表就知道了。
        return (true);
    }

    tuple<bool, vector<int>> ConstInitVal() {
        vector<int> numbers;
        if (isTopWord("{")) {
            getNextWord(); // {
            if (!isTopWord("}")) {
                vector<int> newNumbers = get<1>(ConstInitVal());
                numbers.insert(numbers.end(), newNumbers.begin(), newNumbers.end());
                while (isTopWord(",")) {
                    getNextWord(); // ,
                    newNumbers = get<1>(ConstInitVal());
                    numbers.insert(numbers.end(), newNumbers.begin(), newNumbers.end());
                }
            }
            getNextWord(); // }
        } else {
            numbers.emplace_back(get<1>(ConstExp()));
        }
        return {true, numbers};
    }

    tuple<bool> VarDecl() {
        getNextWord();  // 'int'
        VarDef();
        while (isTopWord(",")) {
            getNextWord();  // ,
            VarDef();
        }
        getNextWord();  // ;
        return (true);
    }

    tuple<bool> VarDef() {
        string identName = getNextWord();
        vector<int> dims;
        while (isTopWord("[")) {
            getNextWord();  // [
            dims.emplace_back(get<1>(ConstExp()));
            getNextWord(); // ]
        }

        symbolTable.addVar(identName, dims);
        shared_ptr<Symbol> ptr = symbolTable.lastPtr;
        int varIndex = Stm::addVar(dims);
        ptr->setIndex(varIndex);

        if (isTopWord("=")) {
            getNextWord();  // =
            vector<shared_ptr<Expression> > expressions = get<1>(InitVal());
            vector<int> indexes;
            for (auto &ptr1 : expressions) {
                indexes.emplace_back(ptr1->getIndex());
            }
            Stm::setInitialValues(varIndex, ((int) dims.size()), indexes);
        }
        return (true);
    }

    tuple<bool, vector<shared_ptr<Expression> > > InitVal() {
        vector<shared_ptr<Expression> > expressions;
        if (isTopWord("{")) {
            getNextWord();  // {
            if (!isTopWord("}")) {
                vector<shared_ptr<Expression> > moreExpressions = get<1>(InitVal());
                expressions.insert(expressions.end(), moreExpressions.begin(), moreExpressions.end());
                while (isTopWord(",")) {
                    getNextWord();  // ,
                    vector<shared_ptr<Expression> > moreExpressions = get<1>(InitVal());
                    expressions.insert(expressions.end(), moreExpressions.begin(), moreExpressions.end());
                }
            }
            getNextWord();  // }
        } else {
            expressions.emplace_back(get<1>(Exp()));
        }
        return {true, expressions};
    }

    tuple<bool> FuncDef() {
        int protect_label = Stm::getLabel();
        Stm::jump(protect_label);

        string functionType = get<1>(FuncType());
        string functionName = getNextWord();

        symbolTable.addFunction(functionName, functionType);
        shared_ptr<Symbol> ptr = symbolTable.lastPtr;   // 刚刚添加的这个函数的指针
        int functionIndex = Stm::addFunction(functionType);
        ptr->setIndex(functionIndex);
        symbolTable.addLev();

        getNextWord();  // (
        if (isTopWord("int")) {
            FuncFParams();
        }
        getNextWord();  // )

        Block();

        Stm::functionReturn(-1, functionIndex);    // 补充返回语句
        Stm::functionEnd(functionIndex);
        Stm::setLabel(protect_label);
        symbolTable.deleteLev();
        return (true);
    }

    tuple<bool> MainFuncDef() {
        string returnType = getNextWord();  // 'int'
        string name = getNextWord();  // "main"
        symbolTable.addFunction(name, returnType);
        symbolTable.addLev();
        Stm::setMain();
        getNextWord();  // (
        getNextWord();  // )
        Block();
        Stm::mainReturn();

        symbolTable.deleteLev();
        return (true);
    }

    tuple<bool, string> FuncType() {
        string funcType = getNextWord();
        return {true, funcType};
    }

    tuple<bool> FuncFParams() {
        FuncFParam();
        while (isTopWord(",")) {
            getNextWord();  // ,
            FuncFParam();
        }
        return {true};
    }

    tuple<bool> FuncFParam() {
        getNextWord();  // 'int'
        string identName = getNextWord();
        vector<int> dims;
        if (isTopWord("[")) {
            getNextWord();  // [
            getNextWord();  // ]
            dims.emplace_back(-1);
            while (isTopWord("[")) {
                getNextWord();  // [
                dims.emplace_back(get<1>(ConstExp()));
                getNextWord();  // ]
            }
        }
        symbolTable.addPara(identName, dims);
        shared_ptr<Symbol> paraPtr = symbolTable.lastPtr;
        paraPtr->setIndex(Stm::addPara(dims));
        return (true);
    }

    tuple<bool, int> Block() {
        getNextWord();  // {
        while (!isTopWord("}")) {
            if (isTopWord("const")) {
                ConstDecl();
            } else if (isTopWord("int")) {
                VarDecl();
            } else {
                Stmt();
            }
        }
        getNextWord();  // }
        return {true, -1};
    }

    tuple<bool> Stmt() {
        if (isTopWord(";")) {
            getNextWord();  // ;
        } else if (isTopWord("{")) {
            symbolTable.addLev();
            Block();
            symbolTable.deleteLev();
        } else if (isTopWord("if")) {
            getNextWord();  // if
            getNextWord();  // (
            int if_label_1 = Stm::getLabel();  // if 语句如果执行，那么应当跳转到哪个标签。
            int if_label_2 = Stm::getLabel();  // if 语句如果不执行，那么应当跳转到哪个标签
            Cond(if_label_1, if_label_2);
            getNextWord();  // )
            Stm::setLabel(if_label_1);
            Stmt();
            if (isTopWord("else")) {
                int else_end_label = Stm::getLabel();
                Stm::jump(else_end_label);
                Stm::setLabel(if_label_2);
                getNextWord();  // else
                Stmt();
                Stm::setLabel(else_end_label);
            } else {
                Stm::setLabel(if_label_2);
            }
        } else if (isTopWord("while")) {
            int while_label1 = Stm::getLabel();  // 条件判断之前
            int while_label2 = Stm::getLabel();  // 即将开始执行
            int while_label3 = Stm::getLabel();  // 不执行
            procedureManager.push_while_label(while_label1, while_label2, while_label3);
            Stm::setLabel(while_label1);
            getNextWord();  // while
            getNextWord();  // (
            Cond(while_label2, while_label3);
            getNextWord();  // )
            Stm::setLabel(while_label2);
            Stmt();
            Stm::jump(while_label1);
            Stm::setLabel(while_label3);
            procedureManager.pop_while_label();
        } else if (isTopWord("break")) {
            getNextWord();  // break
            getNextWord();  // ;
            int label_ = get<2>(procedureManager.get_while_label());
            Stm::jump(label_);
        } else if (isTopWord("continue")) {
            getNextWord();  // continue
            getNextWord();  // ;
            int label_ = get<0>(procedureManager.get_while_label());
            Stm::jump(label_);
        } else if (isTopWord("return")) {
            getNextWord();  // return
            shared_ptr<Function> funcPtr = symbolTable.getFunc();
            if (isTopWord(";")) {
                getNextWord();  // ;
                if (funcPtr->getIndex() == -1) {
                    Stm::mainReturn();
                } else {
                    Stm::functionReturn(-1, funcPtr->getIndex());
                }
            } else {
                shared_ptr<Expression> exp_ptr = get<1>(Exp());
                getNextWord();  // ;
                int resultIndex = exp_ptr->getIndex();
                if (funcPtr->getIndex() == -1) {
                    Stm::mainReturn();
                } else {
                    Stm::functionReturn(resultIndex, funcPtr->getIndex());
                }
            }
        } else if (isTopWord("printf")) {
            getNextWord();  // printf
            getNextWord();  // (
            string formatString = getNextWord();
            // 检查到这里
            vector<int> expressions;
            while (isTopWord(",")) {
                getNextWord();  // ,
                expressions.emplace_back(get<1>(Exp())->getIndex());
            }
            stringstream pool;
            int nextIndex = 0;
            for (int i = 1; i < formatString.size() - 1; i++) {
                char ch = formatString[i];
                if (ch == '%') {
                    string lastStr = pool.str();
                    pool = stringstream();
                    if (!lastStr.empty()) {
                        Stm::printStr(lastStr);
                    }
                    i++;
                    Stm::printInt(expressions[nextIndex]);
                    nextIndex++;
                } else {
                    pool << ch;
                }
            }
            string lastStr = pool.str();
            if (!lastStr.empty()) {
                Stm::printStr(lastStr);
            }
            getNextWord();  // )
            getNextWord();  // ;
        } else { // Exp 或者左值赋值式
            if (isTopType("IDENFR")) {
                setMemory();
                LVal();
                bool isLVal = isTopWord("=");
                returnCommon();
                if (isLVal) {
                    shared_ptr<LValExpression> lValueExpression = get<1>(LVal());
                    int offSet = -1;
                    if (lValueExpression->isOriArray()) {
                        offSet = lValueExpression->getOffset();
                    }

                    getNextWord();  // =
                    int assignIndex = 0;
                    if (isTopWord("getint")) {
                        getNextWord();  // getint
                        getNextWord();  // (
                        getNextWord();  // )
                        assignIndex = Stm::getInt();
                    } else {
                        assignIndex = get<1>(Exp())->getIndex();
                    }
                    if (offSet == -1) {  // 这个左值是普通变量
                        Stm::assignVarValue(lValueExpression->getIndex(), assignIndex);
                    } else {
                        Stm::assignArrayValue(lValueExpression->getIndex(), offSet, assignIndex);
                    }
                } else {
                    get<1>(Exp())->getIndex();
                }
                getNextWord();  // ;
            } else {
                get<1>(Exp())->getIndex();
                getNextWord();  // ;
            }
        }
        return (true);
    }

    tuple<bool, shared_ptr<Expression> > Exp() {
        shared_ptr<AddExpression> addPtr = get<1>(AddExp());
        return {true, make_shared<Expression>(addPtr)};
    }

    tuple<bool> Cond(int label_1, int label_2) {
        //  判断条件时，如果发现成立，那么应当跳转到label_1,如果发现不成立，那么应当跳转到label_2。
        LOrExp(label_1, label_2);
        return (true);
    }

    tuple<bool, shared_ptr<LValExpression>> LVal() {
        string name = getNextWord(); // IdentName
        shared_ptr<LValue> lValPtr = symbolTable.getLVal(name);
        shared_ptr<LValExpression> lValExpression = make_shared<LValExpression>(lValPtr);
        while (isTopWord("[")) {
            getNextWord();  // [
            shared_ptr<Expression> exp = get<1>(Exp());
            lValExpression->addInsight(exp);
            getNextWord();  // ]
        }
        return {true, lValExpression};
    }

    tuple<bool, shared_ptr<PrimaryExpression> > PrimaryExp() {
        shared_ptr<PrimaryExpression> primaryExpressionPtr;
        if (isTopWord("(")) {
            getNextWord();  // (
            shared_ptr<Expression> expression = get<1>(Exp());
            primaryExpressionPtr = make_shared<PrimaryExpression>(expression);
            getNextWord();  // )
        } else if (isTopType("IDENFR")) {
            primaryExpressionPtr = make_shared<PrimaryExpression>(get<1>(LVal()));
        } else {
            primaryExpressionPtr = make_shared<PrimaryExpression>(get<1>(Number()));
        }
        return {true, primaryExpressionPtr};
    }

    tuple<bool, shared_ptr<NumberExpression> > Number() {
        string word = getNextWord();
        return {true, make_shared<NumberExpression>(word)};
    }

    tuple<bool, shared_ptr<UnaryExpression> > UnaryExp() {
        shared_ptr<UnaryExpression> unaryExpressionPtr;
        if (isTopWord("+") || isTopWord("-") || isTopWord("!")) {
            string op = get<1>(UnaryOp());
            unaryExpressionPtr = make_shared<UnaryExpression>(op, get<1>(UnaryExp()));
        } else if (isTopType("IDENFR") && isFarWord("(", 1)) {  // 函数调用的情况
            string identName = getNextWord();
            shared_ptr<Function> funcPtr = symbolTable.getFunc(identName);
            shared_ptr<FunctionCall> functionCall = make_shared<FunctionCall>(funcPtr);
            getNextWord();  // (
            // 判断到底有没有函数参数。
            if (isTopWord("(") || isTopType("IDENFR") || isTopType("INTCON") || isTopWord("+") ||
                isTopWord("-")) {  // First(Exp)，但是普通表达式不含感叹号
                FuncRParams(functionCall);  // 这里需要结合行的信息判断
            }
            getNextWord();  // )
            unaryExpressionPtr = make_shared<UnaryExpression>(functionCall);
        } else {
            unaryExpressionPtr = make_shared<UnaryExpression>(get<1>(PrimaryExp()));
        }
        return {true, unaryExpressionPtr};
    }

    tuple<bool, string> UnaryOp() {
        string op = getNextWord();  // + | - | !
        return {true, op};
    }

    tuple<bool> FuncRParams(shared_ptr<FunctionCall> functionCall) {
        shared_ptr<Expression> expression = get<1>(Exp());
        functionCall->addParam(expression);
        while (isTopWord(",")) {
            getNextWord();  // ,
            expression = get<1>(Exp());
            functionCall->addParam(expression);
        }
        return {true};
    }

    tuple<bool, shared_ptr<MulExpression> > MulExp() {
        shared_ptr<MulExpression> mulPtr = make_shared<MulExpression>(get<1>(UnaryExp()));
        while (isTopWord("*") || isTopWord("/") || isTopWord("%")) {
            string opera = getNextWord();  // * | / | %
            mulPtr->addUnaryExpression(get<1>(UnaryExp()), opera);
        }
        return {true, mulPtr};
    }

    tuple<bool, shared_ptr<AddExpression> > AddExp() {
        shared_ptr<AddExpression> addPtr = make_shared<AddExpression>(get<1>(MulExp()));
        while (isTopWord("+") || isTopWord("-")) {
            string opera = getNextWord();  // + | -
            addPtr->addMulExpression(get<1>(MulExp()), opera);
        }
        return {true, addPtr};
    }

    tuple<bool, int> RelExp() {
        int result1 = get<1>(Exp())->getIndex();
        while (isTopWord("<") || isTopWord(">") || isTopWord("<=") || isTopWord(">=")) {
            string word = getNextWord();  // < | > | <= | >=
            int result2 = get<1>(Exp())->getIndex();
            if (word == "<") {
                result1 = Stm::lt(result1, result2);
            } else if (word == ">") {
                result1 = Stm::gt(result1, result2);
            } else if (word == "<=") {
                result1 = Stm::le(result1, result2);
            } else {
                result1 = Stm::ge(result1, result2);
            }
        }
        return {true, result1};
    }

    tuple<bool> EqExp(int label1, int label2) {
        int rel_result1 = get<1>(RelExp());
        while (isTopWord("==") || isTopWord("!=")) {
            string word = getNextWord();  //  == | !=
            int rel_result2 = get<1>(RelExp());
            if (word == "==") {
                rel_result1 = Stm::equal(rel_result1, rel_result2);
            } else {    // !=
                rel_result1 = Stm::notEqual(rel_result1, rel_result2);
            }
        }

        Stm::jumpIf(rel_result1, label1);
        Stm::jumpIfNot(rel_result1, label2);
        return (true);
    }

    tuple<bool> LAndExp(int label1, int label2) {
        int nextLabel = Stm::getLabel();
        EqExp(nextLabel, label2);
        Stm::setLabel(nextLabel);
        while (isTopWord("&&")) {
            getNextWord();  // &&
            nextLabel = Stm::getLabel();
            EqExp(nextLabel, label2);
            Stm::setLabel(nextLabel);
        }
        Stm::jump(label1);
        return (true);
    }

    tuple<bool> LOrExp(int label1, int label2) {
        int nextLabel = Stm::getLabel();
        LAndExp(label1, nextLabel);
        Stm::setLabel(nextLabel);
        while (isTopWord("||")) {
            nextLabel = Stm::getLabel();
            getNextWord();  // ||
            LAndExp(label1, nextLabel);
            Stm::setLabel(nextLabel);
        }
        Stm::jump(label2);
        return (true);
    }

    tuple<bool, int> ConstExp() {
        shared_ptr<AddExpression> addPtr = get<1>(AddExp());
        return {true, addPtr->getValue()};
    }

};


#endif //COMPILER_SYNTAXANALYSER_H
