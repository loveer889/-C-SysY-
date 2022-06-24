//
// Created by HP on 2021/11/13.
//

#ifndef COMPILER_CODER_H
#define COMPILER_CODER_H

#include "Stm.h"
#include "Optimizer.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <tuple>
#include <sstream>
#include <algorithm>

class Memory {

public :
    int paraIndex = 0;  // 下一个即将定义的参数

    int retValue = 4;       // 有没有返回值都有这块地址
    int retAddr = 4;        // 默认有返回地址
    int prev_abp = 4;       // 默认有上层abp
    vector<tuple<int, int> > paras;  // 各参数占据的地址大小
    vector<tuple<int, int> > temps;  // 各临时变量占据的地址大小

    int getRetValueOffset() {
        return 0;
    }

    int getRetAddrOffset() {
        return getRetValueOffset() + retValue;
    }

    int getPrevApbOffset() {
        return getRetAddrOffset() + retAddr;
    }

    int getParaOffset(int paraIndex) {
        int offset = getPrevApbOffset() + prev_abp;
        for (auto para:paras) {
            if (get<0>(para) != paraIndex) {
                offset += 4;    // 参数都是数，即使是数组，也是地址，所以占用的空间都是4
            } else {
                return offset;
            }
        }
        return -1;  // 正常不会到这里。
    }

    int getTempOffset(int tempIndex) {
        int sum = getPrevApbOffset() + prev_abp + (int) paras.size() * 4;
        for (auto temp :temps) {
            if (get<0>(temp) != tempIndex) {
                sum += get<1>(temp);  // 临时变量可能是数，也可能是数组
            } else {
                return sum;
            }
        }
        return -1;  // 正常不会到这里
    }

    int getAllOffSet() {  // 当前活动记录的总大小

        int sum = retValue + retAddr + prev_abp + (int) paras.size() * 4;
        for (auto par:temps) {
            sum += get<1>(par);
        }
        return sum;
    }
};


class OneCode {
public:
    string type = "";
    int register1 = -1;
    int register2 = -1;
    int register3 = -1;
    int address = -1;
    string information = "";

    OneCode(string type_, int register1, int register2, int register3, int address, string information) {
        this->type = type_;
        this->register1 = register1;
        this->register2 = register2;
        this->register3 = register3;
        this->address = address;
        this->information = information;
    }

    /**
     * 把一个数拆成高16位和低16位
     */
    void splitNumber(int n, int &high, int &low) {
//        32767 是界限
        if (n <= 32767) {

        } else {

        }
    }

    string toStr() {
        string str;
        if (type == "lw") {
            str = lw();
        } else if (type == "sw") {
            str = sw();
        } else if (type == "sll") {
            str = sll();
        } else if (type == "add") {
            str = add();
        } else if (type == "addi") {
            str = addi();
        } else if (type == "mul") {
            str = mul();
        } else if (type == "sub") {
            str = sub();
        } else if (type == "subi") {
            str = subi();
        } else if (type == "div") {
            str = div();
        } else if (type == "mfhi") {
            str = mfhi();
        } else if (type == "seq") {
            str = seq();
        } else if (type == "sne") {
            str = sne();
        } else if (type == "sgt") {
            str = sgt();
        } else if (type == "sge") {
            str = sge();
        } else if (type == "slt") {
            str = slt();
        } else if (type == "slti") {
            str = slti();
        } else if (type == "sle") {
            str = sle();
        } else if (type == "li") {
            str = li();
        } else if (type == "syscall") {
            str = syscall();
        } else if (type == "label") {
            str = label();
        } else if (type == "j") {
            str = j();
        } else if (type == "bne") {
            str = bne();
        } else if (type == "beq") {
            str = beq();
        } else if (type == "jal") {
            str = jal();
        } else if (type == "jr") {
            str = jr();
        } else if (type == "move") {
            str = move();
        } else if (type == "mflo") {
            str = mflo();
        } else if (type == "mthi") {
            str = mthi();
        } else if (type == "mtlo") {
            str = mtlo();
        } else if (type == "srl") {
            str = srl();
        } else if (type == "srlv") {
            str = srlv();
        } else if (type == "sra") {
            str = sra();
        } else if (type == "srav") {
            str = srav();
        } else if (type == "muli") {
            str = muli();
        } else if (type == "mult") {
            str = mult();
        }
        if (str == "") {
            cout << "不能识别的指令" << type;
        }
        return str; // 正常不会到这里
    }

    string mult() {
        stringstream pool;
        pool << "mult $" << register1 << " $" << register2;
        return pool.str();
    }

    string srl() {
        stringstream pool;
        pool << "srl $" << register1 << " $" << register2 << " " << address;
        return pool.str();
    }

    string srlv() {
        stringstream pool;
        pool << "srlv $" << register1 << " $" << register2 << " $" << register3;
        return pool.str();
    }

    string sra() {
        stringstream pool;
        pool << "sra $" << register1 << " $" << register2 << " " << address;
        return pool.str();
    }

    string srav() {
        stringstream pool;
        pool << "srav $" << register1 << " $" << register2 << " $" << register3;
        return pool.str();
    }

    string mthi() {
        stringstream pool;
        pool << "mthi $" << register1;
        return pool.str();
    }

    string mtlo() {
        stringstream pool;
        pool << "mtlo $" << register1;
        return pool.str();
    }

    string mflo() {
        stringstream pool;
        pool << "mflo $" << register1;
        return pool.str();
    }

    string lw() {
        if (information == "offset") {  // lw $1 100($2)
            stringstream pool;
            pool << "lw $" << register1 << " " << address << "($" << register2 << ")";
            return pool.str();
        }
        return ""; // 正常情况不会遇到这里
    }

    string sw() {
        if (information == "offset") {
            stringstream pool;
            pool << "sw $" << register1 << " " << address << "($" << register2 << ")";
            return pool.str();
        }
        return "";
    }

    string sll() {
        if (information == "") {
            stringstream pool;
            pool << "sll " << "$" << register1 << " " << "$" << register2 << " " << address;
            return pool.str();
        }
        return "";
    }

    string add() {
        if (information == "") {
            stringstream pool;
            pool << "addu " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string addi() {
        if (information == "") {
            stringstream pool;
            pool << "addiu " << "$" << register1 << " " << "$" << register2 << " " << address;
            return pool.str();
        }
        return "";
    }

    string mul() {
        if (information == "") {
            stringstream pool;
            pool << "mul" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string muli() {
        stringstream pool;
        pool << "mul $" << register1 << " " << "$" << register2 << " " << address;
        return pool.str();
    }

    string mului() {
        stringstream pool;
        pool << "mulu $" << register1 << " " << "$" << register2 << " " << address;
        return pool.str();
    }

    string sub() {
        if (information == "") {
            stringstream pool;
            pool << "subu" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string subi() {
        if (information == "") {
            stringstream pool;
            pool << "subi" << " " << "$" << register1 << " " << "$" << register2 << " " << address;
            return pool.str();
        } else {
            return "";
        }
    }

    string div() {
        if (information == "") {
            stringstream pool;
            pool << "div" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        } else if (information == "only") {
            stringstream pool;
            pool << "div" << " " << "$" << register1 << " " << "$" << register2;
            return pool.str();
        }
        return "";
    }

    string mfhi() {
        if (information == "") {
            stringstream pool;
            pool << "mfhi" << " " << "$" << register1;
            return pool.str();
        }
        return "";
    }

    string seq() {
        if (information == "") {
            stringstream pool;
            pool << "seq" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string sne() {
        if (information == "") {
            stringstream pool;
            pool << "sne" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string sgt() {
        if (information == "") {
            stringstream pool;
            pool << "sgt" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string sge() {
        if (information == "") {
            stringstream pool;
            pool << "sge" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string slt() {
        if (information == "") {
            stringstream pool;
            pool << "slt" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string slti() {
        stringstream pool;
        pool << "slti" << " " << "$" << register1 << " $" << register2 << " " << address;
        return pool.str();
    }

    string sle() {
        if (information == "") {
            stringstream pool;
            pool << "sle" << " " << "$" << register1 << " " << "$" << register2 << " " << "$" << register3;
            return pool.str();
        }
        return "";
    }

    string move() {
        if (information == "") {
            stringstream pool;
            pool << "move" << " " << "$" << register1 << " " << "$" << register2;
            return pool.str();
        }
        return "";
    }

    string li() {
        if (information == "") {
            stringstream pool;
            pool << "li" << " " << "$" << register1 << " " << address;
            return pool.str();
        }
        return "";
    }


    string syscall() {
        return "syscall";
    }

    string label() {
        stringstream pool;
        pool << information << " : ";
        return pool.str();
    }

    string j() {
        stringstream pool;
        pool << "j" << " " << information;
        return pool.str();
    }

    string bne() {
        stringstream pool;
        pool << "bne" << " " << "$" << register1 << " " << "$" << register2 << " " << information;
        return pool.str();
    }

    string beq() {
        stringstream pool;
        pool << "beq" << " " << "$" << register1 << " " << "$" << register2 << " " << information;
        return pool.str();
    }

    string jal() {
        stringstream pool;
        pool << "jal" << " " << information;
        return pool.str();
    }

    string jr() {
        stringstream pool;
        pool << "jr" << " " << "$" << register1;
        return pool.str();
    }


};


class Coder {
private:

    map<int, int> addressCaches;

    map<int, int> registerBusy;
    map<int, int> registerVar;

    map<int, int> arrayInformations;  // 0 : 常量数组 1：非参数变量数组  2：参数数组

    map<int, int> isVarDirty;

    /**
     * 获取一个全局变量或者main中的变量对应的内存地址
     */
    int getGlobalVarAddress(int index) {
        if (addressCaches.count(index) == 1) {
            return addressCaches[index];
        } else {
            int nowAddress = 0;
            for (auto pair :globalDatas) {
                addressCaches[get<0>(pair)] = nowAddress;
                if (get<0>(pair) == index) {
                    return nowAddress;
                }
                nowAddress += get<1>(pair);
            }
            return -1; // 正常不会到这里
        }
    }

    int getFunctionVarAddress(int functionSection, int index) {
        int address1 = functionSections[functionSection]->getTempOffset(index);
        if (address1 != -1) {
            return address1;
        } else {
            return functionSections[functionSection]->getParaOffset(index);
        }
    }

    int allocateRegister() {
        vector<int> toAllocateRegisters;
        for (int i = 0; i < 32; i++) {
            if (registerBusy[i] == 0 && !nowBlock->isGlobalRegister(i)) {
                toAllocateRegisters.emplace_back(i);  // 只有不忙的非全局寄存器才可能被换出
            }
        }
        if (toAllocateRegisters.empty()) {
            cout << "错误！" << "寄存器不够用了" << endl;
        }
        int longest;
        int targetRegister = -1;
        for (auto register1 :toAllocateRegisters) {  // 对于每个寄存器，看看多久之后会用到这个寄存器。
            int valueIndex;
            if (registerVar.count(register1)) {
                valueIndex = registerVar[register1];
            } else {
                valueIndex = -1;
                registerVar[register1] = -1;
            }
            int l = nowBlock->futureUse(register1, valueIndex);
            if (targetRegister == -1 ||
                (longest != -1 && (l == -1 || l > longest))) {
                longest = l;
                targetRegister = register1;
            }
        }
        registerBusy[targetRegister]++;
        protectRegister(targetRegister);
        return targetRegister;
    }

    void freeRegister(int i) {
        registerBusy[i]--;
        int value = registerVar[i];
        if (!nowBlock->isVarLive(value)) {
            registerVar[i] = -1;
        } else if (!nowBlock->isUseFulHere(value) && !nowBlock->isGlobalRegister(i)) {
            protectRegister(i);
        }
    }

    void loadValueToCertainRegister(int register1, int valueIndex) {
        if (constValues.count(valueIndex) || BasicBlock::certainValues.count(valueIndex)) {
            codes.emplace_back(
                    make_shared<OneCode>("li", register1, -1, -1, BasicBlock::certainValues[valueIndex], ""));
            registerVar[register1] = valueIndex;
        } else {
            if (varsBelongsTo[valueIndex] == -1) {  // 如果是全局变量，那么一定存储在全局数据区
                int globalAddress = getGlobalVarAddress(valueIndex);
                codes.emplace_back(make_shared<OneCode>("lw", register1, 28, -1, globalAddress, "offset"));
                registerVar[register1] = valueIndex;
            } else {
                int functionAddressOffset = getFunctionVarAddress(varsBelongsTo[valueIndex], valueIndex);
                codes.emplace_back(make_shared<OneCode>("lw", register1, 30, -1, functionAddressOffset, "offset"));
                registerVar[register1] = valueIndex;
            }
        }
    }


    /**
     * 必须把变量弄到可以直接使用的寄存器里，不能是hi 或者 lo
     */
    int loadValueToRegister(int valueIndex) {
        if (this->type == "common") {
            nowBlock->useOneVar(valueIndex);
        } else if (this->type == "getVarsBelongsTo") {
            nowBlock->addUseVar(valueIndex, varsBelongsTo[valueIndex]);
            return 1;
        }

        for (int i = 0; i <= 33; i++) {
            if (registerVar[i] == valueIndex) {
                if (i == 32 || i == 33) {
                    registerVar[i] = -1;
                    int newRegister = allocateRegister();
                    if (i == 32) {
                        mfhi(newRegister);
                    } else {
                        mflo(newRegister);
                    }
                    registerVar[newRegister] = valueIndex;
                } else {
                    registerBusy[i]++;  // 如果找到了一个已经存储着这个变量的寄存器。
                    return i;
                }
            }
        }

        if (constValues.count(valueIndex) || BasicBlock::certainValues.count(valueIndex)) {
//            cout << valueIndex << "是已知量" << endl;
            int register1 = allocateRegister();
            codes.emplace_back(
                    make_shared<OneCode>("li", register1, -1, -1, BasicBlock::certainValues[valueIndex], ""));
            registerVar[register1] = valueIndex;
            return register1;
        } else {

            int globalRegister = nowBlock->getGlobalRegister(valueIndex);
            if (globalRegister != -1) {
//                cout << "变量" << valueIndex << "有全局寄存器" << endl;
                registerBusy[globalRegister]++;  // 用全局变量的时候，认为它一定在全局寄存器里，这是由冲突图保证的。
                registerVar[globalRegister] = valueIndex;
                return globalRegister;
            } else {
//                cout << "变量" << valueIndex << "无全局寄存器" << endl;
            }

            if (varsBelongsTo[valueIndex] == -1) {  // 如果是全局变量，那么一定存储在全局数据区
                int globalAddress = getGlobalVarAddress(valueIndex);
                int register1 = allocateRegister();
                // lw $1 100($2)
                codes.emplace_back(make_shared<OneCode>("lw", register1, 28, -1, globalAddress, "offset"));
                registerVar[register1] = valueIndex;

                return register1;
            } else {
                int functionAddressOffset = getFunctionVarAddress(varsBelongsTo[valueIndex], valueIndex);
                int register1 = allocateRegister();
                codes.emplace_back(make_shared<OneCode>("lw", register1, 30, -1, functionAddressOffset, "offset"));
                registerVar[register1] = valueIndex;
                return register1;
            }
        }
    }

    /**
     * TODO 把储存在寄存器里的变量弄到内存里(常量除外）
     */
    void protectRegisters(string type) {

//        cout << "保存寄存器模式: " << type << endl;

        if (type == "global") {
//            cout << "进入global" << endl;
            for (int i = 0; i < 32; i++) {
                int valueIndex = registerVar[i];
                if (valueIndex == -1) {
                    continue;
                }
                if (nowBlock->getGlobalRegister(valueIndex) != -1) {
                    continue;
                }
                if (!(constValues.count(valueIndex) || BasicBlock::certainValues.count(valueIndex)) &&
                    varsBelongsTo[valueIndex] == -1) {
                    protectRegister(i);
                }
//                registerVar[i] = -1;
            }
//            cout << "离开global" << endl;
        } else if (type == "unGlobalRegister") {
//            cout << "进入unGlobalRegister" << endl;
            for (int i = 0; i < 32; i++) {
                int valueIndex = registerVar[i];
                if (valueIndex == -1) {
                    continue;
                }
                if (!(constValues.count(valueIndex) || BasicBlock::certainValues.count(valueIndex)) &&
                    (varsBelongsTo[valueIndex] == -1) && !(nowBlock->isGlobalRegister(i))) {  // 这种模式不保存全局寄存器
                    protectRegister(i);
                }
            }
//            cout << "离开unGlobalRegister" << endl;
        } else if (type == "all") {  // 函数调用前保存所有寄存器
//            cout << "进入all" << endl;
            for (int i = 0; i < 32; i++) {
                int valueIndex = registerVar[i];
                if (valueIndex == -1) {
                    continue;
                }
                if (!(constValues.count(valueIndex) || BasicBlock::certainValues.count(valueIndex))) {
                    protectRegister(i);
                }
//                registerVar[i] = -1;
            }
//            cout << "离开all " << endl;
        }
    }

    /**
     * 把调用函数前保存的变量重新加载到寄存器里。
     */
    void reLoadRegisters() {
        for (int value:protectedValues) {
//            cout << "reload处理到变量" << value << endl;
            if (nowBlock->isVarLive(value)) {
                loadValueToCertainRegister(nowBlock->getGlobalRegister(value), value);
            }
        }
    }

    int logIndex = 0;

    void protectRegisterToCertainValue(int register1, int valueIndex) {
        isVarDirty[valueIndex] = 0;
        if (varsBelongsTo[valueIndex] == -1) {  // 要给一个全局变量赋值
            int globalAddress = getGlobalVarAddress(valueIndex);
            codes.emplace_back(make_shared<OneCode>("sw", register1, 28, -1, globalAddress, "offset"));
        } else {
            int functionAddressOffset = getFunctionVarAddress(varsBelongsTo[valueIndex], valueIndex);
            codes.emplace_back(make_shared<OneCode>("sw", register1, 30, -1, functionAddressOffset, "offset"));
        }
    }

    /**
     * 获取一个原本就在寄存器里的值
     */
    int findValueInRegister(int valueIndex) {
        for (int i = 0; i <= 31; i++) {
            if (registerVar[i] == valueIndex) {
//                if (this->type == "common") {
//                    cout << "D添加繁忙寄存器" << i << endl;
//                }
                registerBusy[i]++;
                return i;
            }
        }
        return -1;
    }

    /**
     * 把某个寄存器里存储的变量存到内存里。
     * @param registerID
     */
    void protectRegister(int register1) {
//        cout << "试图保存寄存器" << register1 << endl;
        int valueIndex = registerVar[register1];
        registerVar[register1] = -1;
        if (valueIndex == -1 || BasicBlock::certainValues.count(valueIndex) ||
            (isVarDirty[valueIndex] == 0 && (nowBlock->getGlobalRegister(valueIndex) == -1))
            || (varsBelongsTo[valueIndex] == -1 && nowBlock->getGlobalRegister(valueIndex) != -1)) {
            //  一个变量脏不脏，是用来辅助判断临时寄存器的保存的。对于全局寄存器，让它保存必须保存。
            //  任何情况下，不保存被分配全局寄存器的全局变量。
            return;  // 无须保护
        }
        if (nowBlock->isVarLive(valueIndex)) {  // 只保存活跃变量
            if (nowBlock->getGlobalRegister(valueIndex) != -1) {
                protectedValues.insert(valueIndex);
            }
            if (register1 == 32 || register1 == 33) {
                int tempRegister = -1;
                for (int i = 0; i <= 31; i++) {
                    if (reservedRegisters.count(i) ||
                        find(paraRegisters.begin(), paraRegisters.end(), i) != paraRegisters.end()) {
                        continue;
                    } else {
                        tempRegister = i;
                        break;
                    }
                }
                if (tempRegister == -1) {
                    cout << "竟然没找到临时寄存器" << endl;
                }
                if (register1 = 32) {
                    mfhi(register1);
                    registerVar[register1] = registerVar[32];
                    registerVar[32] = -1;
                } else {
                    mflo(register1);
                    registerVar[register1] = registerVar[33];
                    registerVar[33] = -1;
                }
                protectRegister(register1);
            } else {
                if (nowBlock->isUseFulHere(valueIndex) &&
                    (nowBlock->getGlobalRegister(valueIndex) == -1)) {
                    // 如果这个变量在当前块里还可能被用到，那么优先保存到cache里。全局变量不保存到cache。
                    int l = nowBlock->futureUse(-1, valueIndex);
                    if (registerVar[32] == -1 &&
                        (nowBlock->futureUse(32, -1) > l || nowBlock->futureUse(32, -1) == -1)) {
                        mthi(register1);
                        registerVar[32] = valueIndex;
                    } else if (registerVar[33] == -1 &&
                               (nowBlock->futureUse(33, -1) > l || nowBlock->futureUse(33, -1) == -1)) {
                        mtlo(register1);
                        registerVar[33] = valueIndex;
                    } else {
                        protectRegisterToCertainValue(register1, valueIndex);
                    }
                } else {
                    protectRegisterToCertainValue(register1, valueIndex);
                }
            }
        }
    }


    /**
     * 把一些寄存器弄干净，因为下一步操作必须用这个寄存器,所以也把这些寄存器的状态设置为正忙。
     */
    void makeRegisterClean(set<int> registers) {
        for (auto register1:registers) {
            if (this->type == "getVarsBelongsTo") {
                nowBlock->addCleanRegister(register1);

            } else if (this->type == "common") {
                nowBlock->cleanOneRegister(register1);
            }
        }

        for (auto register1:registers) {
            if (registerBusy[register1]) {
                if (this->type == "common") {
                    cout << "错误! 寄存器" << register1 << "正忙，它的内容是 : " << registerVar[register1] << endl;
                }
            } else {
                registerBusy[register1]++;
            }
        }

        for (auto register1 :registers) {
            if (registerVar[register1] == -1) {
                continue;
            }
            //  理论上算法能保证到达这里的时候hi和low里面都没有值

            int valueIndex = registerVar[register1];
            if (nowBlock->isVarLive(valueIndex)) {
                if (nowBlock->isUseFulHere(valueIndex)) {
                    int freeRegister = findFreeRegisters(nowBlock->futureUse(-1, valueIndex));
                    if (freeRegister == -1) {
                        protectRegister(register1);
                    } else {
                        if (register1 == 32) {
                            mfhi(freeRegister);
                        } else if (register1 == 33) {
                            mflo(freeRegister);
                        } else {
                            moveTo(freeRegister, register1);
                        }
                        registerVar[freeRegister] = valueIndex;


                        registerVar[register1] = -1;
                    }
                } else {
                    protectRegister(register1);
                }
            } else {
                registerVar[register1] = -1;
            }
        }
    }

    /**
    * 找到一个能无需保存就可用的寄存器。不能在avoidRegisters里。
    */
    int findFreeRegisters(int l) {
        for (int i = 0; i <= 31; i++) {
            if (reservedRegisters.count(i) || registerBusy[i] || nowBlock->isGlobalRegister(i)) {
                continue;
            }
            if (registerVar[i] == -1) {
                return i;
            } else if (!nowBlock->isVarLive(registerVar[i])) {
                registerVar[i] = -1;
                return i;
            } else if (!nowBlock->isUseFulHere(registerVar[i])) {
                protectRegister(i);
                return i;
            }
        }
        return -1;
    }


    void setRegisterToValue(int register1, int valueIndex) {
        isVarDirty[valueIndex] = 1;
        if (this->type == "getVarsBelongsTo") {
            nowBlock->addDefVar(valueIndex, varsBelongsTo[valueIndex]);  // 要修改的变量不可能是常量。
            return;
        } else if (this->type == "common") {
            nowBlock->defOneVar(valueIndex);
        }

        if (nowBlock->getGlobalRegister(valueIndex) != -1) {
            if (register1 == nowBlock->getGlobalRegister(valueIndex)) {
                registerVar[register1] = valueIndex;
                return;
            } else {
                moveTo(nowBlock->getGlobalRegister(valueIndex), register1);
                registerVar[nowBlock->getGlobalRegister(valueIndex)] = valueIndex;
                return;
            }
        }

        if (!nowBlock->isVarLive(valueIndex)) {
            isVarDirty[valueIndex] = 0;
            return;
        } else if (!nowBlock->isUseFulHere(valueIndex)) {
            protectRegisterToCertainValue(register1, valueIndex);
            for (int i = 0; i <= 33; i++) {
                if (registerVar[i] == valueIndex) {
                    registerVar[i] = -1;
                }
            }
            isVarDirty[valueIndex] = 0;
            return;
        }

        for (int i = 0; i <= 31; i++) {
            if (registerVar[i] == valueIndex) {
                if (!nowBlock->isVarLive(registerVar[register1]) &&
                    !(nowBlock->isGlobalRegister(register1))) {
                    registerVar[register1] = valueIndex;
                    registerVar[i] = -1;
                    return;
                } else {
                    moveTo(i, register1);
                    return;
                }
            }
        }

        for (int i :reservedRegisters) {
            if (i == register1) {
                int newRegister = allocateRegister();
                moveTo(newRegister, register1);
                registerVar[i] = -1;
                registerVar[newRegister] = valueIndex;
                registerBusy[newRegister]--;
                return;
            }
        }

        if (!nowBlock->isUseFulHere(registerVar[register1]) && !nowBlock->isGlobalRegister(register1)) {
            protectRegister(register1);
            isVarDirty[registerVar[register1]] = 0;
            registerVar[register1] = valueIndex;
        } else {
            int newRegister = findFreeRegisters(nowBlock->futureUse(-1, valueIndex));
            if (newRegister != -1) {
                moveTo(newRegister, register1);


                registerVar[newRegister] = valueIndex;
            } else {
                // 这里可能是之前一直没有暴露出来的bug
                protectRegisterToCertainValue(register1, valueIndex);
                isVarDirty[valueIndex] = 0;
            }
        }
    }

    void saveToGlobalOffset(int register1, int globalOffset) {
        codes.emplace_back(make_shared<OneCode>("sw", register1, 28, -1, globalOffset, "offset"));
    }

    void saveToFunctionOffset(int register1, int functionOffset) {
        codes.emplace_back(make_shared<OneCode>("sw", register1, 30, -1, functionOffset, "offset"));
    }

    void saveToRegisterOffset(int valueRegister, int baseRegister, int address) {
        codes.emplace_back(make_shared<OneCode>("sw", valueRegister, baseRegister, -1, address, "offset"));
    }

    void loadFromFunctionOffset(int register1, int offSet) {
        codes.emplace_back(make_shared<OneCode>("lw", register1, 30, -1, offSet, "offset"));
    }

    int loadFromFunctionOffset(int offset) {
        int register1 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("lw", register1, 30, -1, offset, "offset"));
        return register1;
    }

    int loadFromRegisterOffset(int baseRegister, int offSet) {
        int valueRegister = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("lw", valueRegister, baseRegister, -1, offSet, "offset"));
        return valueRegister;
    }

    void loadFromRegisterOffset(int resultRegister, int baseRegister, int offSet) {
        codes.emplace_back(make_shared<OneCode>("lw", resultRegister, baseRegister, -1, offSet, "offset"));
        registerBusy[resultRegister]++;
    }


    int shiftLeft(int register1, int len) {
        //sll $t1 $t2 10
        int register2 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("sll", register2, register1, -1, len, ""));
        return register2;
    }

    int shiftLogicalRight(int register1, int number2, string type) {
        int resultRegister = allocateRegister();
        if (type == "const") {
            codes.emplace_back(make_shared<OneCode>("srl", resultRegister, register1, -1, number2, ""));
        } else {
            codes.emplace_back(make_shared<OneCode>("srlv", resultRegister, register1, number2, -1, ""));
        }
        return resultRegister;
    }

    int shiftArithmeticRight(int register1, int number2, string type) {
        int resultRegister = allocateRegister();
        if (type == "const") {
            codes.emplace_back(make_shared<OneCode>("sra", resultRegister, register1, -1, number2, ""));
        } else {
            codes.emplace_back(make_shared<OneCode>("srav", resultRegister, register1, number2, -1, ""));
        }
        return resultRegister;
    }

    int add(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("add", register3, register1, register2, -1, ""));
        return register3;
    }

    void add(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("add", resultRegister, register1, register2, -1, ""));
        registerBusy[resultRegister]++;
    }

    int addConst(int register1, int constValue) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("addi", register3, register1, -1, constValue, ""));
        return register3;
    }

    void addConst(int resultRegister, int register1, int constValue) {
        codes.emplace_back(make_shared<OneCode>("addi", resultRegister, register1, -1, constValue, ""));


//        if (this->type == "common") {
//            cout << "addConst添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int mul(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("mul", register3, register1, register2, -1, ""));
        return register3;
    }

    int muli(int register1, int constNumber) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("muli", register3, register1, -1, constNumber, ""));
        return register3;
    }

    void mult(int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("mult", register1, register2, -1, -1, ""));
    }

    int mului(int register1, int constNumber) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("mului", register3, register1, -1, constNumber, ""));
        return register3;
    }


    void mul(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("mul", resultRegister, register1, register2, -1, ""));

//        if (this->type == "common") {
//            cout << "mul添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int sub(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("sub", register3, register1, register2, -1, ""));
        return register3;
    }

    void sub(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("sub", resultRegister, register1, register2, -1, ""));

//        if (this->type == "common") {
//            cout << "sub添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int subConst(int register1, int constValue) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("subi", register3, register1, -1, constValue, ""));
        return register3;
    }

    void subConst(int resultRegister, int register1, int constValue) {
        codes.emplace_back(make_shared<OneCode>("subi", resultRegister, register1, -1, constValue, ""));

//        if (this->type == "common") {
//            cout << "subConst添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int div(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("div", register3, register1, register2, -1, ""));
        return register3;
    }

    void div(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("div", resultRegister, register1, register2, -1, ""));
        registerBusy[resultRegister]++;
    }

    int mode(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("div", register1, register2, -1, -1, "only"));
        codes.emplace_back(make_shared<OneCode>("mfhi", register3, -1, -1, -1, ""));
        return register3;
    }

    void mode(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("div", register1, register2, -1, -1, "only"));
        codes.emplace_back(make_shared<OneCode>("mfhi", resultRegister, -1, -1, -1, ""));
        registerBusy[resultRegister]++;
    }

    void mfhi(int register1) {
        codes.emplace_back(make_shared<OneCode>("mfhi", register1, -1, -1, -1, ""));
    }

    void mflo(int register1) {
        codes.emplace_back(make_shared<OneCode>("mflo", register1, -1, -1, -1, ""));
    }

    void mthi(int register1) {
        codes.emplace_back(make_shared<OneCode>("mthi", register1, -1, -1, -1, ""));
    }

    void mtlo(int register1) {
        codes.emplace_back(make_shared<OneCode>("mtlo", register1, -1, -1, -1, ""));
    }

    int not_(int register1) {
        int register2 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("seq", register2, register1, 0, -1, ""));
        return register2;
    }

    void not_(int resultRegister, int register1) {
        codes.emplace_back(make_shared<OneCode>("seq", resultRegister, register1, 0, -1, ""));

//        if (this->type == "common") {
//            cout << "not_添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int equal_(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("seq", register3, register1, register2, -1, ""));
        return register3;
    }

    void equal_(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("seq", resultRegister, register1, register2, -1, ""));


//        if (this->type == "common") {
//            cout << "equal_添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int not_equal(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("sne", register3, register1, register2, -1, ""));
        return register3;
    }

    void not_equal(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("sne", resultRegister, register1, register2, -1, ""));


//        if (this->type == "common") {
//            cout << "not_equal添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int gt(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("sgt", register3, register1, register2, -1, ""));
        return register3;
    }

    void gt(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("sgt", resultRegister, register1, register2, -1, ""));

//        if (this->type == "common") {
//            cout << "gt添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int ge(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("sge", register3, register1, register2, -1, ""));
        return register3;
    }

    void ge(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("sge", resultRegister, register1, register2, -1, ""));


//        if (this->type == "common") {
//            cout << "ge添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }


    int lt(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("slt", register3, register1, register2, -1, ""));
        return register3;
    }

    void lt(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("slt", resultRegister, register1, register2, -1, ""));

//        if (this->type == "common") {
//            cout << "lt添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int ltConst(int register1, int constValue) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("slti", register3, register1, -1, constValue, ""));
        return register3;
    }

    void ltConst(int resultRegister, int register1, int constValue) {
        codes.emplace_back(make_shared<OneCode>("slti", resultRegister, register1, -1, constValue, ""));

//        if (this->type == "common") {
//            cout << "ltConst添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    int le(int register1, int register2) {
        int register3 = allocateRegister();
        codes.emplace_back(make_shared<OneCode>("sle", register3, register1, register2, -1, ""));
        return register3;
    }

    void le(int resultRegister, int register1, int register2) {
        codes.emplace_back(make_shared<OneCode>("sle", resultRegister, register1, register2, -1, ""));

//        if (this->type == "common") {
//            cout << "le添加繁忙寄存器" << resultRegister << endl;
//        }

        registerBusy[resultRegister]++;
    }

    void moveTo(int toRegister, int fromRegister) {
        codes.emplace_back(make_shared<OneCode>("move", toRegister, fromRegister, -1, -1, ""));
    }

    void setConst(int register1, int constNumber) {
        codes.emplace_back(make_shared<OneCode>("li", register1, -1, -1, constNumber, ""));
    }

    int setConst(int constNumber) {
        int register1 = allocateRegister();
        setConst(register1, constNumber);
        return register1;
    }

    void syscall(int number) {
        setConst(2, number);
        codes.emplace_back(make_shared<OneCode>("syscall", -1, -1, -1, -1, ""));
    }


    void setLabel(int labelIndex) {
        stringstream pool;
        pool << "label_" << labelIndex;
        codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
    }

    void jumpTo(int labelIndex) {
        stringstream pool;
        pool << "label_" << labelIndex;
        codes.emplace_back(make_shared<OneCode>("j", -1, -1, -1, -1, pool.str()));
    }

    void jumpIf(int register1, int labelIndex) {
        stringstream pool;
        pool << "label_" << labelIndex;
        // 非零的值都认为是真值
        codes.emplace_back(make_shared<OneCode>("bne", register1, 0, -1, -1, pool.str()));
    }

    void jumpIfNot(int register1, int labelIndex) {
        stringstream pool;
        pool << "label_" << labelIndex;
        //  非0的值都是真值，都不跳转
        codes.emplace_back(make_shared<OneCode>("beq", register1, 0, -1, -1, pool.str()));
    }

    void jumpAndLink(int labelIndex) {
        stringstream pool;
        pool << "label_" << labelIndex;
        codes.emplace_back(make_shared<OneCode>("jal", -1, -1, -1, -1, pool.str()));
    }

    void jumpToRegister(int register1) {
        codes.emplace_back(make_shared<OneCode>("jr", register1, -1, -1, -1, ""));
    }

public:

    /**
     * 当前的模式：真正要输出代码，还是模拟生成一次，以便获得各个基本块的变量访问序列。
     */
    string type;
    /**
     * 当前所在基本块
     */
    shared_ptr<BasicBlock> nowBlock;

    /**
     *            指令     寄存器1    寄存器2    寄存器3    其他信息
     */
    vector<shared_ptr<OneCode> > codes;  // 即将输出的mips代码，可供最后优化

    /**
     * 调用函数之前保存的全局寄存器的值，用于重新加载。
     */
    set<int> protectedValues;

    /**
     * 常量的值
     */
    map<int, int> constValues;

    /**
     * 各函数对应的动态内存AP
     * 包括函数内部的变量、变量数组
     */
    map<int, shared_ptr<Memory> > functionSections;

    /**
     * 全局数据，包括所有常量数组、字符串;全局变量、变量数组;main函数里的变量、变量数组。
     * <index,length>
     */
    vector<tuple<int, int> > globalDatas;

    map<int, vector<int> > constArrays;  // 所有全局常量数组中存储的值。最后要输出到.data


    /**
     * 各个变量分别属于哪个函数。-1表示全局或main。
     * 这里不包含常数数组的信息。因为常熟数组一定在.data部分。
     * 不包含普通常数，因为普通常数要解读为字面量。
     */
    map<int, int> varsBelongsTo;

    int nowFunctionIndex = -1;  // 当前在哪个函数里，如果不在任何函数里，那么就为-1，代表全局。main不算函数。

    set<int> reservedRegisters = {0, 1, 28, 30};
    vector<int> paraRegisters = {4, 5, 6, 7, 8};  // 先不用hi和Lo传参

    Coder() {
        protectedValues = {};
        for (int i = 0; i <= 33; i++) {
            if (reservedRegisters.count(i)) {
                registerBusy[i] = 1;  // 保留寄存器繁忙
            } else {
                registerBusy[i] = 0;  // 默认寄存器空闲。
            }
            registerVar[i] = -1;
            //  32 : hi
            //  33 : lo
        }
        setConst(28, 268500992);  // 把全局寄存器设置到固定位置
        setConst(30, 268697600);

    }

    void solveDeclaration(shared_ptr<Stm> &ptr) {
        int index = ptr->result;
        if (ptr->operation == "temp") {     // 计算过程中的临时变量，需要有对应的内存地址
            if (nowFunctionIndex == -1) {
                globalDatas.emplace_back(tuple<int, int>(index, 4));
                varsBelongsTo[index] = -1;
            } else {
                functionSections[nowFunctionIndex]->temps.emplace_back(tuple<int, int>(index, 4));
                varsBelongsTo[index] = nowFunctionIndex;
            }
        } else if (ptr->operation == "ConstUnarray") {
            constValues[ptr->result] = ptr->numberA;
        } else if (ptr->operation == "ConstArray") {
            //  常数数组统一存到全局空间
            globalDatas.emplace_back(tuple<int, int>(ptr->result, ptr->numberA * 4));
            arrayInformations[ptr->result] = 0;
            varsBelongsTo[ptr->result] = -1;  // 常量数组也“属于”全局数据区
        } else if (ptr->operation == "VarUnarray") {
            if (nowFunctionIndex == -1) {
                globalDatas.emplace_back(tuple<int, int>(index, 4));
                varsBelongsTo[index] = -1;
            } else {
                functionSections[nowFunctionIndex]->temps.emplace_back(tuple<int, int>(index, 4));
                varsBelongsTo[index] = nowFunctionIndex;
            }
        } else if (ptr->operation == "VarArray") {
            if (nowFunctionIndex == -1) {
                globalDatas.emplace_back(tuple<int, int>(index, ptr->numberA * 4));
                varsBelongsTo[index] = -1;
            } else {
                functionSections[nowFunctionIndex]->temps.emplace_back(tuple<int, int>(index, ptr->numberA * 4));
                varsBelongsTo[index] = nowFunctionIndex;
            }
            arrayInformations[index] = 1;  // 非参数数组
        } else if (ptr->operation == "ParaUnarray") {   // 参数的大小均为4
            functionSections[nowFunctionIndex]->temps.emplace_back(tuple<int, int>(index, 4));
            varsBelongsTo[index] = nowFunctionIndex;
            int paraIndex = functionSections[nowFunctionIndex]->paraIndex;
            if (this->type == "common") {
                if (paraIndex < paraRegisters.size()) {  // 看看是第几个参数，从而判断它到底是在寄存器里，还是在内存里。
                    if (nowBlock->getGlobalRegister(index) != -1) {
                        moveTo(nowBlock->getGlobalRegister(index), paraRegisters[paraIndex]);
//                        cout << "把寄存器" << nowBlock->getGlobalRegister(index) << "的值设置为" << index << endl;
                        registerVar[nowBlock->getGlobalRegister(index)] = index;
                    } else {
                        registerVar[paraRegisters[paraIndex]] = index;
                    }
                    isVarDirty[index] = 1;
                } else {
                    if (nowBlock->getGlobalRegister(index) != -1) {
                        loadValueToCertainRegister(nowBlock->getGlobalRegister(index), index);
                    }
                }
                functionSections[nowFunctionIndex]->paraIndex++;
            }
        } else if (ptr->operation == "ParaArray") {     // 参数的大小均为4
            functionSections[nowFunctionIndex]->temps.emplace_back(tuple<int, int>(index, 4));
            varsBelongsTo[index] = nowFunctionIndex;
            arrayInformations[index] = 2;  // 参数数组
            int paraIndex = functionSections[nowFunctionIndex]->paraIndex;
            if (this->type == "common") {
                if (paraIndex < paraRegisters.size()) {
                    if (nowBlock->getGlobalRegister(index) != -1) {
                        moveTo(nowBlock->getGlobalRegister(index), paraRegisters[paraIndex]);
                        registerVar[nowBlock->getGlobalRegister(index)] = index;
                    } else {
                        registerVar[paraRegisters[paraIndex]] = index;
                    }
                    isVarDirty[index] = 1;
                } else {
                    if (nowBlock->getGlobalRegister(index) != -1) {
                        loadValueToCertainRegister(nowBlock->getGlobalRegister(index), index);
                    }
                }
                functionSections[nowFunctionIndex]->paraIndex++;
            }
        }
    }

    /**
     * 尝试着把一个寄存器的值弄到另一个寄存器里。
     *
     */
    bool tryMoveTo(int fromRegister, int toRegister, int midRegister) {
        if (fromRegister == toRegister) {
            return true;
        }
        if (registerVar[toRegister] != -1) {
            return false;
        }

        if (fromRegister == 32 || fromRegister == 33) {
            if (toRegister == 32 || toRegister == 33) {
                if (midRegister == -1) {
                    return false;
                }
                if (fromRegister == 32) {
                    mfhi(midRegister);
                } else {
                    mflo(midRegister);
                }
                if (toRegister == 32) {
                    mthi(midRegister);
                } else {
                    mtlo(midRegister);
                }
            } else {
                if (fromRegister == 32) {
                    mfhi(toRegister);
                } else {
                    mflo(toRegister);
                }
            }
        } else {
            if (toRegister == 32) {
                mthi(fromRegister);
            } else if (toRegister == 33) {
                mtlo(toRegister);
            } else {
                moveTo(toRegister, fromRegister);
            }
        }

        registerVar[toRegister] = registerVar[fromRegister];
        registerVar[fromRegister] = -1;
        return true;
    }

    /**
     * 尝试把某个寄存器里的变量存储到参数区
     */
    bool trySaveTo(int fromRegister, int offSet, int midRegister) {
        int realFromRegister = fromRegister;
        if (fromRegister == 32 || fromRegister == 33) {
            if (midRegister == -1) {
                return false;
            } else {
                if (fromRegister == 32) {
                    mfhi(midRegister);
                } else {
                    mflo(midRegister);
                }
                realFromRegister = midRegister;
            }
        }
        if (realFromRegister) {
            cout << "trySaveTo 这里的问题" << endl;
        }
        saveToFunctionOffset(realFromRegister, offSet);


        protectRegister(fromRegister);
        return true;
    }

    bool tryAssignEmptyRegister(int register_, map<int, int> &paraNeedRegisters, int &emptyRegister) {
        if (emptyRegister != -1) {
            return true;
        }
        if (reservedRegisters.count(register_)) {
            return false;
        }
        for (auto tuple:paraNeedRegisters) {
            if (tuple.second == register_) {
                return false;
            }
        }
        emptyRegister = register_;
        return true;
    }

    bool checkParaOK(map<int, int> &paraNeedRegisters, set<int> &doneParas) {
        for (auto tuple:paraNeedRegisters) {
            if (!doneParas.count(tuple.first)) {
                return false;
            }
        }
        return true;
    }

    bool moveToOnce(map<int, int> &paraNeedRegisters, map<int, int> &para2Register,
                    set<int> &doneParas, int midRegister) {
//        cout << "进入moveToOnce" << "打印所有寄存器状态  : " << endl;
//        for (auto tuple:paraNeedRegisters) {
//            cout << "变量" << tuple.first << "需要放到寄存器" << tuple.second << " ;";
//        }
//        cout << endl;
//        for (int i = 0; i <= 33; i++) {
//            if (registerVar[i] != -1) {
//                cout << "寄存器" << i << "里面有变量" << registerVar[i] << " ;";
//            }
//        }
//        cout << endl;
//        for (auto tuple:para2Register) {
//            cout << "变量" << tuple.first << "在寄存器" << tuple.second << "里;";
//        }
//        cout << endl;

        bool find = false;
        for (auto tuple:para2Register) {
            if (!doneParas.count(tuple.first) && tuple.second != -1) {
//                cout << "变量" << tuple.first << "需要转移,它在" << tuple.second << "里" << endl;
                int fromRegister = tuple.second;
                int toRegister = paraNeedRegisters[tuple.first];
                if (fromRegister != toRegister) {
//                    cout << "即将用这三个参数调用tryMoveTo" << fromRegister << "  " << toRegister << "  " << midRegister << endl;
                    if (tryMoveTo(fromRegister, toRegister, midRegister)) {
                        find = true;
                        doneParas.insert(tuple.first);
                        para2Register[tuple.first] = toRegister;
                    }
                }
            } else {
//                if (doneParas.count(tuple.first)) {
//                    cout << "做好的参数集合已经包含了" << tuple.first << endl;
//                }
//                if (tuple.second == -1) {
//                    cout << "参数" << tuple.first << "对应的寄存器为-1" << endl;
//                }
            }
        }
//        if (find) {
//            cout << "成功" << endl;
//        } else {
//            cout << "失败" << endl;
//        }

        return find;
    }

    int findTempRegister(map<int, int> &paraNeedRegisters, set<int> notRegisters = {}) {
        for (int i = 0; i <= 31; i++) {
            if (reservedRegisters.count(i)) {
                continue;
            } else if (registerVar[i] != -1) {
                continue;
            } else if (notRegisters.count(i)) {
                continue;
            }
            bool isOk = true;
            for (auto tuple:paraNeedRegisters) {
                if (tuple.second == i) {
                    isOk = false;
                }
            }
            if (isOk) {
                return i;
            }
        }
        cout << "错误！ 竟然找不到临时寄存器" << endl;
        int n;
        cin >> n;
        return -1;
    }

    void solveParaByLoad(map<int, int> &paraNeedRegisters, map<int, int> &para2Register, set<int> &doneParas) {
        for (auto tuple:paraNeedRegisters) {
            if (doneParas.count(tuple.first) == 0 && para2Register[tuple.first] == -1) {
                int targetRegister = paraNeedRegisters[tuple.first];
                if (targetRegister == 32 || targetRegister == 33) {
                    //  不用hi和Lo传参了。
//                    int tempRegister = findTempRegister(paraNeedRegisters);
//                    loadValueToCertainRegister(tempRegister, tuple.first);
//                    if (targetRegister == 32) {
//                        mthi(tempRegister);
//                    } else {
//                        mtlo(tempRegister);
//                    }
                } else {
                    if (nowBlock->getGlobalRegister(tuple.first) == -1) {
                        loadValueToCertainRegister(targetRegister, tuple.first);
                    } else {
                        moveTo(targetRegister, nowBlock->getGlobalRegister(tuple.first));
                    }
                }
                doneParas.insert(tuple.first);
                para2Register[tuple.first] = paraNeedRegisters[tuple.first];
                registerVar[paraNeedRegisters[tuple.first]] = tuple.first;
            }
        }
    }

    /**
     * 尝试着消除一个环，返回是否已经消除了所有环。
     */
    bool solveOneCircle(map<int, int> &paraNeedRegisters, map<int, int> &para2Register, set<int> &doneParas) {

        int tempRegister1 = findTempRegister(paraNeedRegisters);
//        cout << "正在试图解决环路问题" << "找到的临时寄存器为" << tempRegister1 << endl;
        while (moveToOnce(paraNeedRegisters, para2Register, doneParas, tempRegister1));  // 确保处于环状或者完成状态

        int tempRegister2 = findTempRegister(paraNeedRegisters, {tempRegister1});
        if (checkParaOK(paraNeedRegisters, doneParas)) {
            return true;
        }
//        cout << "环路中检查1_" << endl;
//        for (int u = 0; u <= 33; u++) {
//            if (registerVar[u] != -1) {
//                cout << "寄存器" << u << "的值是" << registerVar[u] << endl;
//            }
//        }

        for (auto tuple:para2Register) {
            if (doneParas.count(tuple.first) == 0) {
                moveTo(tempRegister1, tuple.second);
                registerVar[tempRegister1] = tuple.first;
                registerVar[tuple.second] = -1;
                para2Register[tuple.first] = tempRegister1;
                break;
            }
        }
//        cout << "环路中检查2_" << endl;
//        for (int u = 0; u <= 33; u++) {
//            if (registerVar[u] != -1) {
//                cout << "寄存器" << u << "的值是" << registerVar[u] << endl;
//            }
//        }
//        for (auto tuple :paraNeedRegisters) {
//            cout << "变量" << tuple.first << "需要寄存器" << tuple.second << "记录中它在寄存器" << para2Register[tuple.first] << "里"
//                 << endl;
//        }

        while (moveToOnce(paraNeedRegisters, para2Register, doneParas, tempRegister2));  // 确保处于环状或者完成状态
        return checkParaOK(paraNeedRegisters, doneParas);
    }

    int solveAboutFunction(shared_ptr<BasicBlock> &block, int i) {
        shared_ptr<Stm> &ptr = block->stm_ptrs[i];

        if (ptr->type == "setMain") {

        } else if (ptr->type == "return") {
            if (ptr->operation == "main") {
                syscall(10);
            } else if (ptr->operation == "") {
                int returnValueIndex = ptr->result;
                if (returnValueIndex != -1) {  // 有返回值

                    if (this->type == "common") {
                        cout << "即将因为返回值而清除寄存器2" << endl;
                    }
                    makeRegisterClean({2});
                    if (this->type == "getVarsBelongsTo") {
                        int returnRegister = loadValueToRegister(returnValueIndex);
                        saveToFunctionOffset(returnRegister, 0);  // 存储返回值到内存
                        freeRegister(returnRegister);
                    } else {
                        nowBlock->useOneVar(returnValueIndex);
                        bool find = false;
                        for (int register_ = 0; register_ <= 33; register_++) {
                            if (registerVar[register_] == returnValueIndex) {
                                if (!tryMoveTo(register_, 2, -1)) {
                                    cout << "异常！竟然无法把变量弄到v0里" << endl;
                                }
                                find = true;
                                break;
                            }
                        }
                        if (!find) {
                            loadValueToCertainRegister(2, returnValueIndex);
                        }
                    }
                    freeRegister(2);
                }

//                if (this->type == "common") {
//                    cout << "保存全局寄存器" << endl;
//                }

                protectRegisters("global");
//                if (nowBlock->blockNumber == 14) {
//                    cout << "执行到这里" << this->type << endl;
//                }
                makeRegisterClean({31});
                freeRegister(31);
                loadFromFunctionOffset(31, 4);
                loadFromFunctionOffset(30, 8);
                jumpToRegister(31);
            }
        } else if (ptr->type == "setPara") {
            if (this->type == "getVarsBelongsTo") {  // 如果正在观察寄存器的使用，那么仍然采用传统方法
                int offSet;
                if (nowFunctionIndex == -1) {  // 如果在main函数里调用某个函数，那么当前需要从偏移0开始构造活动记录
                    offSet = 0;
                } else {
                    offSet = functionSections[nowFunctionIndex]->getAllOffSet();
                }
                offSet += 8;  // 越过返回值、返回地址。因为返回地址需要函数自己维护。
                offSet += 4;  // 再越过abp之后，才到参数区
                int register2 = loadValueToRegister(ptr->result);
                saveToFunctionOffset(register2, offSet);
                offSet += 4;
                freeRegister(register2);
                int j = i + 1;
                for (; block->stm_ptrs[j]->type == "setPara"; j++) {
                    shared_ptr<Stm> ptr2 = block->stm_ptrs[j];
                    int register3 = loadValueToRegister(ptr2->result);
                    saveToFunctionOffset(register3, offSet);
                    offSet += 4;
                    freeRegister(register3);
                }
                return j;
            } else {

                map<int, int> paraNeedRegister;  // 应当由寄存器传的参数应当对应的寄存器
                map<int, int> paraNeedOffset;  // 应当由内存传递的参数应当对应的偏移。
                map<int, int> para2Register;  // 当前参数对应的寄存器，-1表示不在寄存器里。
                int emptyRegister = -1;       // 空白中转寄存器
                set<int> doneParas;           // 已经设置好的参数
//                cout << "设置参数前检查" << endl;
//                for (int u = 0; u <= 33; u++) {
//                    if (registerVar[u] != -1) {
//                        cout << "寄存器" << u << "的值是" << registerVar[u] << endl;
//                    }
//                }
                int paraRegisterIndex = 0;
                int paraOffset;  // 越过返回值、返回地址,abp
                if (nowFunctionIndex != -1) {  // 如果在main函数里调用某个函数，那么当前需要从偏移0开始构造活动记录
                    paraOffset = 12 + functionSections[nowFunctionIndex]->getAllOffSet();
                } else {
                    paraOffset = 12;
                }
                int j;
                for (j = i; block->stm_ptrs[j]->type == "setPara"; j++) {
                    if (j - i < paraRegisters.size()) {
                        paraNeedRegister[block->stm_ptrs[j]->result] = paraRegisters[paraRegisterIndex++];
                        paraOffset += 4;
                    } else {
                        paraNeedOffset[block->stm_ptrs[j]->result] = paraOffset;
                        paraOffset += 4;
                    }
                    nowBlock->useOneVar(block->stm_ptrs[j]->result);
                    para2Register[block->stm_ptrs[j]->result] = -1;
                }

                for (int register_ = 0; register_ <= 33; register_++) {
                    if (registerVar[register_] != -1) {  // 这个寄存器里有值
                        int oriValue = registerVar[register_];
                        if (paraNeedRegister.count(oriValue)) {
                            if (tryMoveTo(register_, paraNeedRegister[oriValue], emptyRegister)) {
                                doneParas.insert(oriValue);
                                para2Register[oriValue] = paraNeedRegister[oriValue];
                                tryAssignEmptyRegister(register_, paraNeedRegister, emptyRegister);
                            } else {
                                para2Register[oriValue] = register_;
                            }
                        } else if (paraNeedOffset.count(oriValue)) {
                            int fromRegister = register_;
                            if (trySaveTo(register_, paraNeedOffset[registerVar[register_]], emptyRegister)) {
                                doneParas.insert(oriValue);
                                tryAssignEmptyRegister(register_, paraNeedRegister, emptyRegister);
                            } else {
                                para2Register[oriValue] = register_;
                            }
                        } else {
                            //  调用函数的时候是有可能有非参数存活的，一方面是因为全局寄存器里肯定有值，另一方面是因为：？
//                            if (oriValue == 3) {
//                                cout << "设置参数时发现了变量3在" << register_ << "里" << endl;
//                            }


                            protectRegister(register_);  // 能保护Hi和Lo吗


                        }
                    } else {
                        tryAssignEmptyRegister(register_, paraNeedRegister, emptyRegister);
                    }
                }



//                for (auto tuple:paraNeedRegister) {
//                    cout << "变量" << tuple.first << "需要的寄存器是" << tuple.second << " ; " << "记录中它在寄存器"
//                         << para2Register[tuple.first] << "里" << endl;
//                    if (doneParas.count(tuple.first)) {
//                        cout << "变量" << tuple.first << "已做好" << endl;
//                    } else {
//                        cout << "变量" << tuple.first << "未做好" << endl;
//                    }
//                }

                for (int register_ = 0; register_ <= 33; register_++) {
                    int oriValue = registerVar[register_];
                    if (paraNeedOffset.count(oriValue) && !doneParas.count(oriValue)) {
                        if (trySaveTo(register_, paraNeedOffset[oriValue], emptyRegister)) {
                            doneParas.insert(oriValue);
                        } else {
                            cout << "意外，这里竟然无法存储到函数区" << endl;
                        }
                    }
                }

                for (auto tuple:paraNeedOffset) {
                    if (!doneParas.count(tuple.first)) {
                        loadValueToCertainRegister(emptyRegister, tuple.first);
                        if (emptyRegister == -1) {
                            cout << "错误！" << "这里竟然没找到空白寄存器" << endl;
                        }
                        saveToFunctionOffset(emptyRegister, tuple.second);
                        doneParas.insert(tuple.first);
                        registerVar[emptyRegister] = -1;
                    }
                }
//                cout << "中间寄存器为" << emptyRegister << endl;

                while (moveToOnce(paraNeedRegister, para2Register, doneParas, emptyRegister));
                solveParaByLoad(paraNeedRegister, para2Register, doneParas);
                int circleIndex = 0;
                while (!solveOneCircle(paraNeedRegister, para2Register, doneParas));


                return j;
            }


        } else if (ptr->type == "callFunction") {
            int offSet;
            if (nowFunctionIndex == -1) {  // 如果在main函数里调用某个函数，那么当前需要从偏移0开始构造活动记录
                offSet = 0;
            } else {
                offSet = functionSections[nowFunctionIndex]->getAllOffSet();
            }
            offSet += 8;  // 越过返回值、返回地址。因为返回地址需要函数自己维护。
            saveToFunctionOffset(30, offSet);  // apb
//            if (this->type == "common") {
//                cout << "保护所有寄存器" << endl;
//            }

            protectRegisters("all");  // 这里假设调用的函数里会把
            if (nowFunctionIndex != -1) {
                addConst(30, 30, functionSections[nowFunctionIndex]->getAllOffSet());
            }
            jumpAndLink(ptr->numberA);
        } else if (ptr->type == "getReturnValue") {
            reLoadRegisters();
            if (ptr->result != -1) {  // 有可能只有返回点而没有返回值
                if (this->type == "getVarsBelongsTo") {
                    int offSet;
                    if (nowFunctionIndex == -1) {  // 如果在main函数里调用某个函数，那么当前需要从偏移0开始构造活动记录
                        offSet = 0;
                    } else {
                        offSet = functionSections[nowFunctionIndex]->getAllOffSet();
                    }
                    int valueRegister = loadFromFunctionOffset(offSet);
                    setRegisterToValue(valueRegister, ptr->result);
                    freeRegister(valueRegister);
                } else {

                    nowBlock->defOneVar(ptr->result);
                    registerVar[2] = ptr->result;
                    isVarDirty[ptr->result] = 1;
                    if (nowBlock->getGlobalRegister(ptr->result) != -1) {
                        moveTo(nowBlock->getGlobalRegister(ptr->result), 2);
                        registerVar[2] = -1;
                        registerVar[nowBlock->getGlobalRegister(ptr->result)] = ptr->result;
                    }
                }
            }
            protectedValues = {};
        } else if (ptr->type == "function") {
            if (ptr->operation == "start") {
                setLabel(nowFunctionIndex);
                saveToFunctionOffset(31, 4);  // 保存返回地址
                shared_ptr<Memory> newFunction = make_shared<Memory>();
                functionSections[nowFunctionIndex] = newFunction;


            } else if (ptr->operation == "end") {
                for (int register_ = 0; register_ < 32; register_++) {
                    if (nowBlock->isGlobalRegister(register_)
                        && registerVar[register_] != -1
                        && varsBelongsTo[register_] != -1) {
                        registerVar[register_] = -1;
                    }
                }
            }
        }
        return -1;
    }

    void solveAssign(shared_ptr<Stm> &ptr) {
        if (ptr->operation == "ConstArray") {
            if (constArrays.count(ptr->result) == 0) {
                constArrays[ptr->result] = {};
            }
            constArrays[ptr->result].emplace_back(ptr->numberA);
        } else if (ptr->operation == "VarUnarrayInitial") {
            int varIndex = ptr->result;
            int valueIndex = ptr->numberA;
            // 用一个变量的值给令一个变量赋值
//            if (valueIndex == 6) {
//                cout << "正在用变量6给变量" << varIndex << "赋值" << endl;
//                if (nowBlock->hasCertainValue(valueIndex)) {
//                    cout << "变量" << valueIndex << "有确定值" << nowBlock->getCertainValue(valueIndex);
//                } else {
//                    cout << "变量" << valueIndex << "无确定值" << endl;
//                }
//            }

            int register1 = loadValueToRegister(valueIndex);  // register1存储了要赋的值
            setRegisterToValue(register1, varIndex);
            freeRegister(register1);
        } else if (ptr->operation == "VarArrayInitial") {
            int varArrayIndex = ptr->result;
            int valueIndex = ptr->numberA;
            int number = ptr->numberB;
            int addressOffset;
            if (varsBelongsTo[varArrayIndex] == -1) {
                addressOffset = getGlobalVarAddress(varArrayIndex);
            } else {
                addressOffset = getFunctionVarAddress(varsBelongsTo[varArrayIndex], varArrayIndex);
            }
            addressOffset += 4 * number;
            int register1 = loadValueToRegister(valueIndex);
            if (varsBelongsTo[varArrayIndex] == -1) {
                saveToGlobalOffset(register1, addressOffset);
            } else {
                saveToFunctionOffset(register1, addressOffset);
            }
            freeRegister(register1);
        } else if (ptr->operation == "Unarray") {
            int varIndex = ptr->result;
            int valueIndex = ptr->numberA;
            int register1 = loadValueToRegister(valueIndex);
            setRegisterToValue(register1, varIndex);
            freeRegister(register1);
        } else if (ptr->operation == "Array") {
            int arrayIndex = ptr->result;   // 要给哪个数组赋值
            int offSetIndex = ptr->numberA;
            int valueIndex = ptr->numberB;
            if (arrayInformations[arrayIndex] == 2) {  // 如果这个所谓的数组是参数数组
                int arrayAddressRegister = loadValueToRegister(arrayIndex);  // 数组的真实地址
                int offSetRegister = loadValueToRegister(offSetIndex);
                int realOffSetRegister = shiftLeft(offSetRegister, 2);
                freeRegister(offSetRegister);
                int realAddressRegister = add(arrayAddressRegister, realOffSetRegister);
                freeRegister(arrayAddressRegister);
                freeRegister(realOffSetRegister);
                int valueRegister = loadValueToRegister(valueIndex);
                saveToRegisterOffset(valueRegister, realAddressRegister, 0);
                freeRegister(valueRegister);
                freeRegister(realAddressRegister);
            } else {
                int register1 = loadValueToRegister(offSetIndex);
                int register2 = shiftLeft(register1, 2);
                freeRegister(register1);
                int baseRegister;
                if (varsBelongsTo[arrayIndex] == -1) {
                    baseRegister = add(28, register2);
                } else {
                    baseRegister = add(30, register2);
                }
                freeRegister(register2);
                int arrayOffset;
                if (varsBelongsTo[arrayIndex] == -1) {
                    arrayOffset = getGlobalVarAddress(arrayIndex);
                } else {
                    arrayOffset = getFunctionVarAddress(varsBelongsTo[arrayIndex], arrayIndex);
                }
                int valueRegister = loadValueToRegister(valueIndex);
                saveToRegisterOffset(valueRegister, baseRegister, arrayOffset);
                freeRegister(valueRegister);
                freeRegister(baseRegister);
            }
        }
    }

    void solveIO(shared_ptr<Stm> &ptr) {
        if (ptr->type == "print") {
            if (ptr->operation == "value") {
                int valueIndex = ptr->result;
                if (this->type == "common") {
                    cout << "即将打印值，此时寄存器2的繁忙状态是" << registerBusy[2] << endl;
                }
                makeRegisterClean({2, 4});
                int register1 = loadValueToRegister(valueIndex);
                moveTo(4, register1);
                freeRegister(register1);
                freeRegister(2);
                freeRegister(4);
                syscall(1);
            } else if (ptr->operation == "str") {
                int strIndex = ptr->result;
                string str_to_print = Stm::int2str[strIndex];  // 要打印的字符串
                int len = (int) str_to_print.size() + 1;
                for (char ch:str_to_print) {
                    if (ch == '\\') {  // 转义字符会重复计算，需要弄掉。
                        len--;
                    }
                }

                if ((len) % 4 != 0) {
                    int remain = 4 - (len) % 4;  // 剩余几个位置
                    len = len + remain;
                    if (remain == 1) {
                        len += 4;
                    }
                }
                globalDatas.emplace_back(tuple<int, int>(strIndex, len));
                if (this->type == "common") {
                    cout << "即将打印字符串" << endl;
                }
                makeRegisterClean({2, 4});
                freeRegister(4);
                addConst(4, 28, getGlobalVarAddress(strIndex));
                freeRegister(2);
                freeRegister(4);
//                if (this->type == "common") {
//                    cout << "释放寄存器4后，它的繁忙状态是: " << registerBusy[4] << endl;
//                }
                syscall(4);
            }
        } else if (ptr->type == "getInt") {
            if (this->type == "common") {
                cout << "即将getInt而清除寄存器2" << endl;
            }
            makeRegisterClean({2});
            syscall(5);
            int targetIndex = ptr->result;
            setRegisterToValue(2, targetIndex);
            freeRegister(2);

            if (this->type == "common") {
                cout << "getInt结束时寄存器2的状态是 : " << registerBusy[2] << endl;
            }
        }
    }


    int log2_upper(int x) {
        int y = x;
        int count = 0;
        while (y != 0) {
            y = y >> 1;
            count += 1;
        }
        if ((1 << (count - 1)) == x) {
            return count - 1;
        } else {
            return count;
        }
    }


    tuple<long long, int, int> choose_multiplier(int d, long long prec) {
        long long l = log2_upper(d);
        long long sh_post = l;
        long long m_low = ((long long) 1 << (32 + l)) / d;
        long long m_high = (((long long) 1 << (32 + l)) + ((long long) 1 << (32 + l - prec))) / d;
        while (((m_low / 2) < (m_high / 2)) && sh_post > 0) {
            m_low = m_low / 2;
            m_high = m_high / 2;
            sh_post = sh_post - 1;
        }
        return {m_high, sh_post, l};
    }

    /**
     * 论文算法中的MULSH过程，其中x2_register存储着被除数
     */
    int MULSH(int x1, int x2_register) {
        int temp1_register = muli(x2_register, x1);
        mfhi(temp1_register);
        return temp1_register;
    }

    int XSIGN(int n_register) {
        return shiftArithmeticRight(n_register, 31, "const");
    }

    /**
     * 一个变量除以一个常量，结果存到某个寄存器里。
     */
    int myDiv(int n_register, int d) {
        int _d_ = d >= 0 ? d : -d;
        int q_register;
        if (_d_ != 1) {
            tuple<long long, int, int> tuple_result = choose_multiplier(_d_, 31);
            long long m = get<0>(tuple_result);
            int sh_post = get<1>(tuple_result);
            int l = get<2>(tuple_result);
            if (_d_ == (1 << (long long) l)) {
                int temp1_register = shiftArithmeticRight(n_register, l - 1, "const");
                int temp2_register = shiftLogicalRight(temp1_register, 32 - l, "const");
                freeRegister(temp1_register);
                int temp3_register = add(n_register, temp2_register);
                freeRegister(temp2_register);
                q_register = shiftArithmeticRight(temp3_register, l, "const");
                freeRegister(temp3_register);
            } else if (m < ((long long) 1 << (31))) {
                makeRegisterClean({32, 33});
                int mulsh_register = MULSH(m, n_register);
                int temp1_register = shiftArithmeticRight(mulsh_register, sh_post, "const");
                freeRegister(mulsh_register);
                int temp2_register = XSIGN(n_register);
                q_register = sub(temp1_register, temp2_register);
                freeRegister(temp1_register);
                freeRegister(temp2_register);
                freeRegister(32);
                freeRegister(33);
            } else {
                makeRegisterClean({32, 33});
                int mulsh_register = MULSH(m - ((long long) 1 << 32), n_register);
                int temp1_register = add(n_register, mulsh_register);
                freeRegister(mulsh_register);
                int temp2_register = shiftArithmeticRight(temp1_register, sh_post, "const");
                freeRegister(temp1_register);
                int xsign_register = XSIGN(n_register);
                q_register = sub(temp2_register, xsign_register);
                freeRegister(temp2_register);
                freeRegister(xsign_register);
                freeRegister(32);
                freeRegister(33);
            }
        } else {
            q_register = allocateRegister();
            moveTo(q_register, n_register);
//            q_register = n_register;
        }
        if (d < 0) {
            freeRegister(q_register);
            q_register = sub(0, q_register);
//            q_register = sub(0, q_register);
        }
        return q_register;
    }


    void solveOperate(shared_ptr<Stm> &ptr) {  // + - * / %
        if (ptr->type == "mul") {
            if (ptr->operation == "Const") {
                makeRegisterClean({32, 33});
                int register1 = setConst(ptr->numberB);
                int register2 = loadValueToRegister(ptr->numberA);
                int register3;
                if (this->type == "getVarsBelongsTo" || nowBlock->getGlobalRegister(ptr->result) == -1) {
                    register3 = mul(register1, register2);
                } else if (this->type == "common") {
                    register3 = nowBlock->getGlobalRegister(ptr->result);
                    mul(register3, register1, register2);
                }
                freeRegister(register1);
                freeRegister(register2);
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
                freeRegister(32);
                freeRegister(33);
            } else if (ptr->operation == "") {
                makeRegisterClean({32, 33});
                int register1 = loadValueToRegister(ptr->numberA);
                int register2 = loadValueToRegister(ptr->numberB);
                int register3;
                if (this->type == "getVarsBelongsTo" || nowBlock->getGlobalRegister(ptr->result) == -1) {
                    register3 = mul(register1, register2);
                } else if (this->type == "common") {
                    register3 = nowBlock->getGlobalRegister(ptr->result);
                    mul(register3, register1, register2);
                }
                freeRegister(register1);
                freeRegister(register2);
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
                freeRegister(32);
                freeRegister(33);
            }
        } else if (ptr->type == "add") {
            if (this->type == "getVarsBelongsTo") {
                int register1 = loadValueToRegister(ptr->numberA);
                int register2 = loadValueToRegister(ptr->numberB);
                int register3 = add(register1, register2);
                freeRegister(register1);
                freeRegister(register2);
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
            } else if (this->type == "common") {
                int register3 = nowBlock->getGlobalRegister(ptr->result);
                if (nowBlock->hasCertainValue(ptr->numberA) && nowBlock->hasCertainValue(ptr->numberB)) {
                    int r1 = findValueInRegister(ptr->numberA);
                    int r2 = findValueInRegister(ptr->numberB);
                    if (r1 != -1 && r2 != -1) {
                        nowBlock->useOneVar(ptr->numberA);
                        nowBlock->useOneVar(ptr->numberB);
                        if (register3 == -1) {
                            register3 = add(r1, r2);
                        } else {
                            add(register3, r1, r2);
                        }
                        freeRegister(r1);
                        freeRegister(r2);
                    } else if (r1 != -1) {
                        if (register3 == -1) {
                            register3 = addConst(r1, nowBlock->getCertainValue(ptr->numberB));
                        } else {
                            addConst(register3, r1, nowBlock->getCertainValue(ptr->numberB));
                        }
                        nowBlock->useOneVar(ptr->numberA);
                        nowBlock->useOneVar(ptr->numberB);
                        freeRegister(r1);
                    } else if (r2 != -1) {
                        if (register3 == -1) {
                            register3 = addConst(r2, nowBlock->getCertainValue(ptr->numberA));
                        } else {
                            addConst(register3, r2, nowBlock->getCertainValue(ptr->numberA));
                        }
                        nowBlock->useOneVar(ptr->numberA);
                        nowBlock->useOneVar(ptr->numberB);
                        freeRegister(r2);
                    } else {
                        int constA = nowBlock->getCertainValue(ptr->numberA);
                        int constB = nowBlock->getCertainValue(ptr->numberB);
                        if (constA > constB) {
                            r1 = loadValueToRegister(ptr->numberA);
                            nowBlock->useOneVar(ptr->numberB);
                            if (register3 == -1) {
                                register3 = addConst(r1, constB);
                            } else {
                                addConst(register3, r1, constB);
                            }
                            freeRegister(r1);
                        } else {
                            nowBlock->useOneVar(ptr->numberA);
                            r2 = loadValueToRegister(ptr->numberB);
                            if (register3 == -1) {
                                register3 = addConst(r2, constA);
                            } else {
                                addConst(register3, r2, constA);
                            }
                            freeRegister(r2);
                        }
                    }
                } else if (nowBlock->hasCertainValue(ptr->numberA)) {
                    nowBlock->useOneVar(ptr->numberA);
                    int register2 = loadValueToRegister(ptr->numberB);
                    if (register3 == -1) {
                        register3 = addConst(register2, nowBlock->getCertainValue(ptr->numberA));
                    } else {
                        addConst(register3, register2, nowBlock->getCertainValue(ptr->numberA));
                    }
                    freeRegister(register2);
                } else if (nowBlock->hasCertainValue(ptr->numberB)) {
                    int register1 = loadValueToRegister(ptr->numberA);
                    nowBlock->useOneVar(ptr->numberB);
                    if (register3 == -1) {
                        register3 = addConst(register1, nowBlock->getCertainValue(ptr->numberB));
                    } else {
                        addConst(register3, register1, nowBlock->getCertainValue(ptr->numberB));
                    }
                    freeRegister(register1);
                } else {
                    int register1 = loadValueToRegister(ptr->numberA);
                    int register2 = loadValueToRegister(ptr->numberB);
                    if (register3 == -1) {
                        register3 = add(register1, register2);
                    } else {
                        add(register3, register1, register2);
                    }
                    freeRegister(register1);
                    freeRegister(register2);
                }
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
            }
        } else if (ptr->type == "sub") {
            if (ptr->operation == "ConstSub") {
                int register1 = setConst(ptr->numberA);
                int register2 = loadValueToRegister(ptr->numberB);
                int register3 = nowBlock->getGlobalRegister(ptr->result);
                if (register3 == -1) {
                    register3 = sub(register1, register2);
                } else {
                    sub(register3, register1, register2);
                }
                freeRegister(register1);
                freeRegister(register2);
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
            } else if (ptr->operation == "") {
                if (this->type == "getVarsBelongsTo") {
                    int register1 = loadValueToRegister(ptr->numberA);
                    int register2 = loadValueToRegister(ptr->numberB);
                    int register3 = sub(register1, register2);
                    freeRegister(register1);
                    freeRegister(register2);
                    setRegisterToValue(register3, ptr->result);
                    freeRegister(register3);
                } else if (this->type == "common") {
                    int register3 = nowBlock->getGlobalRegister(ptr->result);
                    if (nowBlock->hasCertainValue(ptr->numberB)) {
                        int register1 = loadValueToRegister(ptr->numberA);
                        nowBlock->useOneVar(ptr->numberB);
                        if (register3 == -1) {
                            register3 = subConst(register1, nowBlock->getCertainValue(ptr->numberB));
                        } else {
                            subConst(register3, register1, nowBlock->getCertainValue(ptr->numberB));
                        }
                        freeRegister(register1);
                    } else {
                        int register1 = loadValueToRegister(ptr->numberA);
                        int register2 = loadValueToRegister(ptr->numberB);
                        if (register3 == -1) {
                            register3 = sub(register1, register2);
                        } else {
                            sub(register3, register1, register2);
                        }
                        freeRegister(register1);
                        freeRegister(register2);
                    }
                    setRegisterToValue(register3, ptr->result);
                    freeRegister(register3);
                }
            }
        } else if (ptr->type == "div") {
            if (nowBlock->hasCertainValue(ptr->numberB)) {
                int d = nowBlock->getCertainValue(ptr->numberB);
                int numberARegister = loadValueToRegister(ptr->numberA);
                int q_register = myDiv(numberARegister, d);
                freeRegister(numberARegister);
                setRegisterToValue(q_register, ptr->result);
                freeRegister(q_register);
            } else {
                makeRegisterClean({32, 33});
                int register1 = loadValueToRegister(ptr->numberA);
                int register2 = loadValueToRegister(ptr->numberB);
                int register3 = nowBlock->getGlobalRegister(ptr->result);
                if (register3 == -1) {
                    register3 = div(register1, register2);
                } else {
                    div(register3, register1, register2);
                }
                freeRegister(register1);
                freeRegister(register2);
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
                freeRegister(32);
                freeRegister(33);
            }
        } else if (ptr->type == "mode") {
            if (this->type == "common") {
                cout << "即将做取余运算，此时寄存器2的值是: " << registerBusy[2] << endl;
            }

            if (nowBlock->hasCertainValue(ptr->numberB)) {
                int d = nowBlock->getCertainValue(ptr->numberB);
                int numberARegister_ = loadValueToRegister(ptr->numberA);
                int numberARegister = addConst(numberARegister_, 0);
                freeRegister(numberARegister_);

                int q_register = myDiv(numberARegister, d);  // 商保存在这个寄存器里。

                makeRegisterClean({32, 33});
                int ji_register = muli(q_register, d);
                freeRegister(32);
                freeRegister(33);
                freeRegister(q_register);

                int result = sub(numberARegister, ji_register);
                freeRegister(numberARegister);
                freeRegister(ji_register);
                setRegisterToValue(result, ptr->result);
                freeRegister(result);
            } else {
                makeRegisterClean({32, 33});
                int register1 = loadValueToRegister(ptr->numberA);
                int register2 = loadValueToRegister(ptr->numberB);
                int register3 = nowBlock->getGlobalRegister(ptr->result);
                if (register3 == -1) {
                    register3 = mode(register1, register2);
                } else {
                    mode(register3, register1, register2);
                }
                freeRegister(register1);
                freeRegister(register2);
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
                freeRegister(32);
                freeRegister(33);
            }

            if (this->type == "common") {
                cout << "取余运算结束，此时寄存器2的值是: " << registerBusy[2] << endl;
            }

        }
    }

    void solveLogisticOperate(shared_ptr<Stm> &ptr) {
        if (ptr->type == "not") {
            int register1 = loadValueToRegister(ptr->numberA);
            int register2 = nowBlock->getGlobalRegister(ptr->result);
            if (register2 == -1) {
                register2 = not_(register1);
            } else {
                not_(register2, register1);
            }
            freeRegister(register1);
            setRegisterToValue(register2, ptr->result);
            freeRegister(register2);
        } else if (ptr->type == "equalCheck") {
            if (ptr->result == 11) {
                cout << "即将确定 @11 = @" << ptr->numberA << " == " << ptr->numberB << endl;
            }

            int register1 = loadValueToRegister(ptr->numberA);
            int register2 = loadValueToRegister(ptr->numberB);
            int register3 = nowBlock->getGlobalRegister(ptr->result);
            if (register3 == -1) {
                register3 = equal_(register1, register2);
            } else {
                equal_(register3, register1, register2);
            }
            freeRegister(register1);
            freeRegister(register2);
            setRegisterToValue(register3, ptr->result);
            freeRegister(register3);
        } else if (ptr->type == "notEqualCheck") {
            int register1 = loadValueToRegister(ptr->numberA);
            int register2 = loadValueToRegister(ptr->numberB);
            int register3 = nowBlock->getGlobalRegister(ptr->result);
            if (register3 == -1) {
                register3 = not_equal(register1, register2);
            } else {
                not_equal(register3, register1, register2);
            }
            freeRegister(register1);
            freeRegister(register2);
            setRegisterToValue(register3, ptr->result);
            freeRegister(register3);
        } else if (ptr->type == "gt") {
            int register1 = loadValueToRegister(ptr->numberA);
            int register2 = loadValueToRegister(ptr->numberB);
            int register3 = nowBlock->getGlobalRegister(ptr->result);
            if (register3 == -1) {
                register3 = gt(register1, register2);
            } else {
                gt(register3, register1, register2);
            }
            freeRegister(register1);
            freeRegister(register2);
            setRegisterToValue(register3, ptr->result);
            freeRegister(register3);
        } else if (ptr->type == "ge") {
            int register1 = loadValueToRegister(ptr->numberA);
            int register2 = loadValueToRegister(ptr->numberB);
            int register3 = nowBlock->getGlobalRegister(ptr->result);
            if (register3 == -1) {
                register3 = ge(register1, register2);
            } else {
                ge(register3, register1, register2);
            }
            freeRegister(register1);
            freeRegister(register2);
            setRegisterToValue(register3, ptr->result);
            freeRegister(register3);
        } else if (ptr->type == "lt") {
            if (this->type == "getVarsBelongsTo") {
                int register1 = loadValueToRegister(ptr->numberA);
                int register2 = loadValueToRegister(ptr->numberB);
                int register3 = lt(register1, register2);
                freeRegister(register1);
                freeRegister(register2);
                setRegisterToValue(register3, ptr->result);
                freeRegister(register3);
            } else if (this->type == "common") {
//                cout << "进入le : " << ptr->numberA << " " << ptr->numberB << " " << ptr->result << endl;

                if (nowBlock->hasCertainValue(ptr->numberB)) {
                    int register3 = nowBlock->getGlobalRegister(ptr->result);
                    int r2 = findValueInRegister(ptr->numberB);
                    if (r2 == -1) {

//                        cout << "即将获取" << ptr->numberA << endl;
                        int register1 = loadValueToRegister(ptr->numberA);
                        nowBlock->useOneVar(ptr->numberB);
                        if (register3 == -1) {
                            register3 = ltConst(register1, nowBlock->getCertainValue(ptr->numberB));
                        } else {
                            ltConst(register3, register1, nowBlock->getCertainValue(ptr->numberB));
                        }
                        freeRegister(register1);
                    } else {
                        int register1 = loadValueToRegister(ptr->numberA);
                        nowBlock->useOneVar(ptr->numberB);
                        if (register3 == -1) {
                            register3 = lt(register1, r2);
                        } else {
                            lt(register3, register1, r2);
                        }
                        freeRegister(register1);
                        freeRegister(r2);
                    }
                    setRegisterToValue(register3, ptr->result);
                    freeRegister(register3);
                } else {
                    int register1 = loadValueToRegister(ptr->numberA);
                    int register2 = loadValueToRegister(ptr->numberB);
                    int register3 = nowBlock->getGlobalRegister(ptr->result);
                    if (register3 == -1) {
                        register3 = lt(register1, register2);
                    } else {
                        lt(register3, register1, register2);
                    }
                    freeRegister(register1);
                    freeRegister(register2);
                    setRegisterToValue(register3, ptr->result);
                    freeRegister(register3);
                }
            }
        } else if (ptr->type == "le") {
            int register1 = loadValueToRegister(ptr->numberA);
            int register2 = loadValueToRegister(ptr->numberB);
            int register3 = nowBlock->getGlobalRegister(ptr->result);
            if (register3 == -1) {
                register3 = le(register1, register2);
            } else {
                le(register3, register1, register2);
            }
            freeRegister(register1);
            freeRegister(register2);
            setRegisterToValue(register3, ptr->result);
            freeRegister(register3);
        }
    }

    void solveBranch(shared_ptr<Stm> &ptr) {
        if (ptr->type == "label") {
            setLabel(ptr->result);
        } else if (ptr->type == "jump") {


            protectRegisters("unGlobalRegister");
            jumpTo(ptr->result);
        } else if (ptr->type == "jumpIf") {
            int register1 = loadValueToRegister(ptr->numberA);
            freeRegister(register1);
            protectRegisters("unGlobalRegister");
            jumpIf(register1, ptr->result);
        } else if (ptr->type == "jumpIfNot") {
            int register1 = loadValueToRegister(ptr->numberA);
            freeRegister(register1);
            protectRegisters("unGlobalRegister");
            jumpIfNot(register1, ptr->result);
        }
    }


    void solveAboutArray(shared_ptr<Stm> &ptr) {
        if (ptr->type == "loadFrom") {
            int resultIndex = ptr->result;
            int arrayIndex = ptr->numberA;
            int offsetIndex = ptr->numberB;
            if (arrayInformations[arrayIndex] == 2) {  // 如果这个数组是参数数组
                int offsetRegister = loadValueToRegister(offsetIndex);
                int realOffsetRegister = shiftLeft(offsetRegister, 2);
                freeRegister(offsetRegister);
                int arrayRegister = loadValueToRegister(arrayIndex);
                int realAddressRegister = add(arrayRegister, realOffsetRegister);
                freeRegister(arrayRegister);
                freeRegister(realOffsetRegister);

                int valueRegister = nowBlock->getGlobalRegister(resultIndex);
                if (valueRegister == -1) {
                    valueRegister = loadFromRegisterOffset(realAddressRegister, 0);
                } else {
                    loadFromRegisterOffset(valueRegister, realAddressRegister, 0);
                }
                freeRegister(realAddressRegister);
                setRegisterToValue(valueRegister, resultIndex);
                freeRegister(valueRegister);
            } else {  // 数组非参数
                if (varsBelongsTo[arrayIndex] == -1) {  // 这是一个全局数组
                    int arrayAddress = getGlobalVarAddress(arrayIndex);  // 相对于全局基地址的偏移
                    int offsetRegister = loadValueToRegister(offsetIndex);
                    int offset_2 = shiftLeft(offsetRegister, 2);  // 左移两位后的结果
                    freeRegister(offsetRegister);
                    int realAddress2 = add(28, offset_2);
                    freeRegister(offset_2);
                    int resultRegister = nowBlock->getGlobalRegister(resultIndex);
                    if (resultRegister == -1) {
                        resultRegister = loadFromRegisterOffset(realAddress2, arrayAddress);
                    } else {
                        loadFromRegisterOffset(resultRegister, realAddress2, arrayAddress);
                    }
                    freeRegister(realAddress2);
                    setRegisterToValue(resultRegister, resultIndex);
                    freeRegister(resultRegister);
                } else {    // 这是一个当前动态区里的数组
                    int arrayAddress = getFunctionVarAddress(varsBelongsTo[arrayIndex], arrayIndex);
                    int offsetRegister = loadValueToRegister(offsetIndex);
                    int offset_2 = shiftLeft(offsetRegister, 2);  // 左移
                    freeRegister(offsetRegister);
                    int realAddress2 = add(30, offset_2);
                    freeRegister(offset_2);
                    int resultRegister = nowBlock->getGlobalRegister(resultIndex);
                    if (resultRegister == -1) {
                        resultRegister = loadFromRegisterOffset(realAddress2, arrayAddress);
                    } else {
                        loadFromRegisterOffset(resultRegister, realAddress2, arrayAddress);
                    }
                    freeRegister(realAddress2);
                    setRegisterToValue(resultRegister, resultIndex);
                    freeRegister(resultRegister);
                }
            }
        } else if (ptr->type == "calculateAddress") {
            int resultIndex = ptr->result;
            int arrayIndex = ptr->numberA;
            int offsetIndex = ptr->numberB;
            if (arrayInformations[arrayIndex] == 2) {   // 这是一个参数数组
                int register2 = loadValueToRegister(offsetIndex);
                int register3 = shiftLeft(register2, 2);
                freeRegister(register2);
                int register1 = loadValueToRegister(arrayIndex);
                int newAddressRegister = nowBlock->getGlobalRegister(resultIndex);
                if (newAddressRegister == -1) {
                    newAddressRegister = add(register1, register3);
                } else {
                    add(newAddressRegister, register1, register3);
                }
                freeRegister(register1);
                freeRegister(register3);
                setRegisterToValue(newAddressRegister, resultIndex);
                freeRegister(newAddressRegister);
            } else {
                if (varsBelongsTo[arrayIndex] == -1) {   // 这是一个全局数组
                    int arrayOffsetToGlobal = getGlobalVarAddress(arrayIndex);
                    int offsetRegister = loadValueToRegister(offsetIndex);
                    int realOffset = shiftLeft(offsetRegister, 2);
                    freeRegister(offsetRegister);
                    int add_result_register = add(realOffset, 28);
                    freeRegister(realOffset);
                    int newAddressRegister = nowBlock->getGlobalRegister(resultIndex);
                    if (newAddressRegister == -1) {
                        newAddressRegister = addConst(add_result_register, arrayOffsetToGlobal);
                    } else {
                        addConst(newAddressRegister, add_result_register, arrayOffsetToGlobal);
                    }
                    freeRegister(add_result_register);
                    setRegisterToValue(newAddressRegister, resultIndex);
                    freeRegister(newAddressRegister);
//                    if (this->type == "common") {
//                        cout << "计算" << arrayIndex << "的地址存储到" << resultIndex << "完毕" << endl;
//                        for (int i = 0; i <= 33; i++) {
//                            if (registerVar[i] == resultIndex) {
//                                cout << "寄存器" << i << "的值是" << resultIndex << endl;
//                            }
//                        }
//                    }
                } else {  // 这是某个函数里的数组
                    int arrayOffsetToFunction = getFunctionVarAddress(varsBelongsTo[arrayIndex],
                                                                      arrayIndex);  // 相对于函数运行栈基地址的偏移
                    int offsetRegister = loadValueToRegister(offsetIndex);
                    int realOffset = shiftLeft(offsetRegister, 2);
                    freeRegister(offsetRegister);
                    int add_result_register = add(realOffset, 30);
                    freeRegister(realOffset);
                    int newAddressRegister = nowBlock->getGlobalRegister(resultIndex);
                    if (newAddressRegister == -1) {
                        newAddressRegister = addConst(add_result_register, arrayOffsetToFunction);
                    } else {
                        addConst(newAddressRegister, add_result_register, arrayOffsetToFunction);
                    }
                    freeRegister(add_result_register);
                    setRegisterToValue(newAddressRegister, resultIndex);
                    freeRegister(newAddressRegister);
                }
            }
        }
    }

    void make(string type) {

        this->type = type;

        for (auto &block:Optimizer::allBlocks) {
            this->nowBlock = block;
            stringstream pool;
            pool = stringstream();
            pool << "block_" << block->blockNumber;
            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));

            pool = stringstream();
            pool << "id_" << ++logIndex << "in_global_vars";
            for (auto &p:block->liveInGlobal2) {
                pool << "_" << p;
            }
            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));


            pool = stringstream();
            pool << "id_" << ++logIndex << "next_blocks";
            for (auto &nextBlock:block->afterBlocks) {
                pool << "_block_" << nextBlock->blockNumber;
            }
            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_use_or_def";
//            for (auto &Index:block->useOrDefArray) {
//                pool << "__" << Index->type << "_" << Index->index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_global_use";
//            for (auto &Index:block->liveUseGlobal) {
//                pool << "__" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_global_def";
//            for (auto &Index:block->liveDefGlobal) {
//                pool << "__" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_local_use";
//            for (auto &Index:block->liveUseLocal) {
//                pool << "__" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_local_def";
//            for (auto &Index:block->liveDefLocal) {
//                pool << "__" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_live_global_in";
//            for (auto &Index:block->liveInGlobal) {
//                pool << "_" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_live_global_out";
//            for (auto &Index:block->liveOutGlobal) {
//                pool << "_" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_live_local_in";
//            for (auto &Index:block->liveInLocal) {
//                pool << "_" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));
//
//            pool = stringstream();
//            pool << "id_" << ++logIndex << "block_live_local_out";
//            for (auto &Index:block->liveOutLocal) {
//                pool << "_" << Index;
//            }
//            codes.emplace_back(make_shared<OneCode>("label", -1, -1, -1, -1, pool.str()));


            if (block->functionAreaPtr == nullptr) {
                this->nowFunctionIndex = -1;
            } else {
                this->nowFunctionIndex = block->functionAreaPtr->functionIndex;
            }

            bool needProtect = true;
            for (int i = 0; i < block->stm_ptrs.size(); i++) {
                shared_ptr<Stm> &ptr = block->stm_ptrs[i];
                if (ptr->type == "declaration") {
                    solveDeclaration(ptr);
                } else if (ptr->type == "assign") {
                    solveAssign(ptr);
                } else if (ptr->type == "setMain" || ptr->type == "return" || ptr->type == "setPara" ||
                           ptr->type == "callFunction" || ptr->type == "getReturnValue" || ptr->type == "function") {
                    int j = solveAboutFunction(block, i);
                    if (j != -1) {
                        i = j - 1;
                        continue;
                    }
                } else if (ptr->type == "label" || ptr->type == "jump" || ptr->type == "jumpIf" ||
                           ptr->type == "jumpIfNot") {
                    solveBranch(ptr);
                } else if (ptr->type == "print" || ptr->type == "getInt") {
                    solveIO(ptr);
                } else if (ptr->type == "mul" || ptr->type == "add" || ptr->type == "sub" || ptr->type == "div" ||
                           ptr->type == "mode") {
                    solveOperate(ptr);
                } else if (ptr->type == "not" || ptr->type == "lt" || ptr->type == "le" || ptr->type == "equalCheck" ||
                           ptr->type == "notEqualCheck" || ptr->type == "gt" || ptr->type == "ge") {
                    solveLogisticOperate(ptr);
                } else if (ptr->type == "loadFrom" || ptr->type == "calculateAddress") {
                    solveAboutArray(ptr);
                }
                if (i == block->stm_ptrs.size() - 1) {  // 遇到最后一条语句时，分析要不要保存寄存器。
                    shared_ptr<Stm> &lastPtr = block->stm_ptrs[i];
                    if (lastPtr->type == "callFunction" || lastPtr->type == "jump"
                        || lastPtr->type == "jumpIf" || lastPtr->type == "jumpIfNot") {
                        needProtect = false;
                    }
                }
            }
            if (needProtect) {
                protectRegisters("unGlobalRegister");  // 离开基本块之前，强行保护变量
            }
        }
        if (type == "getVarsBelongsTo") {
            Optimizer::varsBelongsTo = varsBelongsTo;
            Optimizer::constValues = constValues;
        }

    }

    void outPut(string filePath) {
        ofstream outFile(filePath, ios::out);
        outFile << ".data" << endl;
        for (auto par :globalDatas) {
            int index = get<0>(par);
            if (constArrays.count(index)) {  // 这是一个常量数组
                outFile << ".word";
                vector<int> &numbers = constArrays[index];
                for (auto number : numbers) {
                    outFile << " " << number;
                }
                outFile << endl;
            } else if (arrayInformations.count(index)) {  // 全局变量数组或者mian变量数组
                outFile << ".space " << get<1>(par);
                outFile << endl;
            } else if (Stm::int2str.count(index)) {  // 这是一个字符串。
                string str = Stm::int2str[index];
                outFile << ".asciiz \"" << str << "\"" << endl;
                int len = (int) str.size() + 1;
                for (char ch :str) {
                    if (ch == '\\') {  // 避免转义字符重复计算。
                        len--;
                    }
                }
                int remain = 4 - len % 4;
                if (remain == 2) {
                    outFile << ".asciiz " << "\" \"" << endl;
                } else if (remain == 3) {
                    outFile << ".asciiz " << "\"  \"" << endl;
                } else if (remain == 1) {
                    outFile << ".asciiz " << "\"    \"" << endl;
                }
            } else {  // 一定是全局变量
                outFile << ".space " << 4;
                outFile << endl;
            }
        }

        outFile << ".text" << endl;
        for (auto &code:codes) {
            outFile << code->toStr() << endl;
        }
        outFile.close();
    }

};


#endif //COMPILER_CODER_H
