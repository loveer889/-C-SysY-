#ifndef COMPILER_EXPRESSION_H
#define COMPILER_EXPRESSION_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include "WordAnalyser.h"
#include <fstream>
#include "ErrorManager.h"
#include "SymbolTable.h"
#include "Stm.h"

using namespace std;


class Calculable {

public:
    /**
     * 涉及到的所有标识符是否都能找到
     */
    virtual bool isFind() = 0;


    /**
     * 前置条件: isFind() == true
     * 是否是常量
     */
    virtual bool isConst() = 0;  // TODO 当前常量也未必能求值，因为只有0维的可以求值，而且函数不能求值

    /**
     * 前置条件: isFind() == true
     * 是否有确定的值
     */
    virtual bool isHaveValue() = 0;

    /**
     * 前置条件：isHaveValue() == true
     * 得到这个东西的固定值。
     */
    virtual int getValue() = 0;
};


class Expression;

class FunctionCall;

class PrimaryExpression;

class LValExpression;

class NumberExpression;

class UnaryExpression : Calculable {
    //UnaryExp → PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    // UnaryExp → {UnaryOp} (PrimaryExp | Ident '(' [FuncRParams] ')' )

private:
    vector<string> unaryOps;
    // 以下的几类中，至多有一个不为nullptr。(可能都为空，即函数调用找不到函数时）。
    shared_ptr<PrimaryExpression> primaryExpression = nullptr;
    shared_ptr<FunctionCall> functionCall = nullptr;
public:
    UnaryExpression(shared_ptr<FunctionCall> functionCall) {
        this->functionCall = functionCall;
    }

    UnaryExpression(shared_ptr<PrimaryExpression> primaryExpression) {
        this->primaryExpression = primaryExpression;
    }

    UnaryExpression(string unaryOp, shared_ptr<UnaryExpression> unaryExpression) {
        this->unaryOps.emplace_back(unaryOp);
        this->unaryOps.insert(this->unaryOps.end(), unaryExpression->unaryOps.begin(), unaryExpression->unaryOps.end());
        this->functionCall = unaryExpression->functionCall;
        this->primaryExpression = unaryExpression->primaryExpression;
    }

    int getDim();

    bool isFind() override;

    bool isConst() override;

    bool isHaveValue() override;

    int getValue() override;

    int getIndex();

    bool isArray();

    shared_ptr<LValExpression> getLVal();

};

class PrimaryExpression : Calculable {
    // PrimaryExpression → '(' Exp ')' | LVal | Number
    // PrimaryExpression → Expression | LValExpression | NumberExpression
private:
    // 至多有一个不为nullptr
    shared_ptr<Expression> expression = nullptr;
    shared_ptr<LValExpression> lValExpression = nullptr;
    shared_ptr<NumberExpression> numberExpression = nullptr;

public:
    PrimaryExpression(shared_ptr<Expression> expression) {
        this->expression = expression;
    }

    PrimaryExpression(shared_ptr<LValExpression> lValExpression) {
        this->lValExpression = lValExpression;
    }

    PrimaryExpression(shared_ptr<NumberExpression> numberExpression) {
        this->numberExpression = numberExpression;
    }

    bool isFind() override;

    int getDim();

    bool isConst() override;

    bool isHaveValue() override;

    int getValue() override;

    bool isArray();

    int getIndex();

    shared_ptr<LValExpression> getLVal() {
        return this->lValExpression;
    }

};

class LValExpression : Calculable {
    // Ident {'[' Exp ']'}
private:
    shared_ptr<LValue> lValue;  // 符号表中的左值，它可能是常量或变量，可能是数字或数组。
    vector<shared_ptr<Expression> > insights;  // 对维度的选取。insights.size()表示降维数
    int offSetIndex = -1;
    int addressIndex = -1;
public:
    LValExpression(shared_ptr<LValue> lValue) {
        this->lValue = lValue;
    }

    int getIndex() {
        return lValue->getIndex();
    }

    //  发现原本是数组而不是数时
    int getOffset();

    int getAddress() {
        if (addressIndex == -1) {
            addressIndex = Stm::calcluateAddress(lValue->getIndex(), getOffset());
        }
        return addressIndex;
    }

    void addInsight(shared_ptr<Expression> insight) {
        this->insights.emplace_back(insight);
    }

    bool isFind() override {
        return true;  // 左值一定是能找到的，因为如果找不到，根本不会创建这个对象。
    }

    vector<int> getOriDims() {
        return lValue->getDims();
    }

    bool isConst();


    int getDim() {  // 实际维度
        int sum_ = lValue->getDim();
        return sum_ - (int) insights.size();
    }

    /**
     * 前置条件:
     *      getDim() >=0
     *      0<= lValue->getDim() <= 2
     *
     * @return
     */
    bool isHaveValue() override {
        return isConst() && getDim() == 0;
    }

    int getValue();

    bool isArray() {
        return getDim() != 0;
    }

    bool isOriArray() {
        return lValue->getDim() != 0;
    }


};

class NumberExpression : Calculable {
private:
    int number;
public:
    NumberExpression(string numberStr) {
        this->number = stoi(numberStr);
    }

    NumberExpression(int number) {
        this->number = number;
    }

    int getDim() {
        return 0;  // 普通数字一定是0维的。
    }

    bool isFind() override {
        return true;
    }

    bool isConst() override {
        return true;
    }

    bool isHaveValue() override {
        return true;
    }

    int getValue() override {
        return this->number;
    }

    int getIndex() {
        return Stm::assignConst(getValue());
    }

};

class FunctionCall : Calculable {
//    Ident '(' [FuncRParams] ')'
private:
    shared_ptr<Function> function;  // 符号表中的函数
    vector<shared_ptr<Expression> > rParams;
public:
    FunctionCall(shared_ptr<Function> function) {
        this->function = function;
    }

    void addParam(shared_ptr<Expression> param) {
        this->rParams.emplace_back(param);
    }

    bool isParamNumRight() {
        return rParams.size() == function->getParaNum();
    }

    /**
     * 前置条件 isParamNumRight == true
     */
    bool isParamTypeRight();

    int getDim() {
        if (function->getReturnType() == "void") {
            return -1;
        } else {
            return 0;
        }
    }

    bool isFind() override;

    bool isConst() override {
        return false;  // 函数肯定不是常量
    }

    bool isHaveValue() override {
        return false;  // TODO 当前认为难以确定一个函数的返回值
    }

    int getValue() override {
        return 0;  // TODO 当前应当调用不到这里。
    }

    int getIndex();


};


class MulExpression : Calculable {
    // MulExp → UnaryExp { ('*' | '/' | '%') UnaryExp }
    // 基础为1
private:
    vector<shared_ptr<UnaryExpression> > unaryExpressions;
    vector<string> operators;
public:
    MulExpression(shared_ptr<UnaryExpression> unaryExpression) {
        unaryExpressions.emplace_back(unaryExpression);
        operators.emplace_back("*");
    }

    void addUnaryExpression(shared_ptr<UnaryExpression> unaryExpression, string opera) {
        unaryExpressions.emplace_back(unaryExpression);
        operators.emplace_back(opera);
    }

    int getDim() {
        int a = unaryExpressions[0]->getDim();
        for (int b = 1; b < unaryExpressions.size(); b++) {
            if (unaryExpressions[b]->getDim() != a) {
                return -1;
            }
        }
        return a;
    }

    bool isFind() override {
        for (auto ptr:unaryExpressions) {
            if (!ptr->isFind()) {
                return false;
            }
        }
        return true;
    }

    bool isConst() override {
        bool flag = true;
        for (auto ptr:unaryExpressions) {
            flag &= ptr->isConst();
        }
        return flag;
    }

    bool isHaveValue() override {
        for (auto ptr:unaryExpressions) {
            if (!ptr->isHaveValue()) {
                return false;
            }
        }
        return true;
    }

    int getValue() override {
        int len = (int) operators.size();
        int result = 1;
        for (int i = 0; i < len; i++) {
            if (operators[i] == "*") {
                result *= unaryExpressions[i]->getValue();
            } else if (operators[i] == "/") {
                result /= unaryExpressions[i]->getValue();
            } else {
                result %= unaryExpressions[i]->getValue();
            }
        }
        return result;
    }

    int getIndex() {
        int index = unaryExpressions[0]->getIndex();
        for (int i = 1; i < unaryExpressions.size(); i++) {
            int nextIndex = unaryExpressions[i]->getIndex();
            if (operators[i] == "*") {
                index = Stm::multi(index, nextIndex);
            } else if (operators[i] == "/") {
                index = Stm::div(index, nextIndex);
            } else {
                index = Stm::mode(index, nextIndex);
            }
        }
        return index;
    }

    bool isArray() {
        if (unaryExpressions.size() > 1) {
            return false;
        } else {
            return unaryExpressions[0]->isArray();
        }
    }

    shared_ptr<LValExpression> getLVal() {
        return unaryExpressions[0]->getLVal();
    }
};

class AddExpression : Calculable {
    // AddExp → MulExp { ('+' | '−') MulExp }
    // 基础为0
private:
    vector<shared_ptr<MulExpression> > mulExpressions;
    vector<string> symbols;
public:
    AddExpression(shared_ptr<MulExpression> firstMulExpression) {
        mulExpressions.emplace_back(firstMulExpression);
        symbols.emplace_back("+");
    }

    void addMulExpression(shared_ptr<MulExpression> mulExpression, string &symbol) {
        mulExpressions.emplace_back(mulExpression);
        symbols.emplace_back(symbol);
    }

    int getDim() {
        int a = mulExpressions[0]->getDim();
        for (int b = 1; b < mulExpressions.size(); b++) {
            if (mulExpressions[b]->getDim() != a) {
                return -1;
            }
        }
        return a;
    }

    bool isFind() override {
        for (auto ptr:mulExpressions) {
            if (!ptr->isFind()) {
                return false;
            }
        }
        return true;
    }

    bool isConst() override {
        bool flag = true;
        for (auto ptr:mulExpressions) {
            flag &= ptr->isConst();
        }
        return flag;
    }

    bool isHaveValue() override {
        for (auto ptr:mulExpressions) {
            if (!ptr->isHaveValue()) {
                return false;
            }
        }
        return true;
    }

    int getValue() override {
        int len = (int) symbols.size();
        int sum = 0;
        for (int i = 0; i < len; i++) {
            if (symbols[i] == "+") {
                sum += mulExpressions[i]->getValue();
            } else if (symbols[i] == "-") {
                sum -= mulExpressions[i]->getValue();
            }
        }
        return sum;
    }

    int getIndex() {
        int index = mulExpressions[0]->getIndex();
        for (int i = 1; i < mulExpressions.size(); i++) {
            int nextIndex = mulExpressions[i]->getIndex();
            if (symbols[i] == "+") {
                index = Stm::adder(index, nextIndex);
            } else {
                index = Stm::subber(index, nextIndex);
            }
        }
        return index;
    }

    bool isArray() {
        if (mulExpressions.size() > 1) {
            return false;
        } else {
            return mulExpressions[0]->isArray();
        }
    }

    shared_ptr<LValExpression> getLVal() {
        return mulExpressions[0]->getLVal();
    }

};

class Expression : Calculable {
    // Exp → AddExp
private:
    shared_ptr<AddExpression> addExpression;
    int index = -1;
public:
    Expression(shared_ptr<AddExpression> &addExpression) {
        this->addExpression = addExpression;
    }

    int getIndex() {
        if (index == -1) {
            index = addExpression->getIndex();
        }
        return index;
    }

    /**
     * 返回维度。
     * 一般都是0维的（表达式、数字等）
     * 如果是数组，则是一维或二维等。
     */
    int getDim() {
        return addExpression->getDim();
    }

    bool isFind() override {
        return addExpression->isFind();
    }

    bool isConst() override {
        return addExpression->isConst();
    }

    bool isHaveValue() override {
        return addExpression->isHaveValue();
    }

    int getValue() override {
        return addExpression->getValue();
    }

    bool isArray() {
        return addExpression->isArray();
    }

    shared_ptr<LValExpression> getLVal() {
        return addExpression->getLVal();
    }

};

int UnaryExpression::getDim() {
    if (this->primaryExpression != nullptr) {
        return primaryExpression->getDim();
    } else if (this->functionCall != nullptr) {
        return functionCall->getDim();
    } else {
        return 0;  // 找不到函数
    }
}

int PrimaryExpression::getDim() {
    if (expression != nullptr) {
        return expression->getDim();
    } else if (this->lValExpression != nullptr) {
        return lValExpression->getDim();
    } else if (this->numberExpression != nullptr) {
        return numberExpression->getDim();
    } else {
        return -1;  // 参数是未定义的标识符。
    }
}

/**
 * 当且仅当有至少一个参数维度不符合时，认为参数类型错误。
 * @return
 */
bool FunctionCall::isParamTypeRight() {
    vector<shared_ptr<Para> > paras = function->getParams();
    for (int i = 0; i < rParams.size(); i++) {
        int a = rParams[i]->getDim();
        int b = paras[i]->getDim();
        if (a != b) {
            return false;
        }
    }
    return true;
}

bool UnaryExpression::isFind() {
    if (primaryExpression == nullptr && functionCall == nullptr) {
        return false;
    } else if (primaryExpression != nullptr) {
        return primaryExpression->isFind();
    } else {
        return functionCall->isFind();
    }
}

bool FunctionCall::isFind() {
    //  函数名一定已经从符号表中找到了，否则不会创建这个FunctionCall对象
    for (auto &ptr:rParams) {
        if (!ptr->isFind()) {
            return false;
        }
    }
    return true;
}

bool PrimaryExpression::isFind() {
    if (expression == nullptr && lValExpression == nullptr && numberExpression == nullptr) {
        return false;
    }
    if (expression != nullptr) {
        return expression->isFind();
    } else if (lValExpression != nullptr) {
        return lValExpression->isFind();
    } else {
        return numberExpression->isFind();
    }
}

bool UnaryExpression::isConst() {
    if (primaryExpression != nullptr) {
        return primaryExpression->isConst();
    } else {
        return false;  // 函数调用不是常量，因为它的Ident是函数标识符，不是常量标识符。
    }
}

//
bool PrimaryExpression::isConst() {
    if (expression != nullptr) {
        return expression->isConst();
    } else if (lValExpression != nullptr) {
        return lValExpression->isConst();
    } else {
        return numberExpression->isConst();
    }
}

bool UnaryExpression::isHaveValue() {
    if (primaryExpression != nullptr) {
        return primaryExpression->isHaveValue();
    } else {
        return functionCall->isHaveValue();
    }
}

bool PrimaryExpression::isHaveValue() {
    if (expression != nullptr) {
        return expression->isHaveValue();
    } else if (lValExpression != nullptr) {
        return lValExpression->isHaveValue();
    } else {
        return numberExpression->isHaveValue();
    }
}

int UnaryExpression::getValue() {
    int result;
    if (primaryExpression != nullptr) {
        result = primaryExpression->getValue();
    } else {
        result = functionCall->getValue();
    }
    for (int i = unaryOps.size() - 1; i >= 0; i--) {
        if (unaryOps[i] == "+") {

        } else if (unaryOps[i] == "-") {
            result = -1 * result;
        } else if (unaryOps[i] == "!") {
            result = !result;
        }
    }
    return result;
}

int PrimaryExpression::getValue() {
    if (expression != nullptr) {
        return expression->getValue();
    } else if (lValExpression != nullptr) {
        return lValExpression->getValue();
    } else {
        return numberExpression->getValue();
    }
}

bool LValExpression::isConst() {
    for (auto ptr:insights) {
        if (!ptr->isConst()) {
            return false;
        }
    }
    return lValue->isConst();
}

int LValExpression::getValue() {
    if (lValue->isArray()) {
        shared_ptr<Array> array = static_pointer_cast<Array>(lValue);
        vector<int> newInsights;
        for (auto ptr:insights) {
            newInsights.emplace_back(ptr->getValue());
        }
        return array->getValue(newInsights);
    } else {
        return lValue->getValue();
    }
};

int UnaryExpression::getIndex() {
    int index = -1;
    if (primaryExpression != nullptr) {
        index = primaryExpression->getIndex();
    } else {
        index = functionCall->getIndex();
    }
    for (int i = unaryOps.size() - 1; i >= 0; i--) {
        string op = unaryOps[i];
        if (op == "-") {
            index = Stm::minusSelf(index);
        } else if (op == "!") {
            index = Stm::notSelf(index);
        } else {   // + 不用做

        }
    }
    return index;
}

bool UnaryExpression::isArray() {
    if (primaryExpression != nullptr) {
        return primaryExpression->isArray();
    } else {
        return false;  // 函数调用不可能返回数组
    }
}

bool PrimaryExpression::isArray() {
    if (expression != nullptr) {
        return false;
    } else if (lValExpression != nullptr) {
        return lValExpression->isArray();
    } else {
        return false;
    }
}

int PrimaryExpression::getIndex() {
    if (expression != nullptr) {
        return expression->getIndex();
    } else if (lValExpression != nullptr) {
        if (lValExpression->isOriArray()) {
            return Stm::loadArrayValue(lValExpression->getIndex(), lValExpression->getOffset());
        } else {
            return lValExpression->getIndex();
        }
    } else {
        return numberExpression->getIndex();
    }
}

int FunctionCall::getIndex() {
    int returnValueIndex = Stm::getIndex();  // 函数的返回结果弄到这个变量里。
    for (auto ptr : rParams) {    //  先把所有的参数都求出来作为当前内存区域的临时变量
        if (ptr->isArray()) {
            shared_ptr<LValExpression> lval = ptr->getLVal();
            lval->getAddress();  // 传递数组实际上是传递地址。
        } else {
            ptr->getIndex();
        }
    }
    for (auto ptr : rParams) {
        if (ptr->isArray()) {
            shared_ptr<LValExpression> lval = ptr->getLVal();
            Stm::pushUnArrayPara(lval->getAddress());
        } else {
            Stm::pushUnArrayPara(ptr->getIndex());
        }
    }
    if (function->getReturnType() == "void") {
        Stm::callFunction(function->getIndex(), -1);
        return -1;
    } else {
        Stm::callFunction(function->getIndex(), returnValueIndex);
        return returnValueIndex;
    }
}

shared_ptr<LValExpression> UnaryExpression::getLVal() {
    return primaryExpression->getLVal();
}


int LValExpression::getOffset() {
    if (offSetIndex == -1) {
        vector<int> insightIndexs;
        for (auto ptr : insights) {
            insightIndexs.emplace_back(ptr->getIndex());
        }
        offSetIndex = Stm::calculateOffset(getOriDims(), insightIndexs);
    }
    return offSetIndex;
}

#endif //COMPILER_EXPRESSION_H
