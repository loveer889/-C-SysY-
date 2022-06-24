#ifndef COMPILER_WORDANALYSER_H
#define COMPILER_WORDANALYSER_H

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <tuple>
#include <sstream>

using namespace std;

class WordAnalyser {
private:
    static map<string, string> name2class;
    static map<char, vector<string> > char2words;

public:
    string allStr;
    int line = 1;  // 当前要分析的字符所在行。
    int i = 0;  // 当前要分析的字符所在全局下标。
    string state = "empty";  // 初始状态为可以接受字符串。
    /*
        所有的候选状态：
            empty                   可以读入一个单词。
            comment                   需要继续读注释。
            lineComment             需要继续读行注释。
    */

    // 各个单词信息。单词原文 -  类别码 - 行号
    vector<tuple<string, string, int> > words;

    explicit WordAnalyser(const string filePath) {
        ifstream inputFile(filePath);
        stringstream allStrs;
        string temp;
        while (getline(inputFile, temp)) {
            for (auto temp_c : temp) {
                if (temp_c != '\r') {
                    allStrs << temp_c;
                }
            }
            allStrs << endl;
        }

        allStr = allStrs.str();
        inputFile.close();
    }

    vector<tuple<string, string, int> > getAnalyseResult() {
        return words;
    }

    void analyse() {
        while (!isEnd()) {
            if (state == "empty") {
                Empty();
            } else if (state == "comment") {
                Comment();
            } else if (state == "lineComment") {
                LineComment();
            }
        }
    }

    /*
    当前处于可以接受新字符的状态
    */
    void Empty() {
        char ch = getOneChar();
        if (char2words.count(ch) != 0) {
            backWard();
            string word = continueReadWords(char2words[ch]);
            if (word == "/*") {
                state = "comment";
            } else if (word == "//") {
                state = "lineComment";
            } else {
                words.emplace_back(tuple<string, string, int>(word, name2class[word], line));
            }
        } else if (ch == '"') {
            backWard();
            string formatStr = getFormatStr();
            words.emplace_back(tuple<string, string, int>(formatStr, name2class["FormatString"], line));
        } else if (is_Space(ch)) {
            backWard();
            eatSpace();
        } else if (is_digit(ch)) {
            backWard();
            string numberStr = readNumber();
            words.emplace_back(tuple<string, string, int>(numberStr, name2class["IntConst"], line));
        } else {
            backWard();
            string word = readIdentOrKeepWord();
            if (isKeepWord(word)) {
                words.emplace_back(tuple<string, string, int>(word, name2class[word], line));
            } else {
                words.emplace_back(tuple<string, string, int>(word, name2class["Ident"], line));
            }
        }
    }

    bool isKeepWord(string &word) {
        return name2class.count(word) == 1 && word != "Ident" && word != "IntConst" && word != "FormatString";
    }

    string readIdentOrKeepWord() {
        stringstream strPool;
        while (!isEnd()) {
            char ch = getOneChar();
            if (ch != '_' && !is_digit(ch) && !is_Letter(ch)) {
                backWard();
                return strPool.str();
            }
            strPool << ch;
        }
        return strPool.str();
    }

    bool is_Letter(char ch) {
        return ('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z');
    }

    string readNumber() {
        stringstream strPool;
        char ch;
        while (!isEnd()) {
            ch = getOneChar();
            if (!is_digit(ch)) {
                backWard();
                return strPool.str();
            }
            strPool << ch;
        }
        return strPool.str();
    }

    bool is_digit(char ch) {
        return '0' <= ch && ch <= '9';
    }

    bool is_Space(char ch) {
        return ch == ' ' || ch == '\t' || ch == '\n';
    }

    void eatSpace() {
        while (!isEnd()) {
            char ch = getOneChar();
            if (!is_Space(ch)) {
                backWard();
                return;

            }
        }
    }

    //  在候选单词中按照优先级匹配一个。
    string continueReadWords(vector<string> &targets) {
        for (string target:targets) {
            int j;
            for (j = 0; j < target.length(); j++) {
                if (isEnd() || getOneChar() != target[j]) {
                    break;
                }
            }
            if (j == target.length()) {
                return target;
            } else {
                if (isEnd()) {
                    for (int k = 0; k < j; k++) {
                        backWard();
                    }
                } else {
                    for (int k = 0; k <= j; k++) {
                        backWard();
                    }
                }
            }
        }
        return "WRONGXYP";  // 正常情况不会到这里，因为规定没有这种错误
    }

    // 据助教所说，FormatStr里即使出现错误字符，也不会是引号。所以可以把引号作为终止标志。
    string getFormatStr() {
        stringstream strPool;
        bool isRight = false;
        while (!isEnd()) {
            char ch = getOneChar();
            strPool << ch;
            if (ch == '"') {
                if (isRight) {
                    return strPool.str();
                } else {
                    isRight = true;
                }
            }
        }
        return "WRONGXYP_FORMAT"; // 正常情况不会到这里，因为规定没有这种错误
    }

    //  注释状态，当前已经读入了/ * ，现在需要读入*/ 才能离开。
    //  根据规则，一定能够离开注释。
    void Comment() {
        while (true) {
            if (getOneChar() == '*') {
                if (isNext('/')) {
                    getOneChar();
                    state = "empty";
                    return;
                }
            }
        }
    }

    void LineComment() {
        while (!isEnd()) {
            if (getOneChar() == '\n') {
                state = "empty";
                return;
            }
        }
    }

    char getOneChar() {
        char ch = allStr[i];
        i++;
        if (ch == '\n') {
            line++;
        }

        return ch;
    }

    bool isEnd() {
        return i == allStr.size();
    }

    // 回退
    void backWard() {
        i--;
        if (allStr[i] == '\n') {
            line--;
        }
        char ch = allStr[i];
    }

    bool isNext(char nextChar) {
        if (isEnd()) {
            return false;
        } else {
            return allStr[i] == nextChar;  // i 指向的就是下次要读入的字符。
        }
    }

    void outPut(const string &fileName) {
        ofstream outfile(fileName, ios::out);
        for (auto word : words) {
            outfile << get<1>(word) << " " << get<0>(word) << endl;
        }
        outfile.close();
    }

    static void testOne() {
        WordAnalyser analyser("testfile.txt");
        analyser.analyse();
        analyser.outPut("output.txt");
    }

    static void testAll() {
        for (int index = 1; index <= 30; index++) {
            stringstream pool;
            pool << index;

            string name = "testfile";

            string indexName;
            pool >> indexName;

            name += indexName;
            name += ".txt";

            cout << name << endl;

            WordAnalyser analyser(name);
            analyser.analyse();
            string outputName = "output";
            outputName += indexName;
            outputName += ".txt";
            analyser.outPut(outputName);
        }
    }

};

//  在FormatString 以及注释之外，一旦看到某个字符串的首字符，马上就知道候选单词的范围。
//  在前面的优先级高。
map<char, vector<string> > WordAnalyser::char2words = {
        {'!', vector<string>{"!=", "!"}},
        {'&', vector<string>{"&&"}},
        {'|', vector<string>{"||"}},
        {'+', vector<string>{"+"}},
        {'-', vector<string>{"-"}},
        {'*', vector<string>{"*"}},
        {'/', vector<string>{"/*", "//", "/"}},
        {'%', vector<string>{"%"}},
        {'<', vector<string>{"<=", "<"}},
        {'>', vector<string>{">=", ">"}},
        {'=', vector<string>{"==", "="}},
        {';', vector<string>{";"}},
        {',', vector<string>{","}},
        {'(', vector<string>{"("}},
        {')', vector<string>{")"}},
        {'[', vector<string>{"["}},
        {']', vector<string>{"]"}},
        {'{', vector<string>{"{"}},
        {'}', vector<string>{"}"}},
};

map<string, string> WordAnalyser::name2class = {
        {"Ident",           "IDENFR"},
        {"IntConst",        "INTCON"},
        {"FormatString",    "STRCON"},
        {"main",            "MAINTK"},
        {"const",           "CONSTTK"},
        {"int",             "INTTK"},
        {"break",           "BREAKTK"},
        {"continue",        "CONTINUETK"},
        {"if",              "IFTK"},
        {"else",            "ELSETK"},
        {"!",               "NOT"},
        {"&&",              "AND"},
        {"||",              "OR"},
        {"while",           "WHILETK"},
        {"getint",          "GETINTTK"},
        {"printf",          "PRINTFTK"},
        {"return",          "RETURNTK"},
        {"+",               "PLUS"},
        {"-",               "MINU"},
        {"void",            "VOIDTK"},
        {"*",               "MULT"},
        {"/",               "DIV"},
        {"%",               "MOD"},
        {"<",               "LSS"},
        {"<=",              "LEQ"},
        {">",               "GRE"},
        {">=",              "GEQ"},
        {"==",              "EQL"},
        {"!=",              "NEQ"},
        {"=",               "ASSIGN"},
        {";",               "SEMICN"},
        {",",               "COMMA"},
        {"(",               "LPARENT"},
        {")",               "RPARENT"},
        {"[",               "LBRACK"},
        {"]",               "RBRACK"},
        {"{",               "LBRACE"},
        {"}",               "RBRACE"},
};


#endif //COMPILER_WORDANALYSER_H
