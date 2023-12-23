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
        if (it != ' ' && it != '\n' && it != '\r') {
            str += it;
        }
    }
    m_alphabet.insert(str[0]);
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
            if (str[i] != '@') {
                m_alphabet.insert(str[i]);
            }
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
    n_begin = m_begin = '$';
    m_RRConflict = m_SRConflict = m_ana = m_cnt = m_nodes = 0;
    m_SLR1 = 1;
    m_first.clear();
    m_grammar.clear();
    m_alphabet.clear();
    m_table.clear();
    m_follow.clear();
    m_Graph.clear();
    m_prop2node.clear();
    m_property.clear();
    m_reduce.clear();
    m_shift.clear();
}

void Analyzer::genFirst() {
    m_ana = 1;
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
                    if (!isupper(to)) {
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

void Analyzer::genFirstRec()
{
    std::map<char, int> mp;
    genFirstRec(m_begin, mp);
}

void Analyzer::genFirstRec(char now, std::map<char, int> &mp)
{
    // 终结符或@
    if (!isupper(now)) {
        m_first[now].insert(now);
        return;
    }
    mp[now] = 1;
    for (auto &chain: m_grammar[now]) {
        int n = chain.size();
        int k = 0;
        while (k < n) {
            char to = chain[k];
            if (!mp[to]) {
                genFirstRec(to, mp);
            }
            util::combine(m_first[now], m_first[to]);
            if (!m_first[to].count('@')) {
                break;
            }
            ++k;
        }
        if (k == n) {
            m_first[now].insert('@');
        }
    }
}


void Analyzer::genFollow() {
    m_follow[m_begin].insert('$');
    bool genNew = true;
    while (genNew) {
        genNew = false;
        for (auto &[A, toset]: m_grammar) {
            if (!isupper(A)) {
                continue;
            }
            for (auto &str: toset) {
                for (int i = 0; i < str.size(); i++) {
                    char now = str[i];
                    if (!isupper(now)) {
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
    if (!isupper(str[j])) {
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
        // 拓广文法
        bool flag = 0;
        for (char i = 'A'; i <= 'Z'; i++) {
            if (!m_first.count(i)) {
                n_begin = i;
                m_grammar[n_begin].insert(std::string(1, m_begin));
                flag = 1;
                break;
            }
        }
        if (flag == 0) {
            n_begin = 'Z' + 1;
        }
        m_follow[n_begin].insert('$');  // 文法结束符号在新的begin的Follow集合里面
//        m_follow[n_begin] = m_follow[m_begin];
//        m_first[n_begin] = m_first[m_begin];
    } else {
        n_begin = m_begin;
    }
    // 先构造出第一个点
    std::vector<Item> props;
    getProps(n_begin, props);
    m_property[m_nodes] = props;
    // 反方向的映射
    m_prop2node[m_property[m_nodes]] = m_nodes;

    std::queue<int> q;
    q.push(0);
    while (!q.empty()) {
        auto now = q.front();
        q.pop();
        std::map<char, std::vector<Item>> tempNode; // 临时点
        // 拿出产生式全部往前走一步
        for (auto [produce, newProp, pos]: m_property[now]) {
            if (newProp.empty()) {
                continue;
            }
            // 已经到了最后一步 A->01234  idx=5前面已经走完
            if (pos == newProp.size()) {
                continue;
            }
            // 往前走一步
            char weight = newProp[pos];
            ++pos;
            props.clear();
            props.emplace_back(produce, newProp, pos);
            // 非终结符
            if (pos < newProp.size() && isupper(newProp[pos])) {
                // 要递归处理，因为可能最左边有很多层非终结符
                getProps(newProp[pos], props);
            }
            // 如果这个边权已经有一个点了，直接推进去
            if (tempNode.count(weight)) {
                for (auto it: props) {
                    if (std::count(tempNode[weight].begin(), tempNode[weight].end(), it)) {
                        continue;
                    }
                    tempNode[weight].emplace_back(it);
                }
                continue;
            }
            tempNode[weight] = props;
        }
        for (auto [weight, property]: tempNode) {
            // 判断是不是一个重复的点
            if (m_prop2node.count(property)) {
                // 重复了直接连边
                m_Graph[now][weight] = m_prop2node[property];
            } else {
                int to = ++m_nodes;
                m_Graph[to];    // 插入占位
                m_Graph[now][weight] = to;
                m_property[to] = property;
                m_prop2node[property] = to;
                q.push(to);
            }
        }
    }
}

void Analyzer::getProps(char from, std::vector<Item> &props) {
    std::queue<char> q;
    std::map<char, int> vis;
    q.push(from);
    vis[from] = 1;
    while (!q.empty()) {
        char t = q.front();
        q.pop();
        for (auto to: m_grammar[t]) {
            if (to.empty()) {
                continue;
            }
            std::string rhs = "";
            if (to == "@") {
                if (std::count(props.begin(), props.end(), Item(t, rhs, 0)) == 0) {
                    props.emplace_back(t, rhs, 0);
                }
                continue;
            }
            if (isupper(to[0])) {
                // 已经存在就不重复添加
                if (std::count(props.begin(), props.end(), Item(t, to, 0))) {
                    continue;
                }
                props.emplace_back(t, to, 0);
                // 不是左递归
                if (to[0] != t && !vis[to[0]]) {
                    q.push(to[0]);
                    // to[0] = 1; !!!!!!!!!!!!!!
                    vis[to[0]] = 1;
                }
            } else {
                if (std::count(props.begin(), props.end(), Item(t, to, 0)) == 0) {
                    props.emplace_back(t, to, 0);
                }
            }
        }
    }
}

void Analyzer::getProps2(char from, std::vector<Item> &props) {
    std::queue<char> q;
    std::map<char, int> vis;
    q.push(from);
    vis[from] = 1;
    while (!q.empty()) {
        char t = q.front();
        q.pop();
        for (auto to: m_grammar[t]) {
            if (to.empty()) {
                continue;
            }
            Item item = Item(t);
            if (to == "@") {
                if (std::count(props.begin(), props.end(), item) == 0) {
                    props.emplace_back(item);
                }
            } else {
                item.second = to;
                if (std::count(props.begin(), props.end(), item)) {
                    continue;
                }
                props.emplace_back(item);
                if (isupper(to[0]) && t != to[0] && !vis[to[0]]) {
                    q.push(to[0]);
                    vis[to[0]] = 1;
                }
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
        std::vector<Item> st = m_property[i];

        for (auto it = st.begin(); it != st.end(); it++) {
            char from = it->first;
            std::string to = it->second;
            int pos = it->idx;
            // 检查归约-归约冲突
            // 检查Follow集的交集
            // 从next开始，避免重复
            for (auto rit = std::next(it); rit != st.end(); rit++) {
                char rfrom = rit->first;
                std::string rto = rit->second;
                int rpos = rit->idx;
                if (pos == to.size() && rpos == rto.size()) {
                    for (auto c: m_follow[from]) {
                        if (m_follow[rfrom].count(c)) {
                            m_SLR1 = 0;
                            m_RRConflict = 1;
                            m_reduce[i].insert(std::make_pair(*it, *rit));
                        }
                    }
                }
            }

            // 检查移进-归约冲突
            // 找到一个A->a.Xp的进行匹配，B->y.的只作为被匹配项目
            for (auto &[rfrom, rto, rpos]: st) {
                if (pos == to.size() || isupper(to[pos])) {
                    continue;
                }
                if (rpos != rto.size()) {
                    continue;
                }
                // 看看X在不在Follow(B)
                if (m_follow[rfrom].count(to[pos])) {
                    m_SLR1 = -1;
                    m_SRConflict = 1;
                    m_shift[i].insert(std::make_pair(*it, Item(rfrom, rto, rpos)));
                }
            }
        }
    }
}

void Analyzer::debugShow() {
    std::cout << "First:" << std::endl;
    int cnt = 0;
    for (auto it: m_first) {
        if (!isupper(it.first)) {
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
            std::cout << prod.first << "->" << util::combineDot(prod);
        }
        std::cout << "}";
        std::cout << " -> ";
        for (auto it: m_Graph[i]) {
            std::cout << "[" << it.first << "," << it.second << "]  ";
        }
        std::cout << std::endl;
    }
//    std::cout << "--------------------------------------------------------" << std::endl;
//    if (m_SLR1 == 0) {
//        std::cout << "Reduce conflict" << std::endl;
//        for (auto it: m_reduce) {
//            std::cout << "Follow(" << it.first << ")={";
//            cnt = 0;
//            for (auto c: it.second) {
//                if (++cnt > 1) {
//                    std::cout << ",";
//                }
//                std::cout << c;
//            }
//            std::cout << "}" << std::endl;
//        }
//    } else if (m_SLR1 == -1) {
//        std::cout << "Shift reduce conflict" << std::endl;
//        for (auto prod: m_shift) {
//            std::cout << prod.first << "->" << util::combineDot(prod) << std::endl;
//        }
//    } else {
//        std::cout << "SLR(1)" << std::endl;
//    }
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

int Analyzer::getSLR1()
{
    return m_SLR1;
}

int Analyzer::getAna()
{
    return m_ana;
}

// 二维SLR1表
// 移进-规约冲突的点要删掉规约
// 在某一个点内:
//      如果有出边, 那么该边权就填 s-
//      如果有规约项，就对Follow集合的边权填 r
// 考虑先把移进填进去，再填规约，规约填的时候不能覆盖已存在的项
void Analyzer::genSLR1Table() {
    for (int i = 0; i <= m_nodes; i++) {
        // 先填入移进项目
        std::string move;
        for (auto &[weight, to]: m_Graph[i]) {
            if (isupper(weight)) {
                move = "goto";
            } else {
                move = "s";
            }
            m_table[i][weight] = std::make_pair(move, std::to_string(to));
        }

        // 检查有没有规约项目
        for (auto &[produce, prop, pos]: m_property[i]) {
            if (pos == prop.size()) {
                std::string reduce = std::string(1, produce) + "->";
                if (prop.empty()) {
                    reduce += "@";
                } else {
                    reduce += prop;
                }

                // 查看Follow集
                for (auto weight: m_follow[produce]) {
                    // 这里如果前面被填过了，那就是填了移进
                    if (m_table[i].count(weight)) {
                        continue;
                    }
                    if (produce == n_begin) {
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
            tmp.push_back(action);
            ans.emplace_back(tmp);
            break;
        }

        std::string iter = in.front();
        char nowChar = iter[0];     // 只会是单个字母的string

        if (!util::checkDigit(ana.back())) {
            action.push_back("ERROR");
            tmp.push_back(action);
            ans.emplace_back(tmp);
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
                bool flag = 1;
                // 空的话直接归约
                if (to.back() != '@') {
                    for (int i = to.size() - 1; to[i] != '>'; i--) {
                        ana.pop_back(); // 这一个肯定是状态号
                        std::string top = ana.back();
                        ana.pop_back();
                        if (top[0] != to[i]) {
                            action.push_back("ERROR");
                            tmp.push_back(action);
                            ans.emplace_back(tmp);
                            flag = 0;
                            break;
                        }
                    }
                }
                if (flag == 0) {
                    break;
                }
                nowState = atoi(ana.back().c_str()); // 切换当前状态
                char go = to[0];
                ana.push_back(std::string(1, go));    // 规约入栈
                ana.push_back(m_table[nowState][go].second); // goto
            }
        } else {
            action.push_back("ERROR");
            tmp.push_back(action);
            ans.emplace_back(tmp);
            break;
        }
        tmp.push_back(action);
        ans.emplace_back(tmp);
    }
    return ans;
}


char Analyzer::getBegin() {
    return m_begin;
}

char Analyzer::getNBegin()
{
    return n_begin;
}
int Analyzer::getNodes() {
    return m_nodes;
}

int Analyzer::getRRConflict()
{
    return m_RRConflict;
}

int Analyzer::getSRConflict()
{
    return m_SRConflict;
}

std::set<char> Analyzer::getAlphabet()
{
    return m_alphabet;
}

std::map<char, std::set<char>> Analyzer::getFirst() {
    return m_first;
}

std::map<char, std::set<char>> Analyzer::getFollow() {
    return m_follow;
}

std::map<int, std::map<char, int>> Analyzer::getGraph() {
    return m_Graph;
}

std::map<int, std::vector<Item>> Analyzer::getProperty() {
    return m_property;
}

std::map<int, std::set<std::pair<Item, Item>>> Analyzer::getReduce() {
    return m_reduce;
}

std::map<int, std::set<std::pair<Item, Item>>> Analyzer::getShift() {
    return m_shift;
}

std::map<int, std::map<char, std::pair<std::string, std::string>>> Analyzer::getTable() {
    return m_table;
}
