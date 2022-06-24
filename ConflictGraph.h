//
// Created by HP on 2021/12/14.
//

#ifndef COMPILER_CONFLICTGRAPH_H
#define COMPILER_CONFLICTGRAPH_H

#include <vector>
#include <map>
#include <set>
#include <memory>
#include <iostream>

using namespace std;

struct VarNode {
    int valueIndex;
    /**
     * 冲突变量
     */
    set<shared_ptr<VarNode> > conflictNodes;

    set<int> conflictRegister;

    int register_ = -1;//  给它分配的寄存器

    VarNode(int valueIndex) {
        this->valueIndex = valueIndex;
    }

    void addConflictNode(shared_ptr<VarNode> &anotherNode) {
        conflictNodes.insert(anotherNode);
    }

    void addConflictRegister(int color) {
        conflictRegister.insert(color);
    }

    int conflictAmount() {
        return (int) conflictNodes.size();
    }

    int findUseAbleRegister(set<int> allRegisters) {
        for (auto &varNode:conflictNodes) {
            if (allRegisters.count(varNode->register_)) {
                allRegisters.erase(varNode->register_);
            }
        }
        if (!allRegisters.empty()) {
            return *allRegisters.begin();
        } else {
            return -1;
        }
    }

    int allocateRegister(set<int> &allRegisters) {
        register_ = findUseAbleRegister(allRegisters);
        return register_;
    }

};


class ConflictGraph {

    map<int, shared_ptr<VarNode> > value2Node;

    /**
     * 拥有不同冲突数量的结点
     */
    map<int, set<shared_ptr<VarNode> > > amount2Nodes;

    int mostConflictAmount = -1;

private:


    bool isEnd() {
        for (int i = 0; i <= mostConflictAmount; i++) {
            if (amount2Nodes.count(i)) {
                if (!amount2Nodes[i].empty()) {
                    return false;
                }
            }
        }
        return true;
    }

    void buildAmountMap() {
        for (auto &tuple:value2Node) {
            int amount = tuple.second->conflictAmount();
            if (!amount2Nodes.count(amount)) {
                amount2Nodes[amount] = {};
            }
            amount2Nodes[amount].insert(tuple.second);
            if (mostConflictAmount == -1 || amount > mostConflictAmount) {
                mostConflictAmount = amount;
            }
        }
        for (int i = 0; i <= mostConflictAmount; i++) {
            if (!amount2Nodes.count(i)) {
                amount2Nodes[i] = {};
            }
        }

    }

    shared_ptr<VarNode> findOneNodeToAllocate(int registerAmount) {
        shared_ptr<VarNode> targetNode = nullptr;
        for (int i = registerAmount - 1; i >= 0; i--) {
            if (amount2Nodes.count(i) && !amount2Nodes[i].empty()) {  // 可能寄存器比最大冲突数量还多
                for (auto &new_ptr:amount2Nodes[i]) {
                    if (targetNode == nullptr || new_ptr->valueIndex > targetNode->valueIndex) {
                        targetNode = new_ptr;
                    }
                }
                break;
            }
        }
        if (targetNode != nullptr) {
            int thisAmount = targetNode->conflictAmount();
            amount2Nodes[thisAmount].erase(targetNode);

            for (auto &ptr:targetNode->conflictNodes) {
                int oriAmount = ptr->conflictAmount();
                ptr->conflictNodes.erase(targetNode);
                amount2Nodes[oriAmount].erase(ptr);
                amount2Nodes[oriAmount - 1].insert(ptr);
            }
        }
        return targetNode;
    }

    bool removeOneNode() {
        shared_ptr<VarNode> target = nullptr;
        for (int i = mostConflictAmount; i >= 0; i--) {
            if (!amount2Nodes[i].empty()) {
                for (auto &ptr:amount2Nodes[i]) {
                    if (target == nullptr || ptr->valueIndex > target->valueIndex) {
                        target = ptr;
                    }
                }
                break;
            }
        }
        if (target == nullptr) {
            return false;
        } else {
            amount2Nodes[target->conflictAmount()].erase(target);
            for (auto &ptr:target->conflictNodes) {
                int oriAmount = ptr->conflictAmount();
                ptr->conflictNodes.erase(target);
                amount2Nodes[oriAmount].erase(ptr);
                amount2Nodes[oriAmount - 1].insert(ptr);
            }
            return true;
        }
    }


public:

    set<int> livingSet;

    ConflictGraph() {

    }

    void addOneVar(int valueIndex) {
        if (!value2Node.count(valueIndex)) {
            value2Node[valueIndex] = make_shared<VarNode>(valueIndex);
        }
    }

    void addConflict(int value1, int value2) {
        addOneVar(value1);
        addOneVar(value2);
        if (value1 != value2) {
            value2Node[value1]->addConflictNode(value2Node[value2]);
            value2Node[value2]->addConflictNode(value2Node[value1]);
        }
    }

    void addConflictSet(set<int> &values) {
        for (auto value1:values) {
            for (auto value2:values) {
                addConflict(value1, value2);
            }
        }
        livingSet = values;
    }

    void newLivingValue(int newValue) {
        for (int livingValue:livingSet) {
            addConflict(livingValue, newValue);
        }
        livingSet.insert(newValue);
    }

    void killOneValue(int newValue) {
        if (livingSet.count(newValue)) {
            livingSet.erase(newValue);
        }
    }

    /**
     * 对所有的变量进行寄存器分配。返回分配结果。结果里没有的代表不给他分配寄存器。
     */
    map<int, int> allocateRegisters(set<int> registers) {

        buildAmountMap();
        map<int, int> value2Registers;
        vector<shared_ptr<VarNode> > toAllocateNodes;

        while (!isEnd()) {
            shared_ptr<VarNode> toAllocate = findOneNodeToAllocate((int) registers.size());
            if (toAllocate == nullptr) {
                if (!removeOneNode()) {
                    cout << "这里竟然不能成功删除一个节点" << endl;
                }
            } else {
                toAllocateNodes.emplace_back(toAllocate);
            }
        }
        for (int i = (int) toAllocateNodes.size() - 1; i >= 0; i--) {
            int register_ = toAllocateNodes[i]->allocateRegister(registers);
            if (register_ == -1) {
                cout << "错误！" << "寄存器图着色竟然又失败了!" << endl;
            } else {
                value2Registers[toAllocateNodes[i]->valueIndex] = register_;
            }
        }

        return value2Registers;
    }

};


#endif //COMPILER_CONFLICTGRAPH_H
