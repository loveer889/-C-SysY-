#ifndef COMPILER_DEFINIATIONANALYSE_H
#define COMPILER_DEFINIATIONANALYSE_H

#include "Stm.h"
#include "Optimizer.h"


using namespace std;

/**
 * 到达定义分析。对于任意一条语句（使用的）任意一个变量，应当能够知道它是在哪里定义的，是否有确定值，确定值是多少
 */

/**
 * 某次定义。
 */
class DefOne {

    shared_ptr<BasicBlock> block_ptr;   // 对应的基本块指针
    shared_ptr<Stm> stm_ptr;            // 对应的语句指针
    int valueIndex;                     // 给哪个变量进行定义
    bool hasValue;
    int value;


public:
    DefOne(shared_ptr<BasicBlock> &block_ptr, shared_ptr<Stm> &stm_ptr, int valueIndex,
           bool hasValue = false,
           int value = -1) {
        this->block_ptr = block_ptr;
        this->stm_ptr = stm_ptr;
        this->valueIndex = valueIndex;
        this->hasValue = hasValue;
        this->value = value;
    }

    int getValueIndex() {
        return valueIndex;
    }

    int getCertainValue() {
        return value;
    }

    bool hasCertainValue() {
        return hasValue;
    }

    /**
     * 在分析的过程中，可能能够获知某次定义给目标变量赋的确定值。调用这个方法进行设置。
     * 返回是否发生变化。
     */
    bool setCertainValue(int certainValue) {
        if (hasValue) {
            if (certainValue != value) {
                cout << "error! 两次定义结果不同" << endl;
            }
            return false;
        }
        hasValue = true;
        value = certainValue;
        return true;
    }

    /**
     * 试图找到对某个变量的赋值，返回是否找到。
     */
    bool findCertainValueInSet(int valueIndex,
                               shared_ptr<Stm> stm_ptr,
                               map<tuple<int, shared_ptr<Stm> >, set<shared_ptr<DefOne> >> &valueDefPlaces,
                               int &resultValue,
                               set<shared_ptr<Stm> > &newConstStms) {
        for (auto &ptr:newConstStms) {
            if (ptr->result == valueIndex) {
                resultValue = ptr->numberA;
                return true;
            }
        }

        if (valueDefPlaces.count({valueIndex, stm_ptr})) {
            bool has = false;
            for (auto &hereDef:valueDefPlaces[{valueIndex, stm_ptr}]) {
                if (!hereDef->hasCertainValue()) {
                    return false;
                } else if (!has) {
                    has = true;
                    resultValue = hereDef->getCertainValue();
                } else {
                    if (resultValue != hereDef->getCertainValue()) {
                        return false;
                    }
                }
            }
            return true;
        } else {
            cout << "没有找到变量" << valueIndex << "的任何定义点" << endl;
            return false;
        }
    }

    /**
     * 尽量求出这个赋值语句赋值的结果。返回是否能确定。
     */
    bool tryFindCertainValue(
            map<tuple<int, shared_ptr<Stm> >, set<shared_ptr<DefOne> >> &valueDefPlaces,
            map<int, vector<int> > &constArrays,
            set<shared_ptr<Stm> > &newConstStms

    ) {
        shared_ptr<Stm> ptr = stm_ptr;


        if (ptr->type == "declaration") {
            if (ptr->operation == "ConstUnarray") {
                return this->setCertainValue(ptr->numberA);
            }
        } else if (ptr->type == "assign") {
            if (ptr->operation == "VarUnarrayInitial") {
                int value;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, value, newConstStms)) {
                    int constIndex;
                    newConstStms.insert(Stm::makeConst(value, constIndex));
                    this->setCertainValue(value);
                    ptr->setNumberA(constIndex);
                    return true;
                }
            } else if (ptr->operation == "Unarray") {
                int value;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, value, newConstStms)) {
                    int constIndex;
                    newConstStms.insert(Stm::makeConst(value, constIndex));
                    this->setCertainValue(value);
                    ptr->setNumberA(constIndex);
                    return true;
                }
            }
        } else if (ptr->type == "getReturnValue") {
            return false;  // 认为函数的返回值一定是未知的。
        } else if (ptr->type == "getInt") {
            return false;  // 认为getInt的值一定是未知的。
        } else if (ptr->type == "mul" || ptr->type == "add" || ptr->type == "sub" || ptr->type == "div" ||
                   ptr->type == "mode") {
            if (ptr->type == "mul") {
                if (ptr->operation == "Const") {
                    int constValue = ptr->numberB;
                    int valueA;
                    if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                        int constIndex;
                        newConstStms.insert(Stm::makeConst(valueA, constIndex));
                        this->setCertainValue(valueA * constValue);
                        ptr->setNumberA(constIndex);

                        return true;
                    }
                } else if (ptr->operation == "") {
                    int valueA;
                    int valueB;
                    if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                        && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                        int constIndexA;
                        newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                        ptr->setNumberA(constIndexA);

                        int constIndexB;
                        newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                        ptr->setNumberB(constIndexB);

                        this->setCertainValue(valueA * valueB);
                        return true;
                    } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                        int constIndexA;
                        newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                        ptr->setNumberA(constIndexA);
                    } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                        int constIndexB;
                        newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                        ptr->setNumberB(constIndexB);
                    }
                }
            } else if (ptr->type == "add") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    this->setCertainValue(valueA + valueB);
                    return true;
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            } else if (ptr->type == "sub") {
                if (ptr->operation == "ConstSub") {
                    int valueA = ptr->numberA;
                    int valueB;
                    if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                        int constIndexB;
                        newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                        ptr->setNumberB(constIndexB);

                        this->setCertainValue(valueA - valueB);
                        return true;
                    }
                } else if (ptr->operation == "") {
                    int valueA;
                    int valueB;
                    if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                        && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                        int constIndexA;
                        newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                        ptr->setNumberA(constIndexA);

                        int constIndexB;
                        newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                        ptr->setNumberB(constIndexB);

                        this->setCertainValue(valueA - valueB);
                        return true;
                    } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                        int constIndexA;
                        newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                        ptr->setNumberA(constIndexA);
                    } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                        int constIndexB;
                        newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                        ptr->setNumberB(constIndexB);
                    }
                }
            } else if (ptr->type == "div") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    this->setCertainValue(valueA / valueB);
                    return true;
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            } else if (ptr->type == "mode") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    this->setCertainValue(valueA % valueB);
                    return true;
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            }
        } else if (ptr->type == "not" || ptr->type == "lt" || ptr->type == "le" || ptr->type == "equalCheck" ||
                   ptr->type == "notEqualCheck" || ptr->type == "gt" || ptr->type == "ge") {
            if (ptr->type == "not") {
                int valueA;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                    if (valueA == 0) {
                        this->setCertainValue(1);
                        return true;
                    } else {
                        this->setCertainValue(0);
                        return true;
                    }
                }
            } else if (ptr->type == "equalCheck") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    if (valueA == valueB) {
                        this->setCertainValue(1);
                        return true;
                    } else {
                        this->setCertainValue(0);
                        return true;
                    }
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            } else if (ptr->type == "notEqualCheck") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    if (valueA != valueB) {
                        this->setCertainValue(1);
                        return true;
                    } else {
                        this->setCertainValue(0);
                        return true;
                    }
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            } else if (ptr->type == "gt") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    if (valueA > valueB) {
                        this->setCertainValue(1);
                        return true;
                    } else {
                        this->setCertainValue(0);
                        return true;
                    }
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            } else if (ptr->type == "ge") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    if (valueA >= valueB) {
                        this->setCertainValue(1);
                        return true;
                    } else {
                        this->setCertainValue(0);
                        return true;
                    }
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            } else if (ptr->type == "lt") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {

                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);

                    if (valueA < valueB) {
                        this->setCertainValue(1);
                        return true;
                    } else {
                        this->setCertainValue(0);
                        return true;
                    }
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            } else if (ptr->type == "le") {
                int valueA;
                int valueB;
                if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)
                    && findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);

                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                    if (valueA <= valueB) {
                        this->setCertainValue(1);
                        return true;
                    } else {
                        this->setCertainValue(0);
                        return true;
                    }
                } else if (findCertainValueInSet(ptr->numberA, ptr, valueDefPlaces, valueA, newConstStms)) {
                    int constIndexA;
                    newConstStms.insert(Stm::makeConst(valueA, constIndexA));
                    ptr->setNumberA(constIndexA);
                } else if (findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, valueB, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(valueB, constIndexB));
                    ptr->setNumberB(constIndexB);
                }
            }
        } else if (ptr->type == "loadFrom" || ptr->type == "calculateAddress") {
            if (ptr->type == "loadFrom") {
                int arrayIndex = ptr->numberA;
                int offset;
                if (constArrays.count(arrayIndex) &&
                    findCertainValueInSet(ptr->numberB, ptr, valueDefPlaces, offset, newConstStms)) {
                    int constIndexB;
                    newConstStms.insert(Stm::makeConst(offset, constIndexB));
                    ptr->setNumberB(constIndexB);
                    this->setCertainValue(constArrays[arrayIndex][offset]);
                    return true;
                }
            }
        }
        return false;
    }

};

class DefiniationAnalyse {

    map<int, int> vars2Function;  // 变量、变量数组、参数、参数数组、临时变量、分别属于哪个函数。-1表示全局
    map<int, int> constValues;  // 常量的值。
    map<int, vector<int> > constArrays;  // 常量数组的值

    map<tuple<int, shared_ptr<Stm> >, set<shared_ptr<DefOne> >> valueDefPlaces;  // 某条语句中的某个变量的值都有可能是在哪里被定义的。

    map<shared_ptr<BasicBlock>, set<shared_ptr<DefOne> > > globalIn;  // 每个基本块
    map<shared_ptr<BasicBlock>, set<shared_ptr<DefOne> > > globalOut;  // 每个基本块
    map<shared_ptr<BasicBlock>, set<shared_ptr<DefOne> > > localIn;  // 每个基本块
    map<shared_ptr<BasicBlock>, set<shared_ptr<DefOne> > > localOut;  // 每个基本块
    map<shared_ptr<BasicBlock>, set<shared_ptr<DefOne> > > globalGen;
    map<shared_ptr<BasicBlock>, set<shared_ptr<DefOne> > > localGen;

    map<shared_ptr<Stm>, shared_ptr<DefOne> > stm2DefOne;

    /**
     * 由已知量转化成的常量值。
     */
    map<int, int> newConstValues;
    set<shared_ptr<Stm> > newConstStms;

    void addDefToOneSet(shared_ptr<DefOne> &oneDef, set<shared_ptr<DefOne> > &oneSet) {
        shared_ptr<DefOne> target = nullptr;
        for (auto &ptr:oneSet) {
            if (ptr->getValueIndex() == oneDef->getValueIndex()) {
                target = ptr;
                break;
            }
        }
        if (target != nullptr) {
            oneSet.erase(target);
        }
        oneSet.insert(oneDef);
    }

    /**
     * 分析不同语句的类型，判断出这条语句到底是给谁赋值的。如果是非赋值语句，则返回-1.
     */
    int findDefTarget(shared_ptr<Stm> &ptr) {
        if (ptr->type == "declaration") {
            if (ptr->operation == "ConstUnarray") {
                return ptr->result;
            } else if (ptr->operation == "ParaUnarray") {
                return ptr->result;
            }
        } else if (ptr->type == "assign") {
            if (ptr->operation == "VarUnarrayInitial") {
                return ptr->result;
            } else if (ptr->operation == "Unarray") {
                return ptr->result;
            }
        } else if (ptr->type == "getReturnValue") {
            return ptr->result;
        } else if (ptr->type == "getInt") {
            return ptr->result;
        } else if (ptr->type == "mul" || ptr->type == "add" || ptr->type == "sub" || ptr->type == "div" ||
                   ptr->type == "mode") {
            return ptr->result;
        } else if (ptr->type == "not" || ptr->type == "lt" || ptr->type == "le" || ptr->type == "equalCheck" ||
                   ptr->type == "notEqualCheck" || ptr->type == "gt" || ptr->type == "ge") {
            return ptr->result;
        } else if (ptr->type == "loadFrom" || ptr->type == "calculateAddress") {
            return ptr->result;
        }

        return -1;
    }

    void addDef(shared_ptr<Stm> &stm_ptr, shared_ptr<BasicBlock> &block) {
        int targetValueIndex = findDefTarget(stm_ptr);
        if (targetValueIndex == -1) {
            return;
        }
        shared_ptr<DefOne> oneDef = make_shared<DefOne>(block, stm_ptr, targetValueIndex);
        stm2DefOne[stm_ptr] = oneDef;  // 把这条语句与这次定义关联。
        if (vars2Function.count(targetValueIndex)) {
            if (vars2Function[targetValueIndex] == -1) {  // 这是一个main或全局变量
                addDefToOneSet(oneDef, globalGen[block]);
            } else { // 这是一个函数中的变量
                addDefToOneSet(oneDef, localGen[block]);
            }
        } else {
            cout << "错误！竟然不知道它是全局变量还是函数变量！" << endl;
        }
    }

    bool addDefToOutSet(shared_ptr<DefOne> &oneDef,
                        set<shared_ptr<DefOne> > genDef,
                        set<shared_ptr<DefOne> > &defSet) {
        if (defSet.count(oneDef)) {
            return false;  // 原来添加过了
        }
        for (auto &ptr:genDef) {
            if (ptr->getValueIndex() == oneDef->getValueIndex()) {
                return false;
            }
        }
        defSet.insert(oneDef);
        return true;
    }


public:


    /**
     * 对于每个基本块，计算出他的gen。
     *      包括全局变量和局部变量。
     * 初始化每个块的 out 为 gen
     */
    void analyseEveryBlock() {
        for (auto &block:Optimizer::allBlocks) {
            globalGen[block] = {};
            localGen[block] = {};
            for (auto &ptr:block->stm_ptrs) {
                if (ptr->type == "declaration") {
                    if (ptr->operation == "temp") {     // 计算过程中的临时变量，需要有对应的内存地址
                        if (block->functionAreaPtr == nullptr) {
                            vars2Function[ptr->result] = -1;
                        } else {
                            vars2Function[ptr->result] = block->functionAreaPtr->functionIndex;
                        }
                    } else if (ptr->operation == "ConstUnarray") {
                        constValues[ptr->result] = ptr->numberA;  // 常量属于哪个函数也能知道。
                        if (block->functionAreaPtr == nullptr) {
                            vars2Function[ptr->result] = -1;
                        } else {
                            vars2Function[ptr->result] = block->functionAreaPtr->functionIndex;
                        }

                    } else if (ptr->operation == "ConstArray") {
                        constArrays[ptr->result] = {};
                    } else if (ptr->operation == "VarUnarray") {
                        if (block->functionAreaPtr == nullptr) {
                            vars2Function[ptr->result] = -1;
                        } else {
                            vars2Function[ptr->result] = block->functionAreaPtr->functionIndex;
                        }
                    } else if (ptr->operation == "VarArray") {
                        if (block->functionAreaPtr == nullptr) {
                            vars2Function[ptr->result] = -1;
                        } else {
                            vars2Function[ptr->result] = block->functionAreaPtr->functionIndex;
                        }
                    } else if (ptr->operation == "ParaUnarray") {   // 参数的大小均为4
                        if (block->functionAreaPtr == nullptr) {
                            vars2Function[ptr->result] = -1;
                        } else {
                            vars2Function[ptr->result] = block->functionAreaPtr->functionIndex;
                        }
                    } else if (ptr->operation == "ParaArray") {     // 参数的大小均为4
                        if (block->functionAreaPtr == nullptr) {
                            vars2Function[ptr->result] = -1;
                        } else {
                            vars2Function[ptr->result] = block->functionAreaPtr->functionIndex;
                        }
                    }
                } else if (ptr->type == "assign") {
                    if (ptr->operation == "ConstArray") {
                        if (constArrays.count(ptr->result) == 0) {
                            constArrays[ptr->result] = {};
                        }
                        constArrays[ptr->result].emplace_back(ptr->numberA);
                    }
                }
                addDef(ptr, block);
            }
            globalOut[block].insert(globalGen[block].begin(), globalGen[block].end());
//            cout << "block " << block->blockNumber << "的global_gen 有" << globalOut[block].size() << "个,它们是" << endl;
//            for (auto x:globalOut[block]) {
//                cout << "@" << x->getValueIndex() << endl;
//            }


            localOut[block].insert(localGen[block].begin(), localGen[block].end());
//            cout << "block " << block->blockNumber << "的local_gen 有" << localOut[block].size() << "个,它们是" << endl;
//            for (auto x:localOut[block]) {
//                cout << "@" << x->getValueIndex() << endl;
//            }

        }
    }

    void analyseOutAndIn() {


        Optimizer::calculateBeforeBlocks();  // 计算前驱节点块
        bool isFind = true;
        while (isFind) {
            isFind = false;
            for (auto &block:Optimizer::allBlocks) {
                for (auto &beforeBlock:block->beforeBlocks) {
                    for (auto &oneDef:globalOut[beforeBlock]) {
                        if (!globalIn[block].count(oneDef)) {
                            globalIn[block].insert(oneDef);
                            isFind = true;
                        }
                    }
                }
                for (auto oneDef:globalIn[block]) {
                    if (addDefToOutSet(oneDef, globalGen[block], globalOut[block])) {
                        isFind = true;
                    }
                }
            }
        }

        Optimizer::devideWithFunction();
        Optimizer::calculateBeforeBlocks();  // 计算前驱节点块
        isFind = true;
        while (isFind) {
            isFind = false;
            for (auto &block:Optimizer::allBlocks) {
                for (auto &beforeBlock:block->beforeBlocks) {
                    for (auto &oneDef:localOut[beforeBlock]) {
                        if (!localIn[block].count(oneDef)) {
                            localIn[block].insert(oneDef);
                            isFind = true;
                        }
                    }
                }
                for (auto oneDef:localIn[block]) {
                    if (addDefToOutSet(oneDef, localGen[block], localOut[block])) {
                        isFind = true;
                    }
                }
            }
        }
        Optimizer::analyzeBlocks();
    }

    void findValueDefPlace(int valueIndex, shared_ptr<Stm> &stm_ptr, set<shared_ptr<DefOne> > &defSets) {
        bool isFind = false;
        for (auto &defPtr:defSets) {
            if (defPtr->getValueIndex() == valueIndex) {
                if (!valueDefPlaces.count({valueIndex, stm_ptr})) {
                    valueDefPlaces[{valueIndex, stm_ptr}] = {};
                }
                valueDefPlaces[{valueIndex, stm_ptr}].insert(defPtr);
                isFind = true;
            }
        }
        if (!isFind) {
            cout << "错误 变量" << valueIndex << "竟然没找到定义" << endl;
        }
    }

    /**
     * 针对每个语句用到的每个变量，分析对这个变量的定义可能来自哪里。
     */
    void analyseDefFrom() {
        for (auto &block:Optimizer::allBlocks) {
            set<shared_ptr<DefOne> > inDefs;
            inDefs.insert(globalIn[block].begin(), globalIn[block].end());
            inDefs.insert(localIn[block].begin(), localIn[block].end());
            //  刚进入这个基本块时，已经有许多变量有定义了。
            for (auto &ptr:block->stm_ptrs) {
                if (ptr->type == "declaration") {
                } else if (ptr->type == "assign") {
                    if (ptr->operation == "ConstArray") {
                    } else if (ptr->operation == "VarUnarrayInitial") {
                        int valueIndex = ptr->numberA;
                        findValueDefPlace(valueIndex, ptr, inDefs);
                    } else if (ptr->operation == "VarArrayInitial") {
                        int valueIndex = ptr->numberA;
                        findValueDefPlace(valueIndex, ptr, inDefs);
                    } else if (ptr->operation == "Unarray") {
                        int valueIndex = ptr->numberA;
                        findValueDefPlace(valueIndex, ptr, inDefs);
                    } else if (ptr->operation == "Array") {
                        int arrayIndex = ptr->result;   // 要给哪个数组赋值  。
                        int offSetIndex = ptr->numberA;
                        int valueIndex = ptr->numberB;
                        // 这里并不清楚参数数组的值是在哪里定义的。
                        findValueDefPlace(offSetIndex, ptr, inDefs);
                        findValueDefPlace(valueIndex, ptr, inDefs);
                    }
                } else if (ptr->type == "setMain" || ptr->type == "return" || ptr->type == "setPara" ||
                           ptr->type == "callFunction" || ptr->type == "getReturnValue" || ptr->type == "function") {
                    if (ptr->type == "setMain") {
                    } else if (ptr->type == "return") {
                        if (ptr->operation == "main") {
                        } else if (ptr->operation == "") {
                            int returnValueIndex = ptr->result;
                            if (returnValueIndex != -1) {  // 有返回值
                                findValueDefPlace(returnValueIndex, ptr, inDefs);
                            }
                        }
                    } else if (ptr->type == "setPara") {
                        findValueDefPlace(ptr->result, ptr, inDefs);
                    } else if (ptr->type == "callFunction") {
                    } else if (ptr->type == "getReturnValue") {
                    } else if (ptr->type == "function") {
                    }
                } else if (ptr->type == "label" || ptr->type == "jump" || ptr->type == "jumpIf" ||
                           ptr->type == "jumpIfNot") {
                    if (ptr->type == "label") {
                    } else if (ptr->type == "jump") {
                    } else if (ptr->type == "jumpIf") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                    } else if (ptr->type == "jumpIfNot") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                    }
                } else if (ptr->type == "print" || ptr->type == "getInt") {
                    if (ptr->type == "print") {
                        if (ptr->operation == "value") {
                            int valueIndex = ptr->result;
                            findValueDefPlace(valueIndex, ptr, inDefs);
                        } else if (ptr->operation == "str") {
                        }
                    } else if (ptr->type == "getInt") {
                    }
                } else if (ptr->type == "mul" || ptr->type == "add" || ptr->type == "sub" || ptr->type == "div" ||
                           ptr->type == "mode") {
                    if (ptr->type == "mul") {
                        if (ptr->operation == "Const") {
                            findValueDefPlace(ptr->numberA, ptr, inDefs);
                        } else if (ptr->operation == "") {
                            findValueDefPlace(ptr->numberA, ptr, inDefs);
                            findValueDefPlace(ptr->numberB, ptr, inDefs);
                        }
                    } else if (ptr->type == "add") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    } else if (ptr->type == "sub") {
                        if (ptr->operation == "ConstSub") {
                            findValueDefPlace(ptr->numberB, ptr, inDefs);
                        } else if (ptr->operation == "") {
                            findValueDefPlace(ptr->numberA, ptr, inDefs);
                            findValueDefPlace(ptr->numberB, ptr, inDefs);
                        }
                    } else if (ptr->type == "div") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    } else if (ptr->type == "mode") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    }
                } else if (ptr->type == "not" || ptr->type == "lt" || ptr->type == "le" || ptr->type == "equalCheck" ||
                           ptr->type == "notEqualCheck" || ptr->type == "gt" || ptr->type == "ge") {
                    if (ptr->type == "not") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                    } else if (ptr->type == "equalCheck") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    } else if (ptr->type == "notEqualCheck") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    } else if (ptr->type == "gt") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    } else if (ptr->type == "ge") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    } else if (ptr->type == "lt") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    } else if (ptr->type == "le") {
                        findValueDefPlace(ptr->numberA, ptr, inDefs);
                        findValueDefPlace(ptr->numberB, ptr, inDefs);
                    }
                } else if (ptr->type == "loadFrom" || ptr->type == "calculateAddress") {
                    if (ptr->type == "loadFrom") {
                        int offsetIndex = ptr->numberB;
                        findValueDefPlace(offsetIndex, ptr, inDefs);
                    } else if (ptr->type == "calculateAddress") {
                        int offsetIndex = ptr->numberB;
                        findValueDefPlace(offsetIndex, ptr, inDefs);
                    }
                }

                if (stm2DefOne.count(ptr)) {
                    addDefToOneSet(stm2DefOne[ptr], inDefs);
                }
            }
        }
    }

    /**
     *  对于每一条语句用到的每个变量，分析它的值，判断这个值是否是定值。
     *  有的值是暂时还不能确定的，有的是肯定不能确定的，比如函数返回值，从数组中获取的值，getint()获取的值。
     */
    void analyseCertainValue() {
        set<shared_ptr<DefOne> > remainDefs;  //  剩下的需要分析的赋值语句。
        for (auto &tuple:stm2DefOne) {
            remainDefs.insert(tuple.second);
        }
        bool find = true;
        while (find) {
            find = false;
            for (auto &defOne:remainDefs) {
                if (!defOne->hasCertainValue()
                    && defOne->tryFindCertainValue(valueDefPlaces, constArrays, newConstStms)) {
                    find = true;
                }
            }
        }
    }

    /**
     * 把常量定义重新添加到第一个基本块最前面
     */
    void addConstDefs() {
        shared_ptr<BasicBlock> newBlock = make_shared<BasicBlock>();
        for (auto stm_ptr:newConstStms) {
            newBlock->stm_ptrs.emplace_back(stm_ptr);
        }
        vector<shared_ptr<BasicBlock> > newBlocks;
        newBlocks.emplace_back(newBlock);
        for (auto block:Optimizer::allBlocks) {
            newBlocks.emplace_back(block);
        }
        Optimizer::allBlocks = newBlocks;
    }

    void analyse() {
        analyseEveryBlock();


        analyseOutAndIn();
        analyseDefFrom();

        analyseCertainValue();
        addConstDefs();
        Optimizer::analyzeBlocks();
    }


};


#endif //COMPILER_DEFINIATIONANALYSE_H
