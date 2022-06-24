#include <iostream>
#include "WordAnalyser.h"
#include "SyntaxAnalyser.h"
#include "SymbolTable.h"
#include "ErrorDetector.h"
#include "Coder.h"
#include "Stm.h"
#include <tuple>
#include "Optimizer.h"
#include "ConflictGraph.h"
#include "DefiniationAnalyse.h"

using namespace std;

map<int, string> Stm::int2str = {};
int Stm::index = 0;
int BasicBlock::blockIndex = 0;
map<int, vector<int> > Stm::arrayInitials = {};
vector<shared_ptr<Stm> > Stm::allStms = vector<shared_ptr<Stm> >();
map<int, int> Optimizer::varsBelongsTo = map<int, int>();
map<int, int> Optimizer::constValues = map<int, int>();

map<int, int> BasicBlock::certainValues = map<int, int>();
map<int, vector<int> > BasicBlock::certainArrays = map<int, vector<int> >();
map<int, int> BasicBlock::value2Register = map<int, int>();
set<int> BasicBlock::globalRegisters = {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
vector<shared_ptr<BasicBlock> > Optimizer::allBlocks = vector<shared_ptr<BasicBlock> >();


void checkGraph() {
    ConflictGraph graph;

    graph.addConflict(1, 2);
    graph.addConflict(1, 3);
    graph.addConflict(1, 4);

    graph.addConflict(2, 3);
    graph.addConflict(2, 4);
    graph.addConflict(2, 5);
    graph.addConflict(2, 6);

    graph.addConflict(3, 4);
    graph.addConflict(3, 5);

    graph.addConflict(5, 6);

    graph.addOneVar(7);

    map<int, int> result = graph.allocateRegisters({10, 20, 30, 40});
    for (auto tuple:result) {
        cout << tuple.first << " " << tuple.second << endl;
    }
}


int N = 32;

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

    long long m_low = ((long long) 1 << (N + l)) / d;
    long long m_high = (((long long) 1 << (N + l)) + ((long long) 1 << (N + l - prec))) / d;
    while (((m_low / 2) < (m_high / 2)) && sh_post > 0) {
        m_low = m_low / 2;
        m_high = m_high / 2;
        sh_post = sh_post - 1;
    }

    return {m_high, sh_post, l};
}

int SRA(int x1, int x2) {
    return x1 >> x2;
}

int SRL(int x1, int x2) {
    return (unsigned) x1 >> x2;
}

int MULSH(int x1, int x2) {
//    cout << x1 << endl;
//    int t1 = x1;
//    cout << t1 << endl;

    return ((long long) x1 * x2) >> N;
//    int t1;
//    if (x1 % 2 == 0 || x1 > 0) {
//        t1 = (int) (x1 >> (N - 1));
//    } else {
//        t1 = (int) ((x1 >> (N - 1)) + 1);
//    }
//    cout << "t1 : " << t1 << endl;
//    auto t2 = (int) (x1 - (t1 << (N - 1)));
//    cout << "t2 : " << t2 << endl;
//    int temp1 = (t1 * x2) >> 1;
//    cout << "temp1 : " << temp1 << endl;
//    int temp2 = ((long long) t2 * x2) >> N;
//    cout << "temp2 : " << temp2 << endl;
//    return temp1 + temp2;
}

int XSIGN(int x) {
    return SRA(x, N - 1);
}

/**
 * 除以一个常量。
 * n是未知的整数，存储在某个寄存器中。
 * d是已知整数。
 */
int mydivider(int d, int n) {
    cout << "here!" << endl;
    int _d_ = d >= 0 ? d : -d;
    int q;
    if (_d_ != 1) {
        tuple<long long, int, int> tuple_result = choose_multiplier(_d_, N - 1);
        cout << "end" << endl;
        long long m = get<0>(tuple_result);
        cout << "first m = " << m << endl;
        int sh_post = get<1>(tuple_result);
        cout << "first sh_post = " << sh_post << endl;
        int l = get<2>(tuple_result);
        if (_d_ == (1 << (long long) l)) {
            q = SRA(n + SRL(SRA(n, l - 1), N - l), l);
        } else if (m < ((long long) 1 << (N - 1))) {
            cout << "lesser ! " << endl;
            q = SRA(MULSH(m, n), sh_post) - XSIGN(n);
        } else {
            cout << "bigger !" << endl;
            long long compare = (long long) 1 << (N - 1);
            if ((m - (((long long) 1) << N)) >= compare) {
                long long subber = (m - (((long long) 1) << N));
                cout << m << "越界 ,它的结果是 " << subber << endl;
                cin >> m;
            }

            q = SRA(n + MULSH((int) (m - (((long long) 1) << N)), n), sh_post) - XSIGN(n);
        }
    } else {
        q = n;
    }
    if (d < 0) {
        q = -q;
    }
    return q;
}

int main() {
    ErrorDetector errorDetector("testfile.txt");
    bool isRight = errorDetector.analyse(true, true);

    if (isRight) {
        SyntaxAnalyser syntaxAnalyser("testfile.txt");
        syntaxAnalyser.analyse();
        Stm::outPut("m0.txt");

        Optimizer::devideBlocks();
        Optimizer::analyzeBlocks();
        Optimizer::connectBlocks();  // 这条调用里面会导致产生新的 stm

        Optimizer::analyzeBlocks();
        Optimizer::removeBridge2();
        Optimizer::analyzeBlocks();


        DefiniationAnalyse myDefAnalyser;
        myDefAnalyser.analyse();

        Optimizer::analyseCertainValues();

        Optimizer::outPutM("goodM.txt");
        Coder code1;
        code1.make("getVarsBelongsTo");  // 只是为了获取变量属于哪个函数的信息。
        Optimizer::deleteUselessStatement();
        Optimizer::analyzeBlocks();  // 恢复nextBlocks.因为在计算活跃变量的过程中，剪断了不同函数块之间的链接
        Optimizer::refresh();


        Coder code2;
        code2.make("getVarsBelongsTo");
        Optimizer::allocateGlobalRegister();


        Coder code;
        code.make("common");
        Optimizer::myCheck();
        code.outPut("mips.txt");

        cout << "下面打印全局寄存器分配结果" << endl;
        for (auto tuple:BasicBlock::value2Register) {
            cout << tuple.first << " : " << tuple.second << endl;
        }


    } else {
        cout << "error!";
    }

//
//    for (int i =-2147483648; i <= 3000; i++) {
//        for (int j = -3000; j <= 3000; j++) {
//            if (i == 0) {
//                continue;
//            }
//            int result1 = mydivider(i, j);
//            int realResult = j / i;
//            if (result1 != realResult) {
//                cout << "error ! " << i << " " << j << endl;
//                cin >> realResult;
//            }
//        }
//    }

//    cout << mydivider(-3000, 3000);

    return 0;
}