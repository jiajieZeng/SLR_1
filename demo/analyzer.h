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

#include "Item.h"

class Analyzer {

public:
    Analyzer();

    void addGrammar(std::string &grammar);

    void clear();

    void genFirst();

    void genFirstRec();

    void genFollow();

    void recFollow(const std::string &str, int i, int j, bool &flag);

    void genDFA();

    void getProps(char from, std::vector<Item> &props);

    void checkSLR1();

    void genSLR1Table();

    std::vector< std::vector< std::deque<std::string> > > analyze(std::string input);

    void debugShow();

    int getSLR1();

    int getAna();

    char getBegin();

    char getNBegin();

    int getNodes();

    int getRRConflict();

    int getSRConflict();

    std::set<char> getAlphabet();

    std::map<char, std::set<char>> getFirst();

    std::map<char, std::set<char>> getFollow();

    std::map<int, std::map<char, int>> getGraph();

    std::map<int, std::vector<Item>> getProperty();

    std::map<int, std::set<std::pair<Item, Item>>> getReduce();

    std::map<int, std::set<std::pair<Item, Item>>> getShift();

    std::map<int, std::map<char, std::pair<std::string, std::string>>> getTable();

private:
    void genFirstRec(char now, std::map<char, int> &mp);

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
    std::map<int, std::vector<Item>> m_property;
    int m_nodes;
    // 反向映射
    std::map<std::vector<Item>, int> m_prop2node;
    // SLR1部分
    int m_SLR1;
    int m_RRConflict;
    int m_SRConflict;
    std::map<int, std::set<std::pair<Item, Item>>> m_reduce;

    std::map<int, std::set<std::pair<Item, Item>>> m_shift;
    // 点-边权-动作
    std::map<int, std::map<char, std::pair<std::string, std::string> >> m_table;
};


#endif //DEMO_ANALYZER_H
