//
// Created by HP on 2021/11/12.
//

#ifndef COMPILER_ERRORDETECTOR_H
#define COMPILER_ERRORDETECTOR_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include "WordAnalyser.h"
#include <fstream>
#include "ErrorManager.h"
#include "SymbolTable.h"
#include "Expression.h"
#include <memory>

using namespace std;

class ErrorDetector {

public:
    explicit ErrorDetector(string filePath) {
        WordAnalyser wordAnalyser(filePath);
        wordAnalyser.analyse();
        allWords = wordAnalyser.getAnalyseResult();
    }

    bool analyse(bool isOutPut,bool isOutPutError) {
        CompUnit();
        if(isOutPut){
            procedureManager.outPut();
        }
        if(isOutPutError){
            errorManager.outPut();
        }
        return !errorManager.isErrorDetected();  // 返回是否正确
    }

private:
    int i = 0;
    int memoryi = 0;
    vector<tuple<string, string, int> > allWords;  // 词法分析器给出的所有单词
    ErrorManager errorManager = ErrorManager("error.txt");
    ProcedureManager procedureManager = ProcedureManager("output.txt");
    SymbolTable symbolTable = SymbolTable();


    void setMemory() {
        memoryi = i;
        procedureManager.stopOutPut();
        errorManager.stopOutPut();
    }

    void returnCommon() {
        i = memoryi;
        procedureManager.continueOutPut();
        errorManager.continueOutPut();
    }


    string getNextWord(){
        procedureManager.addInformation(get<1>(allWords[i]) + " " + get<0>(allWords[i]));
        return get<0>(allWords[i++]);
    }

    bool isTopWord(string word) {
        return i < allWords.size() && get<0>(allWords[i]) == word;
    }

    bool isTopType(string type) {
        return i < allWords.size() && get<1>(allWords[i]) == type;
    }

    int getPreLine(){
        if(i==0){
            return 0;
        }else{
            return get<2>(allWords[i-1]);
        }
    }

    bool isFarWord(string word, int offset) {
        return ((i + offset) < allWords.size()) && get<0>(allWords[i + offset]) == word;
    }

    tuple<bool> CompUnit() {
        procedureManager.push("CompUnit");
        while (isTopWord("const") || (isTopWord("int") && !isFarWord("(", 2))) {
            if (isTopWord("const")) {
                ConstDecl();
            } else {
                VarDecl();
            }
        }
        //  主函数也可能重复 : ①名字为main;②返回int③无参数
        while(isTopWord("void")||isTopWord("int")){
            if(isTopWord("int")&&
               isFarWord("main",1)
               &&(isFarWord(")",3)||isFarWord("{",3))){
                MainFuncDef();
            }else{
                FuncDef();
            }
        }
        procedureManager.addInformation("<CompUnit>");
        procedureManager.pop();
        return (true);
    }

    tuple<bool> ConstDecl() {
        procedureManager.push("ConstDecl");
        getNextWord();  // 'const'
        getNextWord();  // 'int'
        ConstDef();
        while (isTopWord(",")) {
            getNextWord();  // ,
            ConstDef();
        }
        if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
            getNextWord();
        }
        procedureManager.addInformation("<ConstDecl>");
        procedureManager.pop();
        return (true);
    }

    tuple<bool> ConstDef() {
        procedureManager.push("ConstDef");
        string IdentName = getNextWord(); // Type : Ident
        int line = getPreLine();
        vector<int>dims;
        while (isTopWord("[")) {
            getNextWord();  // [
            dims.emplace_back(get<1>(ConstExp()));  // 各维的长度
            if(errorManager.assert(isTopWord("]"),getPreLine(),"k")){
                getNextWord();
            }
        }
        getNextWord();  // =
        vector<int> numbers = get<1>(ConstInitVal());  // 所有数字
        errorManager.assert(symbolTable.addConst(IdentName,dims,numbers),line,"b");
        procedureManager.addInformation("<ConstDef>");
        procedureManager.pop();
        return (true);
    }

    tuple<bool,vector<int>> ConstInitVal() {
        procedureManager.push("ConstInitVal");
        vector<int> numbers;
        if (isTopWord("{")) {
            getNextWord(); // {
            if (!isTopWord("}")) {
                vector<int> newNumbers = get<1>(ConstInitVal());
                numbers.insert(numbers.end(),newNumbers.begin(),newNumbers.end());
                while (isTopWord(",")) {
                    getNextWord(); // ,
                    newNumbers = get<1>(ConstInitVal());
                    numbers.insert(numbers.end(),newNumbers.begin(),newNumbers.end());
                }
            }
            getNextWord(); // }
        } else {
            numbers.emplace_back(get<1>(ConstExp()));
        }
        procedureManager.addInformation("<ConstInitVal>");
        procedureManager.pop();
        return {true,numbers};
    }

    tuple<bool> VarDecl() {
        procedureManager.push("VarDecl");
        getNextWord();  // 'int'
        VarDef();
        while (isTopWord(",")) {
            getNextWord();  // ,
            VarDef();
        }
        if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
            getNextWord();
        }
        procedureManager.addInformation("<VarDecl>");
        procedureManager.pop();
        return (true);
    }

    tuple<bool> VarDef() {
        procedureManager.push("VarDef");
        string identName = getNextWord();
        int line = getPreLine();
        vector<int> dims ;
        while (isTopWord("[")) {
            getNextWord();  // [
            dims.emplace_back(get<1>(ConstExp()));
            if(errorManager.assert(isTopWord("]"),getPreLine(),"k")){
                getNextWord();
            }
        }
        if (isTopWord("=")) {
            getNextWord();  // =
            InitVal();  // TODO 没有提取变量初值的信息。
        }
        errorManager.assert(symbolTable.addVar(identName,dims),line,"b");
        procedureManager.addInformation("<VarDef>");
        procedureManager.pop();
        return (true);
    }

    tuple<bool> InitVal() {
        if (isTopWord("{")) {
            getNextWord();  // {
            if (!isTopWord("}")) {
                InitVal();
                while (isTopWord(",")) {
                    getNextWord();  // ,
                    InitVal();
                }
            }
            getNextWord();  // }
        } else {
            Exp();
        }
        procedureManager.addInformation("<InitVal>");
        return (true);
    }

    tuple<bool> FuncDef() {
        string functionType = get<1>(FuncType());
        string functionName = getNextWord();
        errorManager.assert(symbolTable.addFunction(functionName,functionType),getPreLine(),"b");
        symbolTable.addLev();

        getNextWord();  // (
        if (isTopWord("int")) {
            FuncFParams();
        }
        if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
            getNextWord();
        }
        Block();

        shared_ptr<Function> funcPtr = symbolTable.getFunc(functionName);
        errorManager.assert(functionType == "void" || (functionType=="int" && funcPtr->haveReturn()),getPreLine(),"g");
        symbolTable.deleteLev();
        procedureManager.addInformation("<FuncDef>");
        return (true);
    }

    tuple<bool> MainFuncDef() {
        string returnType = getNextWord();  // 'int'
        string name = getNextWord();  // "main"
        errorManager.assert(symbolTable.addFunction(name,returnType),getPreLine(),"b");
        symbolTable.addLev();
        getNextWord();  // (
        if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
            getNextWord();
        }
        Block();
        shared_ptr<Function> funcPtr = symbolTable.getFunc(name);
        errorManager.assert(returnType == "void" || (returnType=="int" && funcPtr->haveReturn()),getPreLine(),"g");
        symbolTable.deleteLev();
        procedureManager.addInformation("<MainFuncDef>");
        return (true);
    }

    tuple<bool,string> FuncType() {
        string funcType = getNextWord();
        procedureManager.addInformation("<FuncType>");
        return {true,funcType};
    }

    tuple<bool> FuncFParams() {
        FuncFParam();
        while (isTopWord(",")) {
            getNextWord();  // ,
            FuncFParam();
        }
        procedureManager.addInformation("<FuncFParams>");
        return {true};
    }

    tuple<bool> FuncFParam() {
        getNextWord();  // 'int'
        string identName = getNextWord();
        int line = getPreLine();
        vector<int> dims;
        if (isTopWord("[")) {
            getNextWord();  // [
            if(errorManager.assert(isTopWord("]"),getPreLine(),"k")){
                getNextWord();
            }
            dims.emplace_back(-1);
            while (isTopWord("[")) {
                getNextWord();  // [
                dims.emplace_back(get<1>(ConstExp()));
                if(errorManager.assert(isTopWord("]"),getPreLine(),"k")){
                    getNextWord();
                }
            }
        }
        errorManager.assert(symbolTable.addPara(identName,dims),line,"b");
        procedureManager.addInformation("<FuncFParam>");
        return (true);
    }

    tuple<bool> Block() {
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
        procedureManager.addInformation("<Block>");
        return (true);
    }

    tuple<bool> Stmt() {
        if (isTopWord(";")) {
            if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                getNextWord();
            }
        } else if (isTopWord("{")) {
            symbolTable.addLev();
            Block();
            symbolTable.deleteLev();
        } else if (isTopWord("if")) {
            getNextWord();  // if
            getNextWord();  // (
            Cond();
            if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
                getNextWord();
            }
            Stmt();
            if (isTopWord("else")) {
                getNextWord();  // else
                Stmt();
            }
        } else if (isTopWord("while")) {
            procedureManager.push("while");
            getNextWord();  // while
            getNextWord();  // (
            Cond();
            if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
                getNextWord();
            }
            Stmt();
            procedureManager.pop();
        } else if (isTopWord("break")) {
            getNextWord();  // break
            errorManager.assert(procedureManager.search("while"),getPreLine(),"m");
            if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                getNextWord();
            }
        } else if (isTopWord("continue")) {
            getNextWord();  // continue
            errorManager.assert(procedureManager.search("while"),getPreLine(),"m");
            if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                getNextWord();
            }
        } else if (isTopWord("return")) {
            getNextWord();  // return
            shared_ptr<Function> funcPtr = symbolTable.getFunc();  // assert funcPtr != nullPtr
            if (isTopWord("(") || isTopType("INTCON") || isTopWord("+") || isTopWord("-") || isTopWord("!")) {
                errorManager.assert(funcPtr->setReturn(),getPreLine(),"f");
                Exp();
                if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                    getNextWord();
                }
            } else if (isTopType("IDENFR")) { // 可能是遗漏了分号，且紧接着的下一个语句是左值表达式。
                setMemory();
                LVal();
                bool isError = isTopWord("=");
                returnCommon();
                if (isError) {
                    // 缺少了一个分号，这里应当是下一条语句。
                    funcPtr->setVoidReturn();
                } else {
                    errorManager.assert(funcPtr->setReturn(),getPreLine(),"f");
                    Exp();
                    if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                        getNextWord();
                    }
                }
            } else {
                funcPtr->setVoidReturn();
                if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                    getNextWord();
                }
            }
        } else if (isTopWord("printf")) {
            getNextWord();  // printf
            int printf_line = getPreLine();
            getNextWord();  // (
            string formatString = getNextWord();
            int format_line = getPreLine();
            int count = 0;
            bool isError = false;
            for(int i = 1;i<formatString.size()-1;i++){
                char ch = formatString[i];
                if((32<=ch&&ch<=33) || (40<=ch&&ch<=126)){
                    if(ch=='\\'){
                        if(formatString[i+1]!='n'){
                            isError = true;
                        }else{
                            i++;  // 那个n不用再判断了
                        }
                    }
                }else{
                    if(ch=='%'){
                        if(formatString[i+1]=='d'){
                            count ++ ;
                            i ++;
                        }else{
                            isError = true;
                        }
                    }else{
                        isError = true;
                    }
                }
            }
            // 检查到这里
            int realCount = 0;
            while (isTopWord(",")) {
                getNextWord();  // ,
                Exp();
                realCount ++;
            }
            errorManager.assert(realCount==count,printf_line,"l");
            errorManager.assert(!isError,format_line,"a");
            if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
                getNextWord();
            }
            if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                getNextWord();
            }
        } else { // Exp 或者左值赋值式
            if (isTopType("IDENFR")) {
                setMemory();
                LVal();
                bool isLVal = isTopWord("=");
                returnCommon();
                if (isLVal) {
                    shared_ptr<LValExpression> lValueExpression = get<1>(LVal());
                    //  认为LVal中不会出现换行。
                    if(lValueExpression!= nullptr){  // 确定能找到这个左值才会检查它是否为常量。
                        errorManager.assert(!lValueExpression->isConst(),getPreLine(),"h");
                    }
                    getNextWord();  // =
                    if (isTopWord("getint")) {
                        getNextWord();  // getint
                        getNextWord();  // (
                        if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
                            getNextWord();
                        }
                    } else {
                        Exp();
                    }
                } else {
                    Exp();
                }
                if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                    getNextWord();
                }
            } else {
                Exp();
                if(errorManager.assert(isTopWord(";"),getPreLine(),"i")){
                    getNextWord();
                }
            }
        }
        procedureManager.addInformation("<Stmt>");
        return (true);
    }

    tuple<bool,shared_ptr<Expression> > Exp() {
        shared_ptr<AddExpression> addPtr = get<1>(AddExp());
        procedureManager.addInformation("<Exp>");
        return {true,make_shared<Expression>(addPtr)};
    }

    tuple<bool> Cond() {
        LOrExp();
        procedureManager.addInformation("<Cond>");
        return (true);
    }

    tuple<bool,shared_ptr<LValExpression>> LVal() {
        string name = getNextWord(); // IdentName
        shared_ptr<LValue> lValPtr = symbolTable.getLVal(name);
        shared_ptr<LValExpression> lValExpression = nullptr;
        if(errorManager.assert(lValPtr!= nullptr,getPreLine(),"c")){
            lValExpression = make_shared<LValExpression>(lValPtr);
        }
        while (isTopWord("[")) {
            getNextWord();  // [
            shared_ptr<Expression> exp = get<1>(Exp());
            if(lValExpression!= nullptr){
                lValExpression->addInsight(exp);
            }
            if(errorManager.assert(isTopWord("]"),getPreLine(),"k")){
                getNextWord();
            }
        }
        procedureManager.addInformation("<LVal>");
        return {true,lValExpression};
    }

    tuple<bool,shared_ptr<PrimaryExpression> > PrimaryExp() {
        shared_ptr<PrimaryExpression> primaryExpressionPtr;  // TODO
        if (isTopWord("(")) {
            getNextWord();  // (
            primaryExpressionPtr = make_shared<PrimaryExpression>(get<1>(Exp()));
            if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
                getNextWord();
            }
        } else if (isTopType("IDENFR")) {
            primaryExpressionPtr = make_shared<PrimaryExpression>(get<1>(LVal()));
        } else {
            primaryExpressionPtr = make_shared<PrimaryExpression>(get<1>(Number()));
        }
        procedureManager.addInformation("<PrimaryExp>");
        return {true,primaryExpressionPtr};
    }

    tuple<bool,shared_ptr<NumberExpression> > Number() {
        string word = getNextWord();
        procedureManager.addInformation("<Number>");
        return {true,make_shared<NumberExpression>(word)};
    }

    tuple<bool,shared_ptr<UnaryExpression> > UnaryExp() {  // TODO
        shared_ptr<UnaryExpression> unaryExpressionPtr ;
        if (isTopWord("+") || isTopWord("-") || isTopWord("!")) {
            string op = get<1>(UnaryOp());
            unaryExpressionPtr = make_shared<UnaryExpression>(op,get<1>(UnaryExp()));
        } else if (isTopType("IDENFR") && isFarWord("(", 1)) {  // 函数调用的情况
            string identName = getNextWord();
            int func_line = getPreLine();
            shared_ptr<Function> funcPtr = symbolTable.getFunc(identName);
            bool findFunction = identName!="main" && funcPtr!= nullptr;  // 是否有这个函数。不能调用main函数，而且要调用的函数必须存在。
            errorManager.assert(findFunction,func_line,"c");
            shared_ptr<FunctionCall> functionCall = nullptr;
            if(findFunction){
                functionCall = make_shared<FunctionCall>(funcPtr);
            }
            getNextWord();  // (
            // 判断到底有没有函数参数。
            if (isTopWord("(") || isTopType("IDENFR") || isTopType("INTCON") || isTopWord("+") ||
                isTopWord("-")) {  // First(Exp)，但是普通表达式不含感叹号
                FuncRParams(functionCall);  // 这里需要结合行的信息判断
            }
            if(errorManager.assert(isTopWord(")"),getPreLine(),"j")){
                getNextWord();
            }
            if(functionCall!= nullptr){  // 只有的确找到了这个函数，才会进行参数检查
                if(errorManager.assert(functionCall->isParamNumRight(),func_line,"d")){  // 参数个数正确后，才会检查参数类型是否正确。
                    errorManager.assert(functionCall->isParamTypeRight(),func_line,"e");
                }
            }
            unaryExpressionPtr = make_shared<UnaryExpression>(functionCall);
        } else {
            unaryExpressionPtr = make_shared<UnaryExpression>(get<1>(PrimaryExp()));
        }
        procedureManager.addInformation("<UnaryExp>");
        return {true,unaryExpressionPtr};
    }

    tuple<bool,string> UnaryOp() {
        string op = getNextWord();  // + | - | !
        procedureManager.addInformation("<UnaryOp>");
        return {true,op};
    }

    tuple<bool> FuncRParams(shared_ptr<FunctionCall> functionCall) {
        shared_ptr<Expression> expression = get<1>(Exp());
        if(functionCall!= nullptr){
            functionCall->addParam(expression);
        }
        while (isTopWord(",")) {  // TODO 这个版本代码可能无法兼容错误处理的情况
            getNextWord();  // ,
            expression = get<1>(Exp());
            if(functionCall!= nullptr){
                functionCall->addParam(expression);
            }
        }
        procedureManager.addInformation("<FuncRParams>");
        return {true};
    }

    tuple<bool,shared_ptr<MulExpression> > MulExp() {
        shared_ptr<MulExpression> mulPtr = make_shared<MulExpression>(get<1>(UnaryExp()));
        procedureManager.addInformation("<MulExp>");
        while (isTopWord("*") || isTopWord("/") || isTopWord("%")) {
            string opera = getNextWord();  // * | / | %
            mulPtr->addUnaryExpression(get<1>(UnaryExp()),opera);
            procedureManager.addInformation("<MulExp>");
        }
        return {true, mulPtr};
    }

    tuple<bool,shared_ptr<AddExpression> > AddExp() {
        shared_ptr<AddExpression> addPtr = make_shared<AddExpression>(get<1>(MulExp()));
        procedureManager.addInformation("<AddExp>");
        while (isTopWord("+") || isTopWord("-")) {
            string opera = getNextWord();  // + | -
            addPtr->addMulExpression(get<1>(MulExp()),opera);
            procedureManager.addInformation("<AddExp>");
        }
        return {true,addPtr};
    }

    tuple<bool> RelExp() {
        AddExp();
        procedureManager.addInformation("<RelExp>");
        while (isTopWord("<") || isTopWord(">") || isTopWord("<=") || isTopWord(">=")) {
            getNextWord();  // < | > | <= | >=
            AddExp();
            procedureManager.addInformation("<RelExp>");
        }
        return (true);
    }

    tuple<bool> EqExp() {
        RelExp();
        procedureManager.addInformation("<EqExp>");
        while (isTopWord("==") || isTopWord("!=")) {
            getNextWord();  //  == | !=
            RelExp();
            procedureManager.addInformation("<EqExp>");
        }
        return (true);
    }

    tuple<bool> LAndExp() {
        EqExp();
        procedureManager.addInformation("<LAndExp>");
        while (isTopWord("&&")) {
            getNextWord();  // &&
            EqExp();
            procedureManager.addInformation("<LAndExp>");
        }
        return (true);
    }

    tuple<bool> LOrExp() {
        LAndExp();
        procedureManager.addInformation("<LOrExp>");
        while (isTopWord("||")) {
            getNextWord();  // ||
            LAndExp();
            procedureManager.addInformation("<LOrExp>");
        }
        return (true);
    }

    tuple<bool,int> ConstExp() {

        shared_ptr<AddExpression> addPtr = get<1>(AddExp());
        int addExpResult = -1;  // -1表示不能确定具体值
        if(addPtr->isFind()){
            if(addPtr->isConst()){
                if(addPtr->isHaveValue()){
                    addExpResult = addPtr->getValue();
                }else{
                    //  尽管是常量表达式，却无法求值（比如数组的情况）
                }
            }else{
                // 所谓的常量表达式里含有非常量
            }
        }else{
            // 所谓的常量表达式里有未知标识符
        }
        procedureManager.addInformation("<ConstExp>");
        return {true, addExpResult};
    }

};



#endif //COMPILER_ERRORDETECTOR_H
