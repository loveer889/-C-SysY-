#ifndef COMPILER_SYMBOLTABLE_H
#define COMPILER_SYMBOLTABLE_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include "WordAnalyser.h"
#include <fstream>
#include <memory>


class Symbol {
protected:
    string name;
    int index = -1;
    int sectionIndex = -1;  // 只有这个Symbol是函数的时候才有意义
public:

    void setSectionIndex(int sectionIndex) {
        this->sectionIndex = sectionIndex;
    }

    int getSectionIndex() {
        return sectionIndex;
    }

    string getName() {
        return name;
    }

    void setIndex(int index) {
        this->index = index;
    }

    int getIndex() {
        return this->index;
    }

    virtual bool isConst() {
        return false;  // 默认是非常量
    }

    virtual bool isArray() {
        return false;  // 默认是非数组
    }

    virtual bool isFunction() {
        return false;  // 默认是非函数
    }

    virtual bool isLVal() {
        return false;  // 默认也不是左值
    }
};

class LValue : public Symbol {
public:
    bool isLVal() {
        return true;
    }

    virtual int getDim() = 0;

    virtual int getValue() {
        return 0;
    }

    virtual vector<int> getDims() = 0;
};

class UnArray : public LValue {
public:
    bool isArray() {
        return false;
    }

    int getDim() {
        return 0;
    }

    vector<int> getDims() override {
        return {};
    }
};


class Array : public LValue {
protected:
    vector<int> dims;
public:
    int getDim() {
        return dims.size();
    }

    bool isArray() {
        return true;
    }

    vector<int> getDims() override {
        return dims;
    }

    virtual int getValue(vector<int> &insights) {
        return 0;
    }
};

class Const : public UnArray {
private:
    int value;
public:
    Const(string &name, int value) {
        this->name = name;
        this->value = value;
    }

    int getValue() {
        return value;
    }

    bool isConst() {
        return true;
    }

};

class ConstArray : public Array {
private:
    vector<int> numbers;
public:
    ConstArray(string &name, vector<int> &dims, vector<int> &numbers) {
        this->name = name;
        this->dims = dims;
        this->numbers = numbers;
    }

    /**
     * @return 这是几维数组
     */

    bool isConst() {
        return true;
    }

    int getValue(vector<int> &insights) {
        if (insights.size() == 2) {
            int offset = insights[0] * this->dims[1] + insights[1];
            return numbers[offset];
        } else if (insights.size() == 1) {
            int offset = insights[0];
            return numbers[offset];
        } else {
            return -31;
        }
    }
};

class Var : public UnArray {
public:
    Var(string &name) {
        this->name = name;
    }
};

class VarArray : public Array {
public:
    VarArray(string &name, vector<int> &dims) {
        this->name = name;
        this->dims = dims;
    }
};

class Para : public LValue {
    vector<int> getDims() = 0;
};

class UnArrayPara : public Para {
public:
    UnArrayPara(string &name) {
        this->name = name;
    }

    bool isArray() {
        return false;
    }

    int getDim() {
        return 0;
    }

    vector<int> getDims() {
        return {};
    }
};

class ArrayPara : public Para {
private:
    vector<int> dims;  // 存储各维的长度(但是第0个维度的值是无用的)。
public:
    ArrayPara(string &name, vector<int> &dims) {
        this->name = name;
        this->dims = dims;
    }

    int getDim() {
        return (int) dims.size();
    }

    bool isArray() {
        return true;
    }

    vector<int> getDims() override {
        return dims;
    }
};


class Function : public Symbol {
private:
    string returnType;
    vector<shared_ptr < Para> >
    paras;
    int returnState = 0;
public:

    Function(string &name, string &returnType) {
        this->name = name;
        this->returnType = returnType;
    }

    void addPara(string &name, vector<int> &dims) {
        if (dims.empty()) {

            paras.emplace_back(make_shared<UnArrayPara>(name));
        } else {
            paras.emplace_back(make_shared<ArrayPara>(name, dims));
        }
    }

    void addPara(shared_ptr <Para> &para) {
        paras.emplace_back(para);
    }

    string getReturnType() {
        return this->returnType;
    }

    /**
     * 给这个函数添加一个返回值，返回这种做法是否合理
     * @return
     */
    bool setReturn() {
        this->returnState = 1;
        return this->returnType != "void";
    }

    void setVoidReturn() {
        this->returnState = 2;
    }

    bool haveReturn() {
        return this->returnState != 0;
    }

    bool isFunction() {
        return true;
    }

    int getParaNum() {
        return paras.size();
    }

    vector<shared_ptr < Para> >

    getParams() {
        return this->paras;
    }

};

class SymbolTable {
private:
    int symbol_lev = 1;


    map<int, vector<shared_ptr < Symbol> > >
    lev2symbols;  // 各层级的symbol。
    /**
     * 检查当前作用域是否有重名的 常量或变量或函数参数
     * 有可能引发重复的：
     *      当前作用域的常量或变量或函数参数
     *
     */
    bool isRepeatLVal(string &targetName) {
        if (lev2symbols.count(symbol_lev)) {
            for (auto &symbol:lev2symbols[symbol_lev]) {
                if (symbol->isLVal()) {
                    if (symbol->getName() == targetName) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool isRepeatFunction(string &funcName) {
        if (lev2symbols.count(symbol_lev)) {
            for (auto &symbol:lev2symbols[symbol_lev]) {
                if (symbol->isFunction()) {
                    if (symbol->getName() == funcName) {
                        return true;
                    }
                }
            }
        }
        return false;
    }


public:
    shared_ptr <Symbol> lastPtr = nullptr;

    SymbolTable() {
        this->lev2symbols[symbol_lev] = vector<shared_ptr < Symbol>>
        ();
    }

    bool addConst(string &name, vector<int> &dims, vector<int> &numbers) {
        bool isRepeat = isRepeatLVal(name);
        if (dims.empty()) {
            shared_ptr <Const> ptr = make_shared<Const>(name, numbers[0]);
            lastPtr = ptr;
            lev2symbols[symbol_lev].emplace_back(ptr);
        } else {
            shared_ptr <ConstArray> ptr = make_shared<ConstArray>(name, dims, numbers);
            lastPtr = ptr;
            lev2symbols[symbol_lev].emplace_back(ptr);
        }

        return !isRepeat;  // 返回是否不重复
    }

    bool addVar(string &name, vector<int> &dims) {
        bool isRepeat = isRepeatLVal(name);
        if (dims.empty()) {
            shared_ptr <Var> ptr = make_shared<Var>(name);
            lastPtr = ptr;
            lev2symbols[symbol_lev].emplace_back(ptr);
        } else {
            shared_ptr <VarArray> ptr = make_shared<VarArray>(name, dims);
            lastPtr = ptr;
            lev2symbols[symbol_lev].emplace_back(ptr);
        }
        return !isRepeat;  // 添加成功。
    }

    bool addFunction(string &name, string &returnType) {
        bool isRepeat = isRepeatFunction(name);
        shared_ptr <Function> ptr = make_shared<Function>(name, returnType);
        lastPtr = ptr;
        lev2symbols[symbol_lev].emplace_back(ptr);
        return !isRepeat;
    }

    int addLev() {
        this->symbol_lev++;
        lev2symbols[symbol_lev] = vector<shared_ptr < Symbol>>
        ();
        return symbol_lev;
    }

    bool addPara(string &name, vector<int> &dims) {
        bool isRepeat = isRepeatLVal(name);
        shared_ptr <Para> para;
        if (dims.empty()) {
            para = make_shared<UnArrayPara>(name);
        } else {
            para = make_shared<ArrayPara>(name, dims);
        }

        this->lev2symbols[symbol_lev].emplace_back(para);
        lastPtr = para;

        vector<shared_ptr < Symbol> > &preLevSymbols = lev2symbols[symbol_lev - 1];
        shared_ptr <Symbol> symbol = preLevSymbols[preLevSymbols.size() - 1];
        // assert : 当添加一个参数时，一定有上一层级的符号表，而且上一层级符号表最后一定是这个参数所在的函数
        shared_ptr <Function> function = static_pointer_cast<Function>(symbol);
        function->addPara(para);
        return !isRepeat;
    }

    int deleteLev() {
        this->lev2symbols.erase(symbol_lev);
        symbol_lev--;
        return symbol_lev;
    }

    shared_ptr <LValue> getLVal(string &targetName) {
        for (int targetLev = symbol_lev; targetLev > 0; targetLev--) {
            vector<shared_ptr < Symbol> > &symbols = lev2symbols[targetLev];
            for (int index = symbols.size() - 1; index >= 0; index--) {
                bool isL = symbols[index]->isLVal();
                string theName = symbols[index]->getName();
                if (symbols[index]->isLVal() && symbols[index]->getName() == targetName) {
                    return static_pointer_cast<LValue>(symbols[index]);
                }
            }
        }
        return nullptr;
    }

    /**
     * 按照名字寻找一个函数
     */
    shared_ptr <Function> getFunc(string &name) {
        for (int targetLev = symbol_lev; targetLev > 0; targetLev--) {
            vector<shared_ptr < Symbol> > &symbols = lev2symbols[targetLev];
            for (int index = symbols.size() - 1; index >= 0; index--) {
                if (symbols[index]->isFunction() && symbols[index]->getName() == name) {
                    return static_pointer_cast<Function>(symbols[index]);
                }
            }
        }
        return nullptr;
    }

    /**
     * 找到当前标识符所在的最内层函数。
     */
    shared_ptr <Function> getFunc() {
        for (int targetLev = symbol_lev - 1; targetLev > 0; targetLev--) {
            vector<shared_ptr < Symbol> > &symbols = lev2symbols[targetLev];
            for (int index = symbols.size() - 1; index >= 0; index--) {
                if (symbols[index]->isFunction()) {
                    return static_pointer_cast<Function>(symbols[index]);
                }
            }
        }
        return nullptr;
    }


};


#endif //COMPILER_SYMBOLTABLE_H
