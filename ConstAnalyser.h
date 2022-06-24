//
// Created by HP on 2021/12/13.
//

#ifndef COMPILER_CONSTANALYSER_H
#define COMPILER_CONSTANALYSER_H

#include "Stm.h"
#include <map>

using namespace std;

/**
 * 分析中间代码，得到哪些变量是确定值的量的信息
 *      常量有确定值
 *      常量数组确定位置有确定值
 *      由常量运算得到的temp变量有确定值。
 */

class ConstAnalyser {

private:
    set<int> tempVars;


public:

    static map<int, int> certainVarValues;

    ConstAnalyser() {

    }

    void analyseOneStm(shared_ptr<Stm> &stm_ptr) {
        string type = stm_ptr->type;

    }

    /**
     * 应当按顺序遍历中间代码，这样保证用到的量一定已经先定义了
     */
    void findConst() {
        for (auto stm_ptr:Stm::allStms) {
            analyseOneStm(stm_ptr);
        }
    }


};


#endif //COMPILER_CONSTANALYSER_H
