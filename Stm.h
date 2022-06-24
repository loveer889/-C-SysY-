//
// Created by HP on 2021/11/12.
//

#ifndef COMPILER_STM_H
#define COMPILER_STM_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <fstream>
#include <memory>

using namespace std;

class Stm {
private:


public:
    static map<int, string> int2str;

    static int index;  // 变量的序号

    static map<int, vector<int> > arrayInitials;  // 数组初始化信息  TODO 当前只考虑了常量

    static int getIndex() {
        index++;
        int nextIndex = index;
        addStm(make_shared<Stm>("declaration", "temp", -1, -1, nextIndex));
        return nextIndex;
    }

    static int getUnTempIndex() {
        index++;
        return index;
    }

    static int getLabel() {
        index++;
        return index;
    }

    static vector<shared_ptr<Stm> > allStms;

    static void addStm(shared_ptr<Stm> ptr) {
        allStms.emplace_back(ptr);
    }

    static int addConst(vector<int> dims, vector<int> &numbers) {
        int index = getUnTempIndex();
        shared_ptr<Stm> ptr;
        if (dims.empty()) {
            ptr = make_shared<Stm>("declaration", "ConstUnarray", numbers[0], -1, index);
            addStm(ptr);
        } else {
            int length = dims[0];
            if (dims.size() == 2) {
                length *= dims[1];
            }
            ptr = make_shared<Stm>("declaration", "ConstArray", length, -1, index);
            addStm(ptr);
            for (int i = 0; i < numbers.size(); i++) {
                addStm(make_shared<Stm>("assign", "ConstArray", numbers[i], i, index));
            }
            arrayInitials[index] = numbers;
        }
        return index;
    }

    static int addVar(vector<int> dims) {
        int index = getUnTempIndex();
        shared_ptr<Stm> ptr;
        if (dims.empty()) {
            ptr = make_shared<Stm>("declaration", "VarUnarray", -1, -1, index);
            addStm(ptr);
        } else {
            int length = dims[0];
            if (dims.size() == 2) {
                length *= dims[1];
            }
            addStm(make_shared<Stm>("declaration", "VarArray", length, -1, index));
        }
        return index;
    }

    static void setInitialValues(int index, int dim, vector<int> indexes) {
        if (dim == 0) {
            shared_ptr<Stm> ptr = make_shared<Stm>("assign", "VarUnarrayInitial", indexes[0], -1, index);
            addStm(ptr);
        } else {
            for (int i = 0; i < indexes.size(); i++) {
                shared_ptr<Stm> ptr = make_shared<Stm>("assign", "VarArrayInitial", indexes[i], i, index);
                addStm(ptr);
            }
        }
    }

    static int addFunction(string &functionType) {
        int index = getLabel();
        int mark = 0;
        if (functionType == "int") {
            mark = 1;
        }
        shared_ptr<Stm> ptr = make_shared<Stm>("function", "start", mark, -1, index);
        addStm(ptr);
        return index;
    }

    static void setMain() {
        int index = getLabel();
        shared_ptr<Stm> ptr = make_shared<Stm>("setMain", "", -1, -1, index);
        addStm(ptr);
    }

    static void functionEnd(int functionIndex) {
        shared_ptr<Stm> ptr = make_shared<Stm>("function", "end", -1, -1, functionIndex);
        addStm(ptr);
    }

    static void functionReturn(int expression, int nowFunction) {
        // nowFunction 表示当前是哪个函数在返回。意味着可能返回到调用哪个函数的那些调用点。
        shared_ptr<Stm> ptr = make_shared<Stm>("return", "", nowFunction, -1, expression);
        addStm(ptr);
    }

    static void mainReturn() {  // 主函数返回与普通函数返回不一样。
        shared_ptr<Stm> ptr = make_shared<Stm>("return", "main", -1, -1, -1);
        addStm(ptr);
    }

    static int addPara(vector<int> &dims) {
        int index = getUnTempIndex();
        if (dims.size() == 0) {
            shared_ptr<Stm> ptr = make_shared<Stm>("declaration", "ParaUnarray", -1, -1, index);
            addStm(ptr);
        } else if (dims.size() == 1) {
            shared_ptr<Stm> ptr = make_shared<Stm>("declaration", "ParaArray", -1, -1, index);
            addStm(ptr);
        } else {
            shared_ptr<Stm> ptr = make_shared<Stm>("declaration", "ParaArray", dims[1], -1, index);
            addStm(ptr);
        }
        return index;
    }

    static void setLabel(int label) {
        shared_ptr<Stm> ptr = make_shared<Stm>("label", "", -1, -1, label);
        addStm(ptr);
    }

    static void jump(int label) {
        shared_ptr<Stm> ptr = make_shared<Stm>("jump", "", -1, -1, label);
        addStm(ptr);
    }


    static void printInt(int index) {
        shared_ptr<Stm> ptr = make_shared<Stm>("print", "value", -1, -1, index);
        addStm(ptr);
    }

    static void printStr(string &str) {
        int index = getUnTempIndex();
        int2str[index] = str;
        shared_ptr<Stm> ptr = make_shared<Stm>("print", "str", -1, -1, index);
        addStm(ptr);
    }

    static int getInt() {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("getInt", "", -1, -1, index);
        addStm(ptr);
        return index;
    }

    static void assignVarValue(int index, int valueIndex) {
        shared_ptr<Stm> ptr = make_shared<Stm>("assign", "Unarray", valueIndex, -1, index);
        addStm(ptr);
    }

    /**
      * 给数组某个位置赋值
     */
    static void assignArrayValue(int arrayIndex, int offsetIndex, int valueIndex) {
        shared_ptr<Stm> ptr = make_shared<Stm>("assign", "Array", offsetIndex, valueIndex, arrayIndex);
        addStm(ptr);
    }

    static int multiConst(int numberAIndex, int factor) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("mul", "Const", numberAIndex, factor, index);
        addStm(ptr);
        return index;
    }

    static int multi(int numberAIndex, int numberBIndex) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("mul", "", numberAIndex, numberBIndex, index);
        addStm(ptr);
        return index;
    }

    static int adder(int numberAIndex, int numberBIndex) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("add", "", numberAIndex, numberBIndex, index);
        addStm(ptr);
        return index;
    }

    static int subber(int numberAIndex, int numberBIndex) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("sub", "", numberAIndex, numberBIndex, index);
        addStm(ptr);
        return index;
    }

    static int constSub(int numberConst, int numberAIndex) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("sub", "ConstSub", numberConst, numberAIndex, index);
        addStm(ptr);
        return index;
    }

    static int div(int numberAIndex, int numberBIndex) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("div", "", numberAIndex, numberBIndex, index);
        addStm(ptr);
        return index;
    }

    static int mode(int numberAIndex, int numberBIndex) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("mode", "", numberAIndex, numberBIndex, index);
        addStm(ptr);
        return index;
    }


    static int minusSelf(int numberAIndex) {
        return constSub(0, numberAIndex);
    }

    static int notSelf(int numberAIndex) {
        int index = getIndex();
        //  非0 -> 0  ; 0 -> 1
        shared_ptr<Stm> ptr = make_shared<Stm>("not", "", numberAIndex, -1, index);
        addStm(ptr);
        return index;
    }

    static int equal(int result1Index, int result2Index) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("equalCheck", "", result1Index, result2Index, index);
        addStm(ptr);
        return index;
    }

    static int notEqual(int result1Index, int result2Index) {
        int index = getIndex();
        shared_ptr<Stm> ptr = make_shared<Stm>("notEqualCheck", "", result1Index, result2Index, index);
        addStm(ptr);
        return index;
    }

    static int gt(int result1Index, int result2Index) {
        int index = getIndex();
        addStm(make_shared<Stm>("gt", "", result1Index, result2Index, index));
        return index;
    }

    static int ge(int result1Index, int result2Index) {
        int index = getIndex();
        addStm(make_shared<Stm>("ge", "", result1Index, result2Index, index));
        return index;
    }

    static int lt(int result1Index, int result2Index) {
        int index = getIndex();
        addStm(make_shared<Stm>("lt", "", result1Index, result2Index, index));
        return index;
    }

    static int le(int result1Index, int result2Index) {
        int index = getIndex();
        addStm(make_shared<Stm>("le", "", result1Index, result2Index, index));
        return index;
    }

    static void jumpIf(int result1Index, int label) {
        addStm(make_shared<Stm>("jumpIf", "", result1Index, -1, label));
    }

    static void jumpIfNot(int result1Index, int label) {
        addStm(make_shared<Stm>("jumpIfNot", "", result1Index, -1, label));
    }

    static int assignConst(int constNumber) {
        vector<int> temp = {constNumber};
        return addConst({}, temp);
    }

    static void pushUnArrayPara(int index) {
        shared_ptr<Stm> ptr = make_shared<Stm>("setPara", "UnArray", -1, -1, index);
        addStm(ptr);
    }

    static int loadArrayValue(int arrayIndex, int offset) {
        // offset 那个变量存储的是“第几个”
        int index2 = getIndex();
        addStm(make_shared<Stm>("loadFrom", "", arrayIndex, offset, index2));
        return index2;
    }

    static int calculateOffset(vector<int> oriDims, vector<int> insightIndexs) {
        if (oriDims.size() == 0) {
            return assignConst(0);
        } else if (oriDims.size() == 1) {
            if (insightIndexs.size() == 0) {
                return assignConst(0);
            } else if (insightIndexs.size() == 1) {
                return insightIndexs[0];
            } else {
                return -1;
            }
        } else {
            if (insightIndexs.size() == 0) {
                return assignConst(0);
            } else if (insightIndexs.size() == 1) {
                return multiConst(insightIndexs[0], oriDims[1]);
            } else {
                int index1 = multiConst(insightIndexs[0], oriDims[1]);
                return adder(index1, insightIndexs[1]);
            }
        }
    }

    static int calcluateAddress(int arrayIndex, int offsetIndex) {
        int index = getIndex();
        addStm(make_shared<Stm>("calculateAddress", "", arrayIndex, offsetIndex, index));
        return index;
    }

    static void callFunction(int functionIndex, int returnIndex) {
        // 调用函数并把结果赋给某个变量 。 -1表示不要结果。
        addStm(make_shared<Stm>("callFunction", "", functionIndex, -1, returnIndex));

        addStm(make_shared<Stm>("getReturnValue", "", functionIndex, -1, returnIndex));

    }

    static int outPut(string filePath) {
        ofstream outFile(filePath, ios::out);
        for (auto stm_ptr:allStms) {
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

                    outFile << "print" << " " << int2str[stm_ptr->result] << endl;
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
                outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << ">" << "@" << stm_ptr->numberB
                        << endl;
            } else if (stm_ptr->type == "lt") {
                outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << "<" << "@" << stm_ptr->numberB
                        << endl;
            } else if (stm_ptr->type == "le") {
                outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << "<=" << "@" << stm_ptr->numberB
                        << endl;
            } else if (stm_ptr->type == "setPara") {
                if (stm_ptr->operation == "Array") {
                    outFile << "push array " << "@" << stm_ptr->result << "[" << "@" << stm_ptr->numberA << "]" << endl;
                } else {
                    outFile << "push " << "@" << stm_ptr->result << endl;
                }
            } else if (stm_ptr->type == "load") {
                outFile << "@" << stm_ptr->result << " = " << "@" << stm_ptr->numberA << "[@" << stm_ptr->numberB << "]"
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
        outFile.close();
        return 0;
    }

public:
    string type = "";
    string operation = "";
    int numberA = -1;
    int numberB = -1;
    int result = -1;


    Stm(string type, string operation, int numberA, int numberB, int result) {
        this->type = type;
        this->operation = operation;
        this->numberA = numberA;
        this->numberB = numberB;
        this->result = result;
    }

    void setNumberA(int newNumberAIndex) {
        this->numberA = newNumberAIndex;
    }

    void setNumberB(int newNumberBIndex) {
        this->numberB = newNumberBIndex;
    }

    void setResult(int newResultIndex) {
        this->result = newResultIndex;
    }

    /**
     * 制作一个添加常量的语句并返回，常量序号存到constIndex里。
     */
    static shared_ptr<Stm> makeConst(int constValue, int &constIndex) {
        constIndex = getUnTempIndex();


        return make_shared<Stm>("declaration", "ConstUnarray", constValue, -1, constIndex);
    }
};


#endif //COMPILER_STM_H
