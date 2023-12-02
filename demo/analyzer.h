//
// Created by Zzz0522 on 2023/12/1.
//

#ifndef DEMO_ANALYZER_H
#define DEMO_ANALYZER_H

#include <map>
#include <set>
#include <vector>
#include <string>
#include <queue>

class Analyzer {

public:
    Analyzer();

    void addGrammar(std::string &grammar);

    void clear();

    void genFirst();

    void genFollow();

    void recFollow(const std::string &str, int i, int j, bool &flag);

    void genDFA();

    void getProps(char from, std::vector<std::pair<char, std::string>> &props);

    void checkSLR1();

    void genSLR1Table();

    std::vector< std::vector< std::deque<std::string> > > analyze(std::string input);

    void debugShow();

    int getSLR1();

    int getAna();

    char getBegin();

    char getNBegin();

    int getNodes();

    int getWstate();

    std::set<char> getAlphabet();
    std::map<char, std::set<char>> getFirst();
    std::map<char, std::set<char>> getFollow();
    std::map<int, std::map<char, int>> getGraph();
    std::map<int, std::vector<std::pair<char, std::string>>> getProperty();
    std::vector<std::pair<char, std::set<char>>> getReduce();
    std::vector<std::pair<char, std::string>> getShift();
    std::map<int, std::map<char, std::pair<std::string, std::string>>> getTable();


private:

    int m_ana;
    std::set<char> m_alphabet;
    // First, Follow 部分
    std::map<char, std::set<std::string>> m_grammar;
    std::map<char, std::set<char>> m_first;
    std::map<char, std::set<char>> m_follow;
    char m_begin;
    char n_begin;
    int m_cnt;

    // DFA 部分
    // from weight to
    std::map<int, std::map<char, int>> m_Graph;
    std::map<int, std::vector<std::pair<char, std::string>>> m_property;
    int m_nodes;
    std::map<std::vector<std::pair<char, std::string>>, int> m_prop2node;

    // SLR1部分
    int m_SLR1;
    int m_wstate;
    std::vector<std::pair<char, std::set<char>>> m_reduce;
    std::vector<std::pair<char, std::string>> m_shift;
    // 点-边权-动作
    std::map<int, std::map<char, std::pair<std::string, std::string> >> m_table;

};


#endif //DEMO_ANALYZER_H
