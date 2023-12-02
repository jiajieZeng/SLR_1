//
// Created by Zzz0522 on 2023/12/1.
//

#include "analyzer.h"
#include "util.h"

#include <iostream>
#include <queue>
#include <algorithm>

Analyzer::Analyzer() {
    clear();
}

void Analyzer::addGrammar(std::string &grammar) {
    std::string str = "";
    for (auto it: grammar) {
        if (it != ' ' && it != '\n') {
            str += it;
        }
    }
    int i = 3;
    std::string to = "";
    char from = str[0];
    if (m_cnt == 0) {
        m_begin = from;
    }
    ++m_cnt;
    for (; i < str.size(); i++) {
        if (str[i] != '|') {
            to += str[i];
        } else {
            m_grammar[from].insert(to);
            to = "";
        }
    }
    if (!to.empty()) {
        m_grammar[from].insert(to);
    }
}

void Analyzer::clear() {
    m_first.clear();
    m_grammar.clear();
    m_begin = '$';
    m_cnt = m_nodes = 0;
    m_SLR1 = 1;
}

void Analyzer::genFirst() {
    bool genNew = true;
    while (genNew) {
        genNew = false;
        for (auto &[A, toset]: m_grammar) {
            int presize = m_first[A].size();
            for (auto &chain: toset) {
                int n = chain.size();
                int k = 0;
                while (k < n) {
                    char to = chain[k];
                    if (to == '@' || std::islower(to)) {
                        if (m_first[to].empty()){
                            genNew = true;
                        }
                        m_first[to].insert(to);
                    }
                    util::combine(m_first[A], m_first[to]);
                    if (!m_first[to].count('@')) {
                        break;
                    }
                    ++k;
                }
                if (k == n) {
                    m_first[A].insert('@');
                }
            }
            if (presize < m_first[A].size()) {
                genNew = true;
            }
        }
    }
}

void Analyzer::genFollow() {
    m_follow[m_begin].insert('$');
    bool genNew = true;
    while (genNew) {
        genNew = false;
        for (auto &[A, toset]: m_grammar) {
            if (!std::isupper(A)) {
                continue;
            }
            for (auto &str: toset) {
                for (int i = 0; i < str.size(); i++) {
                    char now = str[i];
                    if (A == 'A' && now == 'D') {
                        int x = 1;
                    }
                    if (!std::isupper(now)) {
                        continue;
                    }
                    if (i + 1 == str.size()) {
                        int presize = m_follow[now].size();
                        util::combine(m_follow[now], m_follow[A]);
                        if (presize < m_follow[now].size()) {
                            genNew = true;
                        }
                    } else {
                        int presize = m_follow[now].size();
                        bool flag = false;
                        recFollow(str, i, i + 1, flag);
                        // 推出了epsilon
                        if (flag) {
                            util::combine(m_follow[now], m_follow[A]);
                        }
                        if (presize < m_follow[now].size()) {
                            genNew = 1;
                        }
                    }
                }
            }
        }
    }
}

void Analyzer::recFollow(const std::string &str, int i, int j, bool &flag) {
    if (!std::isupper(str[j])) {
        m_follow[str[i]].insert(str[j]);
        return;
    }
    util::combine(m_follow[str[i]], m_first[str[j]]);
    if (m_first[str[j]].count('@')) {
        if (j + 1 < str.size()) {
            recFollow(str, i, j + 1, flag);
        } else {
            flag = true;
            return;
        }
    }
}

void Analyzer::genDFA() {
    if (m_grammar[m_begin].size() > 1) {
        // 不能用尽26个大写字母
        for (char i = 'A'; i <= 'Z'; i++) {
            if (!m_first.count(i)) {
                char t_begin = i;
                m_grammar[t_begin].insert(std::string(1, m_begin));
                m_begin = t_begin;
                break;
            }
        }
        m_follow[m_begin].insert('$');  // 文法结束符号在新的begin的Follow集合里面
    }
    // 先构造出第一个点
    std::vector<std::pair<char, std::string>> props;
    getProps(m_begin, props);
    m_property[m_nodes] = props;
    // 反方向的映射
    m_prop2node[m_property[m_nodes]] = m_nodes;

    std::queue<int> q;
    q.push(0);
    while (!q.empty()) {
        auto now = q.front();
        q.pop();
        // 拿出产生式全部往前走一步
        for (auto it: m_property[now]) {
            std::string newProp = it.second;
            if (newProp.empty()) {
                continue;
            }
            int pos = newProp.find('.');
            // 已经到了最后一步
            if (pos == newProp.size() - 1) {
                continue;
            }
            // 往前走一步
            newProp[pos] = newProp[pos + 1];
            newProp[++pos] = '.';
            char weight = newProp[pos - 1];
            props.clear();
            props.emplace_back(it.first, newProp);
            // 非终结符
            if (pos + 1 < newProp.size() && std::isupper(newProp[pos + 1])) {
                // 要递归处理，因为可能最左边有很多层非终结符
                for (auto &to: m_grammar[newProp[pos + 1]]) {
                    getProps(newProp[pos + 1], props);
                }
            }
            // 加点
            // 如果这个边权已经有一个点了，那就推进去就好了
            if (m_Graph[now][weight] != 0) {
                int toPoint = m_Graph[now][weight];
                // 插入
                for (auto it: props) {
                    if (std::count(m_property[toPoint].begin(), m_property[toPoint].end(), it)) {
                        continue;
                    }
                    m_property[toPoint].emplace_back(it);
                }
                continue;
            }
            // 如果没有，那么首先看看是不是去一个已经出现过的点
            if (m_prop2node.count(props)) {
                m_Graph[now][weight] = m_prop2node[props];
                continue;
            }
            int to = ++m_nodes;
            m_Graph[now][weight] = to;
            m_property[to] = props;
            m_prop2node[props] = to;
            q.push(to);
        }
    }
}

void Analyzer::getProps(char from, std::vector<std::pair<char, std::string>> &props) {
    for (auto to: m_grammar[from]) {
        if (to.empty()) {
            continue;
        }
        std::string rhs = ".";
        if (to == "@") {
            if (std::count(props.begin(), props.end(), std::make_pair(from, rhs)) == 0) {
                props.emplace_back(from, rhs);
            }
            continue;
        }
        if (std::isupper(to[0])) {
            // 已经存在就不重复添加
            int flag = 1;
            for (auto it: props) {
                if (it.second.empty()) {
                    continue;
                }
                if (it.first == from && it.second == (rhs + to)) {
                    flag = 0;
                    break;
                }
            }
            if (flag == 0) {
                continue;
            }
            props.emplace_back(from, rhs + to);
            // 不是左递归
            if (to[0] != from) {
                getProps(to[0], props);
            }
        } else {
            if (std::count(props.begin(), props.end(), std::make_pair(from,  rhs + to)) == 0) {
                props.emplace_back(from, rhs + to);
            }

        }
    }
}

/*
 * SLR(1)解决移进-规约冲突
 * 在同一个点内，如果有A->aE.cD, D->E. 此时{c}交Follow(D)应该为空
 * 找到一个原因马上停止
 */
void Analyzer::checkSLR1() {

    // 检查每一个点
    for (int i = 0; i <= m_nodes; i++) {
        std::vector<std::pair<char, std::string>> st = m_property[i];

        for (auto &[from, to]: st) {
            // 检查Follow集的交集
            for (auto &[rfrom, rto]: st) {
                int pos = to.find('.');
                int rpos = rto.find('.');
                if (from != rfrom && pos == to.size() - 1 && rpos == rto.size() - 1) {
                    for (auto c: m_follow[from]) {
                        if (m_follow[rfrom].count(c)) {
                            m_SLR1 = 0;
                            m_reduce.emplace_back(from, m_follow[from]);
                            m_reduce.emplace_back(rfrom, m_follow[rfrom]);
                            return;
                        }
                    }
                }
                // 检查移进-规约冲突
                // 找到一个A->a.Xp的进行匹配，B->y.的只作为被匹配项目
                if (pos == to.size() - 1 || std::isupper(to[pos + 1])) {
                    continue;
                }
                if (rpos != rto.size() - 1) {
                    continue;
                }
                // 看看X在不在Follow(B)
                if (m_follow[rfrom].count(to[pos + 1])) {
                    m_SLR1 = -1;
                    m_shift.emplace_back(from, to);
                    m_shift.emplace_back(rfrom, rto);
                    return;
                }
            }
        }
    }
}

void Analyzer::debugShow() {
    std::cout << "First:" << std::endl;
    int cnt = 0;
    for (auto it: m_first) {
        if (!std::isupper(it.first)) {
            continue;
        }
        std::cout << it.first << ":  {";
        cnt = 0;
        for (auto v: it.second) {
            if (cnt++ >= 1) {
                std::cout << ",";
            }
            std::cout << v;
        }
        std::cout << "}" << std::endl;
    }
    std::cout << "Follow:" << std::endl;
    for (auto &[A, st]: m_follow) {
        std::cout << A << ":  {";
        cnt = 0;
        for (auto it: st) {
            if (cnt++ >= 1) {
                std::cout << ",";
            }
            std::cout << it;
        }
        std::cout << "}" << std::endl;
    }
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "DFA:" << std::endl;
    for (int i = 0; i <= m_nodes; i++) {
        std::cout << i << ": {";
        cnt = 0;
        for (auto prod: m_property[i]) {
            if (cnt++ >= 1) {
                std::cout << " | ";
            }
            std::cout << prod.first << "->" << prod.second;
        }
        std::cout << "}";
        std::cout << " -> ";
        for (auto it: m_Graph[i]) {
            std::cout << "[" << it.first << "," << it.second << "]  ";
        }
        std::cout << std::endl;
    }
    std::cout << "--------------------------------------------------------" << std::endl;
    if (m_SLR1 == 0) {
        std::cout << "Reduce conflict" << std::endl;
        for (auto it: m_reduce) {
            std::cout << "Follow(" << it.first << ")={";
            cnt = 0;
            for (auto c: it.second) {
                if (++cnt > 1) {
                    std::cout << ",";
                }
                std::cout << c;
            }
            std::cout << "}" << std::endl;
        }
    } else if (m_SLR1 == -1) {
        std::cout << "Shift reduce conflict" << std::endl;
        for (auto it: m_shift) {
            std::cout << it.first << "->" << it.second << std::endl;
        }
    } else {
        std::cout << "SLR(1)" << std::endl;
    }
    std::cout << "--------------------------------------------------------" << std::endl;
    std::cout << "SLR(1) table" << std::endl;
    for (int i = 0; i <= m_nodes; i++) {
        std::cout << i << ": ";
        for (auto &[weight, move]: m_table[i]) {
            std::cout << "{" << weight << " , " << move.first << " " << move.second << "}  ";
        }
        std::cout << std::endl;
    }
}

// 二维SLR1表
// 移进-规约冲突的点要删掉规约
// 在某一个点内:
//      如果有出边, 那么该边权就填 s-
//      如果有规约项，就对Follow集合的边权填 r
// 考虑先把移进填进去，再填规约，规约填的时候不能覆盖已存在的项
void Analyzer::genSLR1Table() {
    std::cout << "DEBUG:" << std::endl;
    for (int i = 0; i <= m_nodes; i++) {
        // 先填入移进项目
        std::string move;
        for (auto &[weight, to]: m_Graph[i]) {
            if (std::isupper(weight)) {
                move = "goto";
            } else {
                move = "s";
            }
            m_table[i][weight] = std::make_pair(move, std::to_string(to));
        }

        // 检查有没有规约项目
        for (auto &[produce, prop]: m_property[i]) {
            if (prop.find(".") == prop.size() - 1) {
                std::string reduce = std::string(1, produce) + "->";
                if (prop.size() == 1) {
                    reduce += "@";
                } else {
                    reduce += prop.substr(0, prop.size() - 1);
                }

                // 查看Follow集
                for (auto weight: m_follow[produce]) {
                    // 这里如果前面被填过了，那就是填了移进
                    if (m_table[i].count(weight)) {
                        continue;
                    }
                    if (produce == m_begin) {
                        m_table[i][weight] = std::make_pair("acc", "");
                    } else {
                        m_table[i][weight] = std::make_pair("r", reduce);
                    }
                }
            }
        }
    }
}

/*
 * 使用栈分析，由于需要展示，那么可以使用 string/vector 代替
 * 考虑左右都统一，使用string
 * 考虑当前字符：
 *  1.当前所在点存在这条边，且是移进，那么移进
 *  2.当前所在点存在这条边，且是GOTO，那么GOTO
 *  3.如果是规约，那么进行规约
 */
std::vector< std::vector< std::deque<std::string> > > Analyzer::analyze(std::string input) {
    std::deque<std::string> in;
    std::deque<std::string> ana;
    ana.push_back("$");
    ana.push_back("0");

    for (auto c: input) {
        in.push_back(std::string(1, c));
    }
    int done = 0;
    std::vector< std::vector< std::deque<std::string> > > ans;
    while (done == 0){
        std::vector< std::deque<std::string> > tmp = {ana, in};
        std::deque<std::string> action;

        if (in.empty()) {
            action.push_back("ERROR");
            break;
        }

        std::string iter = in.front();
        char nowChar = iter[0];     // 只会是单个字母的string

        if (!util::checkDigit(ana.back())) {
            action.push_back("ERROR");
            break;
        }
        // 看当前栈顶状态
        int nowState = atoi(ana.back().c_str());
        if (m_table[nowState].count(nowChar)) {
            std::string move = m_table[nowState][nowChar].first;
            std::string to = m_table[nowState][nowChar].second;
            if (move == "acc") {
                action.push_back("accept");
                done = 1;
            } else if (move == "s") {
                int toState = atoi(to.c_str());
                action.push_back("shift  " + to);
                ana.push_back(iter); // 推入分析栈
                ana.push_back(to);
                in.pop_front();
            } else {    // 规约
                action.push_back("reduce  " + to);
                if (to.back() != '@') {
                    for (int i = to.size() - 1; to[i] != '>'; i--) {
                        ana.pop_back();
                        ana.pop_back();
                    }
                }
                nowState = atoi(ana.back().c_str()); // 切换当前状态
                char go = to[0];
                ana.push_back(std::string(1, go));    // 规约入栈
                ana.push_back(m_table[nowState][go].second); // goto
                //                ana.push_back(std::to_string(nowState)); // goto状态入栈
            }
        } else {
            action.push_back("ERROR");
            break;
        }
        tmp.push_back(action);
        ans.emplace_back(tmp);
    }
    return ans;
}
