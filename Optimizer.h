//
// Created by HP on 2021/12/1.
//

#ifndef COMPILER_OPTIMIZER_H
#define COMPILER_OPTIMIZER_H

#include "Stm.h"
#include "vector"
#include "map"
#include <queue>
#include <set>
#include "tuple"
#include "ConflictGraph.h"

using namespace std;

class Optimizer;

class UseOrDef {
public:
    int index;  // 这个变量的序号
    string type;   // "use" or "def"
    int functionIndex;  //  所在函数的Index 。若为-1则表示全局。
    UseOrDef(int index, string type, int functionIndex) {
        this->index = index;
        this->type = type;
        this->functionIndex = functionIndex;
    }

};

class BasicBlock;

class FunctionArea {
public:
    int functionIndex = -1;
    vector<shared_ptr<BasicBlock> > innerBlocks;  // 本函数包含的所有基本块

};

class BasicBlock {

    static int blockIndex;
public:

    /**
 * 全局不变量
 */
    static map<int, int> certainValues;
    /**
     * 全局不变数组(常量数组)
     */
    static map<int, vector<int> > certainArrays;

    static set<int> globalRegisters;
    /**
     * 全局寄存器的分配结果
     */
    static map<int, int> value2Register;

    int blockNumber;

    BasicBlock() {
        this->blockNumber = ++blockIndex;
    }

    shared_ptr<FunctionArea> functionAreaPtr = nullptr;  // 这个基本块属于哪个函数，如果是nullptr则代表不属于任何函数。

    vector<shared_ptr<BasicBlock> > beforeBlocks;  // 哪些基本块可以跳转到这个基本块
    vector<shared_ptr<BasicBlock> > afterBlocks;  // 这个基本块可以跳转到哪些基本块

    vector<shared_ptr<Stm> > stm_ptrs = vector<shared_ptr<Stm> >();  // 这个基本块包含的所有语句
    vector<bool> isUseful = vector<bool>();  // 各个语句是否有意义

//    /**
//     * 进入这个基本块时能确定值的变量
//     */
//    set<int, int> inCertainValues;
//
//    /**
//     * 离开这个基本块时能确定值的变量
//     */
//    set<int, int> outCertainValues;

    vector<shared_ptr<UseOrDef> > useOrDefArray;
    int useOrDefIndex = 0;  // 下一个即将遇到的定义或者使用。

    void addUseVar(int index, int functionIndex) {

        useOrDefArray.emplace_back(make_shared<UseOrDef>(index, "use", functionIndex));
    }

    void addCleanRegister(int registerIndex) {

        useOrDefArray.emplace_back(make_shared<UseOrDef>(registerIndex, "register", -1));
    }

    void addDefVar(int index, int functionIndex) {

        useOrDefArray.emplace_back(make_shared<UseOrDef>(index, "def", functionIndex));
    }

    bool hasCertainValue(int valueIndex) {
        return certainValues.count(valueIndex);
    }

    int getCertainValue(int valueIndex) {
        return certainValues[valueIndex];
    }

    void useOneVar(int index) {
        if (index != useOrDefArray[useOrDefIndex]->index) {
            cout << "这里出现了报错 ： " << "预测使用 : " << useOrDefArray[useOrDefIndex]->index << " 实际遇到的是: " << index << endl;
        }
        useOrDefIndex += 1;
    }

    void defOneVar(int index) {

        if (index != useOrDefArray[useOrDefIndex]->index) {
            cout << "这里出现了报错 ： " << "预测定义 : " << useOrDefArray[useOrDefIndex]->index << " 实际遇到的是: " << index << endl;
        }
        useOrDefIndex += 1;
    }

    void cleanOneRegister(int register1) {

        if (register1 != useOrDefArray[useOrDefIndex]->index) {
            cout << "这里出现了报错 : " << "预测使用寄存器" << register1 << "实际上不一定" << endl;
        }
        useOrDefIndex += 1;
    }


    bool isVarLive(int valueIndex) {
        for (int i = useOrDefIndex; i < useOrDefArray.size(); i++) {
            shared_ptr<UseOrDef> ptr = useOrDefArray[i];
            if (ptr->index == valueIndex && (ptr->type == "use" || ptr->type == "def")) {
                return ptr->type == "use";
            }
        }

        for (auto &ptr:liveOutGlobal2) {
            if (ptr == valueIndex) {
                return true;
            }
        }
        for (auto &ptr:liveOutLocal2) {
            if (ptr == valueIndex) {
                return true;
            }
        }
        return false;
    }

    /**
     * 在当前块里还会用到这个变量吗
     */
    bool isUseFulHere(int valueIndex) {
        for (int i = useOrDefIndex; i < useOrDefArray.size(); i++) {
            shared_ptr<UseOrDef> ptr = useOrDefArray[i];
            if (ptr->index == valueIndex && (ptr->type == "use" || ptr->type == "def")) {
                return ptr->type == "use";
            }
        }
        return false;
    }


    /**
     * 活跃变量的In集合。
     */
    set<int> liveInGlobal2;

    set<int> liveInLocal2;

    /**
     * 活跃变量的Out集合
     */
    set<int> liveOutGlobal2;

    set<int> liveOutLocal2;


    shared_ptr<Stm> getFirstStm() {
        if (stm_ptrs.empty()) {
            return nullptr;
        } else {
            return stm_ptrs[0];
        }
    }

    shared_ptr<Stm> getLastStm() {
        if (stm_ptrs.empty()) {
            return nullptr;
        } else {
            return stm_ptrs[stm_ptrs.size() - 1];
        }
    }

    void addStm(shared_ptr<Stm> stm_ptr) {
        stm_ptrs.emplace_back(stm_ptr);
    }

    /**
     * 多久之后会用到这个寄存器/变量。
     */
    int futureUse(int register1, int varIndex) {
        bool findDef = false;
        for (int i = useOrDefIndex; i < useOrDefArray.size(); i++) {
            if (!findDef && useOrDefArray[i]->type == "use" && useOrDefArray[i]->index == varIndex) {
                return i - useOrDefIndex;
            } else if (useOrDefArray[i]->type == "register" && useOrDefArray[i]->index == register1) {
                return i - useOrDefIndex;
            } else if (useOrDefArray[i]->type == "def" && useOrDefArray[i]->index == varIndex) {
                findDef = true;
            }
        }
        return -1;  // 当前基本块不会再用了
    }


    /**
     * 这个基本块是不是由某个label引领的
     */
    bool isLabel(int labelIndex) {
        return stm_ptrs[0]->type == "label" && stm_ptrs[0]->result == labelIndex;
    }

    int getLabel() {
        if (stm_ptrs[0]->type == "label") {
            return stm_ptrs[0]->result;
        } else {
            return -1;  // 正常情况不会到这里
        }
    }

    /**
     * 看看这个基本块的最后是不是跳到了一个桥梁，如果是，那么获取桥梁的目标，然后直接跳到目标。
     */
    void makeStright() {
        string type = stm_ptrs[stm_ptrs.size() - 1]->type;
        if (type == "jump" || type == "jumpIf" || type == "jumpIfNot") {
            shared_ptr<BasicBlock> targetBlock = nullptr;
            shared_ptr<BasicBlock> newTarget = nullptr;
            int label = stm_ptrs[stm_ptrs.size() - 1]->result;
            for (auto &afterBlock:afterBlocks) {
                if (afterBlock->isLabel(label)) {
                    shared_ptr<BasicBlock> nextAfter = afterBlock->getJumpStright();
                    if (nextAfter != nullptr) {
                        newTarget = nextAfter;
                        stm_ptrs[stm_ptrs.size() - 1] = make_shared<Stm>(
                                stm_ptrs[stm_ptrs.size() - 1]->type,
                                stm_ptrs[stm_ptrs.size() - 1]->operation,
                                stm_ptrs[stm_ptrs.size() - 1]->numberA,
                                stm_ptrs[stm_ptrs.size() - 1]->numberB,
                                nextAfter->getLabel());;
                        targetBlock = afterBlock;
                        break;
                    }
                }
            }
            if (targetBlock != nullptr) {
                vector<shared_ptr<BasicBlock> > newAfterBlocks;
                for (auto afterBlock:afterBlocks) {
                    if (afterBlock != targetBlock) {
                        newAfterBlocks.emplace_back(afterBlock);
                    }
                }
                newAfterBlocks.emplace_back(newTarget);
                afterBlocks = newAfterBlocks;
            }
        }
    }

    /**
     * 如果这个基本块仅仅起到跳转的中间连接作用，则返回一个能直接起到这种跳转作用的语句。
     * 否则返回nullptr
     * @return
     */
    shared_ptr<BasicBlock> getJumpStright() {
        if (stm_ptrs.size() == 1) {
            if (stm_ptrs[0]->type == "jump") {
                int label = stm_ptrs[0]->result;
                for (auto &nextBlock:afterBlocks) {
                    if (nextBlock->isLabel(label)) {
                        shared_ptr<BasicBlock> nextAfter = nextBlock->getJumpStright();
                        if (nextAfter == nullptr) {
                            return nextBlock;
                        } else {
                            return nextAfter;
                        }
                    }
                }
            }
        } else if (stm_ptrs.size() == 2) {
            if (stm_ptrs[0]->type == "label" && stm_ptrs[1]->type == "jump") {
                int label = stm_ptrs[1]->result;
                for (auto &nextBlock:afterBlocks) {
                    if (nextBlock->isLabel(label)) {

                        shared_ptr<BasicBlock> nextAfter = nextBlock->getJumpStright();
                        if (nextAfter == nullptr) {
                            return nextBlock;
                        } else {
                            return nextAfter;
                        }
                    }
                }

            }
        }
        return nullptr;
    }


    /**
     * 对于每个基本块，分析它哪些变量的值是能在编译时确定的。返回是否找到了新的
     */
    bool analyseCertainValues() {
        bool isFind = false;
        set<int> tempVars;  // temp变量只会在同一个基本块内出现，而且只会被赋值一次
        int change = 0;
        for (auto &stm:stm_ptrs) {
            string type = stm->type;

            if (certainValues.count(5)) {
                change++;
            }


            if (type == "declaration") {
                if (stm->operation == "temp") {
                    tempVars.insert(stm->result);
                } else if (stm->operation == "ConstUnarray") {
                    if (!certainValues.count(stm->result)) {
                        certainValues[stm->result] = stm->numberA;
                        isFind = true;
                    }
                } else if (stm->operation == "ConstArray") {
                    certainArrays[stm->result] = vector<int>();  //  这里每次都会重新赋值，可能会影响性能
                }
            } else if (type == "assign") {
                if (stm->operation == "ConstArray") {
                    certainArrays[stm->result].emplace_back(stm->numberA);
                }
            } else if (type == "mul" || type == "add" || type == "sub" || type == "div" ||
                       type == "mode") {
                int numberA = stm->numberA;
                int numberB = stm->numberB;
                int resultIndex = stm->result;
                if (certainValues.count(resultIndex) || !tempVars.count(resultIndex)) {  // 不考虑非临时变量的常量
                    continue;  // 之前算过这个变量
                }
                if (type == "mul") {
                    if (stm->operation == "Const") {
                        if (certainValues.count(numberA)) {

                            certainValues[resultIndex] = certainValues[numberA] * numberB;
                            isFind = true;
                        }
                    } else if (stm->operation == "") {
                        if (certainValues.count(numberA) && certainValues.count(numberB)) {
                            certainValues[resultIndex] = certainValues[numberA] * certainValues[numberB];
                            isFind = true;
                        }
                    }
                } else if (type == "add") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {

                        certainValues[resultIndex] = certainValues[numberA] + certainValues[numberB];
                        isFind = true;
                    }
                } else if (type == "sub") {
                    if (stm->operation == "ConstSub") {
                        if (certainValues.count(numberB)) {
                            certainValues[resultIndex] = numberA - certainValues[numberB];
                            isFind = true;
                        }
                    } else {
                        if (certainValues.count(numberA) && certainValues.count(numberB)) {
                            certainValues[resultIndex] = certainValues[numberA] - certainValues[numberB];
                            isFind = true;
                        }
                    }
                } else if (type == "div") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] / certainValues[numberB];
                        isFind = true;
                    }
                } else if (type == "mode") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] % certainValues[numberB];
                        isFind = true;
                    }
                }
            } else if (type == "not" || type == "lt" || type == "le" || type == "equalCheck" ||
                       type == "notEqualCheck" || type == "gt" || type == "ge") {
                int resultIndex = stm->result;
                int numberA = stm->numberA;
                int numberB = stm->numberB;
                if (certainValues.count(resultIndex) || !tempVars.count(resultIndex)) {
                    continue;
                }
                if (type == "not") {
                    if (certainValues.count(numberA)) {
                        certainValues[resultIndex] = !certainValues[numberA];
                        isFind = true;
                    }
                } else if (type == "lt") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] < certainValues[numberB];
                        isFind = true;
                    }
                } else if (type == "le") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] <= certainValues[numberB];
                        isFind = true;
                    }
                } else if (type == "equalCheck") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] == certainValues[numberB];
                        isFind = true;
                    }
                } else if (type == "notEqualCheck") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] != certainValues[numberB];
                        isFind = true;
                    }
                } else if (type == "gt") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] > certainValues[numberB];
                        isFind = true;
                    }
                } else if (type == "ge") {
                    if (certainValues.count(numberA) && certainValues.count(numberB)) {
                        certainValues[resultIndex] = certainValues[numberA] >= certainValues[numberB];
                        isFind = true;
                    }
                }
            } else if (type == "loadFrom" || type == "calculateAddress") {
                if (type == "loadFrom") {
                    if (certainValues.count(stm->result) || !certainArrays.count(stm->numberA) ||
                        !certainValues.count(stm->numberB) || !tempVars.count(stm->result)) {
                        continue;
                    }
                    certainValues[stm->result] = certainArrays[stm->numberA][certainValues[stm->numberB]];
                    isFind = true;
                }
            }
        }
        return isFind;
    }

    int getGlobalRegister(int value) {
        if (value2Register.count(value)) {
            return value2Register[value];
        } else {
            return -1;
        }
    }

    bool isGlobalRegister(int register_) {
        return globalRegisters.count(register_);
    }

};

class Optimizer {


public:
    static vector<shared_ptr<BasicBlock> > allBlocks;

    static map<int, int> varsBelongsTo;
    static map<int, int> constValues;


    static int varFunction(int varIndex) {
        if (constValues.count(varIndex)) {
            return -2;
        } else if (varsBelongsTo.count(varIndex)) {
            return Optimizer::varsBelongsTo[varIndex];
        } else {
            cout << "不知道变量所属的域" << varIndex << endl;
            return -3;
        }
    }


    /**
     * 死代码删除
     */
    static void deleteUselessStatement() {
        bool find = true;
        while (find) {
            find = false;
            for (auto &block:allBlocks) {
                set<int> liveVar;
                for (auto &nextBlock:block->afterBlocks) {
                    for (auto &index:nextBlock->liveInGlobal2) {
                        liveVar.insert(index);
                    }
                }

                if (block->isUseful.empty()) {
                    block->isUseful = vector<bool>(block->stm_ptrs.size(), false);
                }

                for (int i = (int) block->stm_ptrs.size() - 1; i >= 0; i--) {
                    shared_ptr<Stm> stm_ptr = block->stm_ptrs[i];
                    if (stm_ptr->type == "assign") {
                        if (BasicBlock::certainValues.count(stm_ptr->result)) {
                            continue;
                        }
                        if (stm_ptr->operation == "ConstArray") {
                            block->isUseful[i] = true;
                        } else if (stm_ptr->operation == "VarUnarrayInitial") {
                            int varIndex = stm_ptr->result;
                            int valueIndex = stm_ptr->numberA;
                            if (liveVar.count(varIndex)) {
                                liveVar.erase(varIndex);
                                block->isUseful[i] = true;
                                liveVar.insert(valueIndex);
                            }
                        } else if (stm_ptr->operation == "VarArrayInitial") {
                            int valueIndex = stm_ptr->numberA;
                            block->isUseful[i] = true;
                            liveVar.insert(valueIndex);
                        } else if (stm_ptr->operation == "Unarray") {
                            int varIndex = stm_ptr->result;
                            int valueIndex = stm_ptr->numberA;
                            if (liveVar.count(varIndex)) {
                                liveVar.erase(varIndex);
                                liveVar.insert(valueIndex);
                                block->isUseful[i] = true;
                            }
                        } else if (stm_ptr->operation == "Array") {
                            block->isUseful[i] = true;
                            liveVar.insert(stm_ptr->numberB);
                            liveVar.insert(stm_ptr->numberA);
                            liveVar.insert(stm_ptr->result);
                        } else {
                            cout << "出现了不能处理的assign语句" << endl;
                        }
                    } else if (stm_ptr->type == "return") {
                        block->isUseful[i] = true;
                        if (stm_ptr->operation == "") {
                            int returnValueIndex = stm_ptr->result;
                            if (returnValueIndex != -1) {  // 有返回值
                                liveVar.insert(returnValueIndex);
                            }
                        }
                    } else if (stm_ptr->type == "setPara") {
                        liveVar.insert(stm_ptr->result);  // 给函数设置参数的语句都是有意义的。
                        block->isUseful[i] = true;
                    } else if (stm_ptr->type == "callFunction") {  // TODO 这里认为所有对函数的调用都是有意义的。
                        block->isUseful[i] = true;
                    } else if (stm_ptr->type == "getReturnValue") {
                        block->isUseful[i] = true;
                        if (stm_ptr->result != -1) {
                            if (liveVar.count(stm_ptr->result)) {
                                liveVar.erase(stm_ptr->result);
                            }
                        }
                    } else if (stm_ptr->type == "jumpIf" || stm_ptr->type == "jumpIfNot") {
                        block->isUseful[i] = true;
                        liveVar.insert(stm_ptr->numberA);
                    } else if (stm_ptr->type == "print") {
                        block->isUseful[i] = true;
                        if (stm_ptr->operation == "value") {
                            liveVar.insert(stm_ptr->result);
                        }
                    } else if (stm_ptr->type == "getInt") {
                        block->isUseful[i] = true;
                        liveVar.erase(stm_ptr->result);


                    } else if (stm_ptr->type == "mul" || stm_ptr->type == "add" || stm_ptr->type == "sub" ||
                               stm_ptr->type == "div" || stm_ptr->type == "mode") {
                        if (BasicBlock::certainValues.count(stm_ptr->result)) {
                            continue;
                        }
                        if (liveVar.count(stm_ptr->result)) {
                            liveVar.erase(stm_ptr->result);
                            block->isUseful[i] = true;
                            if (stm_ptr->type == "mul") {
                                if (stm_ptr->operation == "Const") {
                                    liveVar.insert(stm_ptr->numberA);
                                } else if (stm_ptr->operation == "") {
                                    liveVar.insert(stm_ptr->numberA);
                                    liveVar.insert(stm_ptr->numberB);
                                }
                            } else if (stm_ptr->type == "add") {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            } else if (stm_ptr->type == "sub") {


                                if (stm_ptr->operation == "ConstSub") {
                                    liveVar.insert(stm_ptr->numberB);
                                } else if (stm_ptr->operation == "") {
                                    liveVar.insert(stm_ptr->numberA);
                                    liveVar.insert(stm_ptr->numberB);
                                }
                            } else if (stm_ptr->type == "div") {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            } else if (stm_ptr->type == "mode") {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            }
                        }

                    } else if (stm_ptr->type == "not" || stm_ptr->type == "lt" || stm_ptr->type == "le" ||
                               stm_ptr->type == "equalCheck" || stm_ptr->type == "notEqualCheck" ||
                               stm_ptr->type == "gt" || stm_ptr->type == "ge") {
                        if (BasicBlock::certainValues.count(stm_ptr->result)) {
                            continue;
                        }
                        if (liveVar.count(stm_ptr->result)) {
                            block->isUseful[i] = true;
                            liveVar.erase(stm_ptr->result);
                            if (stm_ptr->type == "not") {
                                liveVar.insert(stm_ptr->numberA);
                            } else {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            }
                        }
                    } else if (stm_ptr->type == "loadFrom") {
                        if (BasicBlock::certainValues.count(stm_ptr->result)) {
                            continue;
                        }
                        if (liveVar.count(stm_ptr->result)) {
                            liveVar.erase(stm_ptr->result);
                            block->isUseful[i] = true;
                            liveVar.insert(stm_ptr->numberB);
                            liveVar.insert(stm_ptr->numberA);
                        }
                    } else if (stm_ptr->type == "calculateAddress") {
                        if (liveVar.count(stm_ptr->result)) {
                            liveVar.erase(stm_ptr->result);
                            block->isUseful[i] = true;
                            liveVar.insert(stm_ptr->numberB);
                            liveVar.insert(stm_ptr->numberA);
                        }
                    } else {
                        block->isUseful[i] = true;
                    }
                }

                set<int> globalLiveVar;
                for (auto varIndex:liveVar) {
                    globalLiveVar.insert(varIndex);
                }


                for (auto &index:globalLiveVar) {
                    if (block->liveInGlobal2.count(index) == 0) {
                        find = true;
                        block->liveInGlobal2.insert(index);
                    }
                }
            }
        }

        devideWithFunction();

        find = true;
        while (find) {
            find = false;
            for (auto &block:allBlocks) {
                set<int> liveVar;


                for (auto &nextBlock:block->afterBlocks) {
                    for (auto &index:nextBlock->liveInLocal2) {
                        liveVar.insert(index);
                    }
                }

                if (block->isUseful.empty()) {
                    block->isUseful = vector<bool>(block->stm_ptrs.size(), false);
                }

                for (int i = (int) block->stm_ptrs.size() - 1; i >= 0; i--) {
                    shared_ptr<Stm> stm_ptr = block->stm_ptrs[i];
                    if (stm_ptr->type == "assign") {
                        if (stm_ptr->operation == "ConstArray") {
                            block->isUseful[i] = true;
                        } else if (stm_ptr->operation == "VarUnarrayInitial") {
                            int varIndex = stm_ptr->result;
                            int valueIndex = stm_ptr->numberA;
                            if (liveVar.count(varIndex)) {
                                liveVar.erase(varIndex);
                                block->isUseful[i] = true;
                                liveVar.insert(valueIndex);
                            }
                        } else if (stm_ptr->operation == "VarArrayInitial") {
                            int varArrayIndex = stm_ptr->result;
                            int valueIndex = stm_ptr->numberA;
                            int number = stm_ptr->numberB;
                            block->isUseful[i] = true;
                            liveVar.insert(valueIndex);
                        } else if (stm_ptr->operation == "Unarray") {
                            int varIndex = stm_ptr->result;
                            int valueIndex = stm_ptr->numberA;
                            if (liveVar.count(varIndex)) {
                                liveVar.erase(varIndex);
                                liveVar.insert(valueIndex);
                                block->isUseful[i] = true;
                            }
                        } else if (stm_ptr->operation == "Array") {
                            block->isUseful[i] = true;
                            liveVar.insert(stm_ptr->numberB);
                            liveVar.insert(stm_ptr->numberA);
                            liveVar.insert(stm_ptr->result);
                        } else {
                            cout << "出现了不能处理的assign语句" << endl;
                        }
                    } else if (stm_ptr->type == "return") {
                        block->isUseful[i] = true;
                        if (stm_ptr->operation == "") {
                            int returnValueIndex = stm_ptr->result;
                            if (returnValueIndex != -1) {  // 有返回值
                                liveVar.insert(returnValueIndex);
                            }
                        }
                    } else if (stm_ptr->type == "setPara") {
                        liveVar.insert(stm_ptr->result);  // 给函数设置参数的语句都是有意义的。
                        block->isUseful[i] = true;
                    } else if (stm_ptr->type == "callFunction") {  // TODO 这里认为所有对函数的调用都是有意义的。
                        block->isUseful[i] = true;
                    } else if (stm_ptr->type == "getReturnValue") {
                        if (stm_ptr->result != -1) {
                            if (liveVar.count(stm_ptr->result)) {
                                liveVar.erase(stm_ptr->result);
                                block->isUseful[i] = true;
                            }
                        }
                    } else if (stm_ptr->type == "jumpIf" || stm_ptr->type == "jumpIfNot") {
                        block->isUseful[i] = true;
                        liveVar.insert(stm_ptr->numberA);
                    } else if (stm_ptr->type == "print") {
                        block->isUseful[i] = true;
                        if (stm_ptr->operation == "value") {
                            liveVar.insert(stm_ptr->result);
                        }
                    } else if (stm_ptr->type == "getInt") {
                        block->isUseful[i] = true;
                        liveVar.erase(stm_ptr->result);
                    } else if (stm_ptr->type == "mul" || stm_ptr->type == "add" || stm_ptr->type == "sub" ||
                               stm_ptr->type == "div" || stm_ptr->type == "mode") {
                        if (liveVar.count(stm_ptr->result)) {
                            liveVar.erase(stm_ptr->result);
                            block->isUseful[i] = true;
                            if (stm_ptr->type == "mul") {
                                if (stm_ptr->operation == "Const") {
                                    liveVar.insert(stm_ptr->numberA);
                                } else if (stm_ptr->operation == "") {
                                    liveVar.insert(stm_ptr->numberA);
                                    liveVar.insert(stm_ptr->numberB);
                                }
                            } else if (stm_ptr->type == "add") {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            } else if (stm_ptr->type == "sub") {
                                if (stm_ptr->operation == "ConstSub") {
                                    liveVar.insert(stm_ptr->numberB);
                                } else if (stm_ptr->operation == "") {
                                    liveVar.insert(stm_ptr->numberA);
                                    liveVar.insert(stm_ptr->numberB);
                                }
                            } else if (stm_ptr->type == "div") {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            } else if (stm_ptr->type == "mode") {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            }
                        }

                    } else if (stm_ptr->type == "not" || stm_ptr->type == "lt" || stm_ptr->type == "le" ||
                               stm_ptr->type == "equalCheck" || stm_ptr->type == "notEqualCheck" ||
                               stm_ptr->type == "gt" || stm_ptr->type == "ge") {
                        if (liveVar.count(stm_ptr->result)) {
                            block->isUseful[i] = true;
                            liveVar.erase(stm_ptr->result);
                            if (stm_ptr->type == "not") {
                                liveVar.insert(stm_ptr->numberA);
                            } else {
                                liveVar.insert(stm_ptr->numberA);
                                liveVar.insert(stm_ptr->numberB);
                            }
                        }
                    } else if (stm_ptr->type == "loadFrom") {
                        if (liveVar.count(stm_ptr->result)) {
                            liveVar.erase(stm_ptr->result);
                            block->isUseful[i] = true;
                            liveVar.insert(stm_ptr->numberB);
                            liveVar.insert(stm_ptr->numberA);
                        }
                    } else if (stm_ptr->type == "calculateAddress") {
                        if (liveVar.count(stm_ptr->result)) {
                            liveVar.erase(stm_ptr->result);
                            block->isUseful[i] = true;
                            liveVar.insert(stm_ptr->numberB);
                            liveVar.insert(stm_ptr->numberA);
                        }
                    } else {
                        block->isUseful[i] = true;
                    }
                }

                set<int> locallLiveVar;
                for (auto varIndex:liveVar) {
                    if (varFunction(varIndex) < 0) {
                        continue;
                    }
                    locallLiveVar.insert(varIndex);
                }


                for (auto &index:locallLiveVar) {
                    if (block->liveInLocal2.count(index) == 0) {
                        find = true;
                        block->liveInLocal2.insert(index);
                    }
                }

            }
        }

    }

    /**
     * 把原来的语句划分成若干个基本块。
     *
     */
    static void devideBlocks() {
        bool first = true;
        shared_ptr<BasicBlock> nowBlockPtr = nullptr;
        shared_ptr<FunctionArea> nowFunction = nullptr;

        for (shared_ptr<Stm> &stm_ptr: Stm::allStms) {

            if (stm_ptr->type == "jump" || stm_ptr->type == "jumpIf" || stm_ptr->type == "jumpIfNot" ||
                stm_ptr->type == "return" || stm_ptr->type == "callFunction") {  // 当前语句所在基本块结束
                if (nowBlockPtr == nullptr) {
                    nowBlockPtr = make_shared<BasicBlock>();
                    nowBlockPtr->functionAreaPtr = nowFunction;
                    nowBlockPtr->stm_ptrs = vector<shared_ptr<Stm> >();
                    allBlocks.emplace_back(nowBlockPtr);
                }
                nowBlockPtr->addStm(stm_ptr);
                if (nowFunction != nullptr) {
                    nowFunction->innerBlocks.emplace_back(nowBlockPtr);
                }
                nowBlockPtr = nullptr;
            } else if (stm_ptr->type == "label" ||
                       (stm_ptr->type == "function" && stm_ptr->operation == "start")) {  //  前一条语句所在基本块结束

                if (stm_ptr->type == "function" && stm_ptr->operation == "start") {
                    nowFunction = make_shared<FunctionArea>();
                    nowFunction->functionIndex = stm_ptr->result;
//                    cout << "创建新的函数块" << endl;
                }

                nowBlockPtr = make_shared<BasicBlock>();
                nowBlockPtr->functionAreaPtr = nowFunction;
                if (nowFunction != nullptr) {
                    nowFunction->innerBlocks.emplace_back(nowBlockPtr);
                }
                nowBlockPtr->stm_ptrs = vector<shared_ptr<Stm> >();
                allBlocks.emplace_back(nowBlockPtr);
                nowBlockPtr->addStm(stm_ptr);
            } else {

                if (nowBlockPtr == nullptr) {
                    nowBlockPtr = make_shared<BasicBlock>();
                    nowBlockPtr->functionAreaPtr = nowFunction;
                    nowBlockPtr->stm_ptrs = vector<shared_ptr<Stm> >();
                    allBlocks.emplace_back(nowBlockPtr);
                }
                if (nowFunction != nullptr) {
                    nowFunction->innerBlocks.emplace_back(nowBlockPtr);
                }
                if (stm_ptr->type == "function" && stm_ptr->operation == "end") {
                    nowFunction = nullptr;
                }
                nowBlockPtr->addStm(stm_ptr);
            }

        }
    }


    /**
        * 一个基本块是否可以与它前面的基本块合并。条件是除了前面那个基本块之外，其他基本块不会跳转到当前基本块。
        * @return
    */
    static bool canMergeWithBefore(shared_ptr<BasicBlock> block) {
        int thisIndex = -1;
        shared_ptr<BasicBlock> beforeBlock = nullptr;  // 紧挨着在前面的。
        set<shared_ptr<BasicBlock> > beforeBlocks;   //  所有能跳转到本块的块。
        for (int i = 0; i < allBlocks.size(); i++) {
            shared_ptr<BasicBlock> thisBlock = allBlocks[i];
            for (auto &block2:thisBlock->afterBlocks) {
                if (block2 == block) {
                    beforeBlocks.insert(thisBlock);
                }
            }
            if (allBlocks[i] == block) {
                if (i == 0) {
                    return false;
                }
                beforeBlock = allBlocks[i - 1];
            }
        }


        if (beforeBlocks.count(beforeBlock)) {

            beforeBlocks.erase(beforeBlock);
            if (!beforeBlocks.empty()) {
                return false;
            }
            if (beforeBlock->afterBlocks.size() > 1 || beforeBlock->afterBlocks.size() == 0) {
                return false;
            } else {
                return beforeBlock->afterBlocks[0] == block;
            }
        } else {
            return false;
        }
    }

    static void deleteEmptyBlocks() {

    }

    /**
     * 分析基本块的相互跳转信息。要求allBlocks存储着按顺序从前到后出现的各个基本块。
     *
     * 分析后会存储至每个基本块的beforeBlocks和afterBlocks。
     */
    static void analyzeBlocks() {
        map<int, shared_ptr<BasicBlock> > label2Block;  // label序号或者function序号 对应的基本块
        map<int, vector<shared_ptr<BasicBlock> > > getFunctionReturnValues;  // 对于一个函数，哪些基本块会获取它的返回值或者返回点

        //  去除空基本块
        vector<shared_ptr<BasicBlock> > withOutEmptyBlocks;
        for (auto &blockPtr:allBlocks) {
            if (!blockPtr->stm_ptrs.empty()) {
                withOutEmptyBlocks.emplace_back(blockPtr);
            }
        }
        allBlocks = withOutEmptyBlocks;


        for (auto &blockPtr:allBlocks) {
            if (blockPtr->getFirstStm() != nullptr) {
                string type = blockPtr->getFirstStm()->type;
                if (type == "label" || (type == "function" && blockPtr->getFirstStm()->operation == "start")) {
                    label2Block[blockPtr->getFirstStm()->result] = blockPtr;
                } else if (type == "getReturnValue") {
                    int functionIndex = blockPtr->getFirstStm()->numberA;
                    if (!getFunctionReturnValues.count(functionIndex)) {
                        getFunctionReturnValues[functionIndex] = vector<shared_ptr<BasicBlock> >();
                    }
                    getFunctionReturnValues[functionIndex].emplace_back(blockPtr);
                }
            }
        }


        for (auto &blockPtr:allBlocks) {
            blockPtr->afterBlocks = vector<shared_ptr<BasicBlock> >();
        }

        for (int i = 0; i < allBlocks.size(); i++) {
            shared_ptr<BasicBlock> blockPtr = allBlocks[i];
            if (blockPtr->getLastStm()->type == "jump") {
                blockPtr->afterBlocks.emplace_back(label2Block[blockPtr->getLastStm()->result]);
            } else if (blockPtr->getLastStm()->type == "jumpIf" || blockPtr->getLastStm()->type == "jumpIfNot") {
                blockPtr->afterBlocks.emplace_back(label2Block[blockPtr->getLastStm()->result]);
                if (i < allBlocks.size() - 1 &&
                    blockPtr->afterBlocks[blockPtr->afterBlocks.size() - 1] != allBlocks[i + 1]) {
                    blockPtr->afterBlocks.emplace_back(allBlocks[i + 1]);
                }
            } else if (blockPtr->getLastStm()->type == "callFunction") {
                if (i < allBlocks.size() - 1) {
                    blockPtr->afterBlocks.emplace_back(allBlocks[i + 1]);
                }
                blockPtr->afterBlocks.emplace_back(label2Block[blockPtr->getLastStm()->numberA]);

            } else if (blockPtr->getLastStm()->type == "return") {
                int thisFunctionIndex = blockPtr->getLastStm()->numberA;  // 当前所在的函数
                if (thisFunctionIndex != -1) {  // 如果是-1，说明是main函数返回
                    blockPtr->afterBlocks.insert(blockPtr->afterBlocks.end(),
                                                 getFunctionReturnValues[thisFunctionIndex].begin(),
                                                 getFunctionReturnValues[thisFunctionIndex].end());
                }
            } else {
                if (i < allBlocks.size() - 1) {
                    blockPtr->afterBlocks.emplace_back(allBlocks[i + 1]);
                }
            }
        }


    }

    static void refresh() {
        for (auto &block:allBlocks) {
            block->useOrDefArray = vector<shared_ptr<UseOrDef> >();
            vector<shared_ptr<Stm> > newStmPtrs;

            for (int i = 0; i < block->stm_ptrs.size(); i++) {
                if (block->isUseful[i]) {
                    newStmPtrs.emplace_back(block->stm_ptrs[i]);
                }
            }


            block->stm_ptrs = newStmPtrs;
            block->isUseful = vector<bool>();


            set<int> newMyLiveGlobalVar;
            for (auto &p:block->liveInGlobal2) {
                if (varFunction(p) == -1) {
                    newMyLiveGlobalVar.insert(p);
                }
            }
            block->liveInGlobal2 = newMyLiveGlobalVar;

            set<int> newMyLiveLocalVar;
            for (auto &p:block->liveInLocal2) {
                if (varFunction(p) >= 0) {
                    newMyLiveLocalVar.insert(p);
                }
            }
            block->liveInLocal2 = newMyLiveLocalVar;

            for (auto &nextBlock:block->afterBlocks) {

                for (auto &index:nextBlock->liveInGlobal2) {
                    block->liveOutGlobal2.insert(index);
                }
                for (auto &index:nextBlock->liveInLocal2) {
                    block->liveOutLocal2.insert(index);
                }
            }
        }

    }


    static void calculateBeforeBlocks() {
        for (auto &blockPtr:allBlocks) {
            blockPtr->beforeBlocks = vector<shared_ptr<BasicBlock> >();
        }

        for (auto &blockPtr:allBlocks) {
            for (auto &nextBlock:blockPtr->afterBlocks) {
                nextBlock->beforeBlocks.emplace_back(blockPtr);
            }
        }
    }

    /**
     * 删除不可到达的基本块。同时维护 Stm::all_stms 和 allBlocks
     */
    static void removeUnArrived() {
        set<shared_ptr<BasicBlock> > canArrived;
        queue<shared_ptr<BasicBlock> > myQue;
        myQue.push(allBlocks[0]);
        while (!myQue.empty()) {
            shared_ptr<BasicBlock> topQue = myQue.front();
            myQue.pop();
            if (canArrived.count(topQue) == 0) {
                canArrived.insert(topQue);
                for (auto &nextBlock:topQue->afterBlocks) {
                    myQue.push(nextBlock);
                }
            }
        }
        vector<shared_ptr<BasicBlock> > newAllBlocks;
        for (auto oldBlock:allBlocks) {
            if (canArrived.count(oldBlock)) {
                newAllBlocks.emplace_back(oldBlock);
            }
        }
        allBlocks = newAllBlocks;
    }

    /**
     * 合并可以合并的基本块，并删除基本块内部的跳转语句
     */
    static void removeBridge2() {
        vector<shared_ptr<BasicBlock> > newAllBlocks;
        for (auto &block:allBlocks) {
            if (canMergeWithBefore(block)) {
                shared_ptr<BasicBlock> beforeBlock = newAllBlocks[newAllBlocks.size() - 1];
                beforeBlock->stm_ptrs.insert(beforeBlock->stm_ptrs.end(),
                                             block->stm_ptrs.begin(),
                                             block->stm_ptrs.end());

            } else {
                newAllBlocks.emplace_back(block);
            }
        }
        for (auto &block:newAllBlocks) {
            vector<shared_ptr<Stm> > newStms;

            for (int i = 0; i < block->stm_ptrs.size(); i++) {
                shared_ptr<Stm> stm_ptr = block->stm_ptrs[i];
                if (i == 0 || i == block->stm_ptrs.size() - 1) {
                    newStms.emplace_back(stm_ptr);
                    continue;
                }
                if (stm_ptr->type == "jump" || stm_ptr->type == "jumpIf" || stm_ptr->type == "jumpIfNot" ||
                    stm_ptr->type == "label") {
                    continue;
                } else {
                    newStms.emplace_back(stm_ptr);
                }
            }
            block->stm_ptrs = newStms;
        }


        for (int i = 0; i < newAllBlocks.size() - 1; i++) {
            shared_ptr<BasicBlock> block = newAllBlocks[i];
            shared_ptr<Stm> lastStm = block->stm_ptrs[block->stm_ptrs.size() - 1];
            if (lastStm->type == "jump" || lastStm->type == "jumpIf" || lastStm->type == "jumpIfNot") {
                shared_ptr<BasicBlock> nextBlock = newAllBlocks[i + 1];
                int labelIndex = lastStm->result;
                if (nextBlock->isLabel(labelIndex)) {
                    block->stm_ptrs.pop_back();
                }
            }
        }


        allBlocks = newAllBlocks;
    }

    /**
     * 有些基本块只起到连接作用，需要把它们消除掉。
     */
    static void connectBlocks() {
        for (auto &block:allBlocks) {
            block->makeStright();
        }


//        removeUnArrived();
    }

    static void functionCheck() {
        vector<int> functionIndexs;
        for (auto &block:allBlocks) {
        }
    }

    /**
     * 如果一个函数内部的基本块通过调用自身函数而获得了一个nextBlock，就要把这个链接删掉。
     */
    static void devideWithFunction() {
        for (auto &block:allBlocks) {
            if (block->stm_ptrs.size() != 0 && block->functionAreaPtr != nullptr) {
                shared_ptr<Stm> lastStm = block->stm_ptrs[block->stm_ptrs.size() - 1];
                if (lastStm->type == "callFunction" && block->functionAreaPtr->functionIndex == lastStm->numberA) {
                    if (block->afterBlocks.size() == 2) {
                        block->afterBlocks.pop_back();
                        if (block->afterBlocks.size() != 1) {
                            cout << "这里出现了错误!“ 《" << endl;
                        }
                    }
                }
            }
            vector<shared_ptr<BasicBlock> > newNextBlocks;
            for (auto &nextBlock:block->afterBlocks) {
                if (nextBlock->functionAreaPtr == block->functionAreaPtr) {
                    newNextBlocks.emplace_back(nextBlock);
                }
            }
            block->afterBlocks = newNextBlocks;

        }
    }

    static void myCheck() {
        for (auto &block:allBlocks) {
            if (block->useOrDefIndex != block->useOrDefArray.size()) {
                cout << "基本块" << block->blockNumber << "发现了一个不一致 , useDefIndex = " << block->useOrDefIndex
                     << " useDefArraySize = "
                     << block->useOrDefArray.size() << endl;
                for (int j = 0; j < block->useOrDefArray.size(); j++) {
                    cout << "need : " << block->useOrDefArray[j]->type << " " << block->useOrDefArray[j]->index << endl;
                }
                for (int j = 0; j < block->useOrDefIndex; j++) {
                    cout << "real : " << block->useOrDefArray[j]->type << " " << block->useOrDefArray[j]->index << endl;
                }
            }
        }
    }

    static void analyseCertainValues() {
        for (auto block:allBlocks) {
            block->analyseCertainValues();
        }
    }

    /**
     * 分配全局寄存器
     */
    static void allocateGlobalRegister() {
        ConflictGraph graph;
        for (auto &block:allBlocks) {
            set<int> globalAndLocalIn;
            for (auto value:block->liveInGlobal2) {
                if (!block->hasCertainValue(value)) {
                    globalAndLocalIn.insert(value);
                }
            }
            for (auto value:block->liveInLocal2) {
                if (!block->hasCertainValue(value)) {
                    globalAndLocalIn.insert(value);
                }
            }
            graph.addConflictSet(globalAndLocalIn);

            for (int i = 0; i < block->useOrDefArray.size(); i++) {
                shared_ptr<UseOrDef> useOrDefPtr = block->useOrDefArray[i];
                if (useOrDefPtr->type == "def") {
                    block->defOneVar(useOrDefPtr->index);
                    if (!block->hasCertainValue(useOrDefPtr->index)) {
                        graph.newLivingValue(useOrDefPtr->index);
                    }
                } else if (useOrDefPtr->type == "use") {
                    block->useOneVar(useOrDefPtr->index);
                    if (!block->hasCertainValue(useOrDefPtr->index) && !block->isVarLive(useOrDefPtr->index)) {
                        graph.killOneValue(useOrDefPtr->index);
                    }
                } else if (useOrDefPtr->type == "register") {
                    block->cleanOneRegister(useOrDefPtr->index);
                } else {
                    cout << "编程错误！" << endl;
                }
            }
            block->useOrDefIndex = 0;
        }
        BasicBlock::value2Register = graph.allocateRegisters(BasicBlock::globalRegisters);
//        for (auto tuple:value2Register) {
//            cout << tuple.first << " " << tuple.second << endl;
//        }
    }

    static void outPutM(string fileName) {
        ofstream outFile(fileName, ios::out);
        for (auto block:allBlocks) {
            outFile << "block_" << block->blockNumber << endl;
            for (auto stm_ptr:block->stm_ptrs) {
                if (stm_ptr->type == "label") {
                    outFile << "label_" << stm_ptr->result << endl;
                } else if (stm_ptr->type == "jump") {
                    outFile << "jump : " << "label_" << stm_ptr->result << endl;
                } else if (stm_ptr->type == "jumpIf") {
                    outFile << "jumpIf @" << stm_ptr->numberA << " " << "label_" << stm_ptr->result << endl;
                } else if (stm_ptr->type == "jumpIfNot") {
                    outFile << "jumpIfNot @" << stm_ptr->numberA << " " << "label_" << stm_ptr->result << endl;
                } else if (stm_ptr->type == "declaration") {
                    if (stm_ptr->operation == "ConstUnarray") {
                        outFile << "const @" << stm_ptr->result << " = " << stm_ptr->numberA << endl;
                    } else if (stm_ptr->operation == "ConstArray") {
                        outFile << "const @" << stm_ptr->result << "[" << stm_ptr->numberA << "]" << endl;
                    } else if (stm_ptr->operation == "VarUnarray") {
                        outFile << "var @" << stm_ptr->result << endl;
                    } else if (stm_ptr->operation == "VarArray") {
                        outFile << "var @" << stm_ptr->result << "[" << stm_ptr->numberA << "]" << endl;
                    } else if (stm_ptr->operation == "Function") {
                        outFile << "function_" << stm_ptr->result << endl;
                    } else if (stm_ptr->operation == "ParaUnarray") {
                        outFile << "para @" << stm_ptr->result << endl;
                    } else if (stm_ptr->operation == "ParaArray") {
                        if (stm_ptr->numberA == -1) {
                            outFile << "para @" << stm_ptr->result << "[]" << endl;
                        } else {
                            outFile << "para @" << stm_ptr->result << "[]" << "[" << stm_ptr->numberA << "]" << endl;
                        }
                    } else if (stm_ptr->operation == "temp") {
                        outFile << "const @" << stm_ptr->result << " = " << stm_ptr->numberA << endl;  // 中间变量当成常量
                    } else {
                        outFile << stm_ptr->type << " " << stm_ptr->operation << " " << stm_ptr->numberA << " "
                                << stm_ptr->numberB << " " << stm_ptr->result << endl;
                    }
                } else if (stm_ptr->type == "assign") {
                    if (stm_ptr->operation == "ConstArray") {
                        outFile << "@" << stm_ptr->result << "[" << stm_ptr->numberB << "]" << " = " << stm_ptr->numberA
                                << endl;
                    } else if (stm_ptr->operation == "VarUnarrayInitial") {
                        outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << endl;
                    } else if (stm_ptr->operation == "VarArrayInitial") {
                        outFile << "@" << stm_ptr->result << "[" << stm_ptr->numberB << "]" << " = " << "@"
                                << stm_ptr->numberA << endl;
                    } else if (stm_ptr->operation == "Unarray") {
                        outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << endl;
                    } else if (stm_ptr->operation == "Array") {
                        outFile << "@" << stm_ptr->result << "[" << "@" << stm_ptr->numberA << "]" << " = " << "@"
                                << stm_ptr->numberB << endl;
                    } else {
                        outFile << stm_ptr->type << " " << stm_ptr->operation << " " << stm_ptr->numberA << " "
                                << stm_ptr->numberB << " " << stm_ptr->result << endl;
                    }
                } else if (stm_ptr->type == "setMain") {
                    outFile << "Main:" << endl;
                } else if (stm_ptr->type == "return") {
                    if (stm_ptr->result != -1) {
                        outFile << "return " << "@" << stm_ptr->result << endl;
                    } else {
                        outFile << "return" << endl;
                    }
                } else if (stm_ptr->type == "print") {
                    if (stm_ptr->operation == "str") {

                        outFile << "print" << " STR@" << stm_ptr->result << endl;
                    } else {
                        outFile << "print" << " " << "@" << stm_ptr->result << endl;
                    }
                } else if (stm_ptr->type == "getInt") {
                    outFile << "@" << stm_ptr->result << " = " << "getInt()" << endl;
                } else if (stm_ptr->type == "mul") {
                    if (stm_ptr->operation == "Const") {
                        outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " * "
                                << stm_ptr->numberB << endl;
                    } else if (stm_ptr->operation == "") {
                        outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " * "
                                << "@" << stm_ptr->numberB << endl;
                    } else {
                        outFile << stm_ptr->type << " " << stm_ptr->operation << " " << stm_ptr->numberA << " "
                                << stm_ptr->numberB << " " << stm_ptr->result << endl;
                    }
                } else if (stm_ptr->type == "div") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " / " << "@"
                            << stm_ptr->numberB << endl;
                } else if (stm_ptr->type == "mode") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " % " << "@"
                            << stm_ptr->numberB << endl;
                } else if (stm_ptr->type == "add") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " + " << "@"
                            << stm_ptr->numberB << endl;
                } else if (stm_ptr->type == "sub") {
                    if (stm_ptr->operation == "ConstSub") {
                        outFile << "@" << stm_ptr->result << " = " << stm_ptr->numberA << " - " << "@"
                                << stm_ptr->numberB << endl;
                    } else if (stm_ptr->operation == "") {
                        outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " - " << "@"
                                << stm_ptr->numberB << endl;
                    } else {
                        outFile << stm_ptr->type << " " << stm_ptr->operation << " " << stm_ptr->numberA << " "
                                << stm_ptr->numberB << " " << stm_ptr->result << endl;
                    }
                } else if (stm_ptr->type == "not") {
                    outFile << "@" << stm_ptr->result << " = " << "!" << "@" << stm_ptr->numberA << endl;
                } else if (stm_ptr->type == "equalCheck") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " == " << "@"
                            << stm_ptr->numberB << endl;
                } else if (stm_ptr->type == "notEqualCheck") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " != " << "@"
                            << stm_ptr->numberB << endl;
                } else if (stm_ptr->type == "ge") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << " >= " << "@"
                            << stm_ptr->numberB << endl;
                } else if (stm_ptr->type == "gt") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << ">" << "@"
                            << stm_ptr->numberB
                            << endl;
                } else if (stm_ptr->type == "lt") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << "<" << "@"
                            << stm_ptr->numberB
                            << endl;
                } else if (stm_ptr->type == "le") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << "<=" << "@"
                            << stm_ptr->numberB
                            << endl;
                } else if (stm_ptr->type == "setPara") {
                    if (stm_ptr->operation == "Array") {
                        outFile << "push array " << "@" << stm_ptr->result << "[" << "@" << stm_ptr->numberA << "]"
                                << endl;
                    } else {
                        outFile << "push " << "@" << stm_ptr->result << endl;
                    }
                } else if (stm_ptr->type == "load") {
                    outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << "[@" << stm_ptr->numberB
                            << "]"
                            << endl;
                } else if (stm_ptr->type == "function") {
                    if (stm_ptr->operation == "start") {
                        outFile << "function_" << stm_ptr->result << endl;
                    } else if (stm_ptr->operation == "end") {
                        outFile << "function_end" << endl;
                    }
                } else if (stm_ptr->type == "loadFrom") {
                    outFile << "@" << stm_ptr->result << " = @" << stm_ptr->numberA << "[@" << stm_ptr->numberB << "]"
                            << endl;
                } else if (stm_ptr->type == "callFunction") {
                    outFile << "@" << stm_ptr->result << " = function_" << stm_ptr->numberA << "()" << endl;
                } else if (stm_ptr->type == "calculateAddress") {
                    outFile << "@" << stm_ptr->result << " = @" << stm_ptr->numberA << "[@" << stm_ptr->numberB << "]"
                            << endl;
                } else {
                    outFile << "未定义: " << stm_ptr->type << " " << stm_ptr->operation << " " << stm_ptr->numberA << " "
                            << stm_ptr->numberB
                            << " " << stm_ptr->result << endl;
                }
            }
        }
        outFile.close();
    }

};


#endif //COMPILER_OPTIMIZER_H
