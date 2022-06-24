//
// Created by HP on 2021/10/22.
//

#ifndef COMPILER_ERRORMANAGER_H
#define COMPILER_ERRORMANAGER_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include "WordAnalyser.h"
#include <fstream>

using namespace std;

/**
 * 错误管理器
 * 要求：一行只能有一处错误。
 * 功能：
 *      自动去重
 *      输出前可以排序（未做）
 *
 */
class ErrorManager {
private:
    string filePath;
    bool isOutPut = true;
    map<int, string> errors = map<int, string>();  // line - errorStr

public:
    explicit ErrorManager(string filePath) {
        this->filePath = filePath;
    }

    void stopOutPut() {
        this->isOutPut = false;
    }

    void continueOutPut() {
        this->isOutPut = true;
    }

    /**
     * 要声明的那个事实是否成立。
     * 假如flag不成立，而且当前处于记录错误的状态，那么就把错误信息记录进去。str是错误标识符。information是错误详细信息。
     */
    bool assert(bool flag, int line, string str, string information = "") {
        if (isOutPut && !flag) {
            if (!information.empty()) {
                errors[line] = str + " " + information;
            } else {
                errors[line] = str;
            }

        }
        return flag;
    }

    /**
     * 输出报错信息到文件，返回是否有错误。
     * @return 是否有错误。
     */
    bool outPut() {
        ofstream outFile(filePath, ios::out);
        for (auto &lineAndError : errors) {
            outFile << lineAndError.first << " " << lineAndError.second << endl;
        }
        outFile.close();
        return !errors.empty();
    }

    bool isErrorDetected() {
        return !errors.empty();
    }

};

/**
 * 与错误处理相关的过程记录器
 */
class ProcedureManager {
    string filePath;
    bool isOutPut = true;
    vector<string> informations;
    vector<string> allInformation;

public:
    ProcedureManager(string filePath) {
        this->filePath = filePath;
    }



    void addInformation(string information) {
        if (isOutPut) {
            informations.emplace_back(information);
        }
    }

    void push(string information) {
        this->allInformation.emplace_back(information);
    }

    void pop() {
        this->allInformation.pop_back();
    }

    bool search(string target) {
        for (int index = allInformation.size() - 1; index >= 0; index--) {
            if (allInformation[index] == target) {
                return true;
            }
        }
        return false;
    }

    void stopOutPut() {
        this->isOutPut = false;
    }

    void continueOutPut() {
        this->isOutPut = true;
    }

    void outPut() {
        ofstream outFile(filePath, ios::out);
        for (auto &str:informations) {
            outFile << str << endl;
        }
        outFile.close();
    }


};

class LabelManager{
public:
    vector<tuple<int, int, int> > while_labels;
    void push_while_label(int label_1, int label_2, int label_3) {
        while_labels.emplace_back(tuple<int, int, int>(label_1, label_2, label_3));
    }

    tuple<int, int, int> pop_while_label() {
        tuple<int, int, int> tuple1 = while_labels[-1];
        while_labels.pop_back();
        return tuple1;
    }

    tuple<int, int, int> get_while_label() {
        return while_labels[while_labels.size() - 1];
    }
};

#endif //COMPILER_ERRORMANAGER_H
