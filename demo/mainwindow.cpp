#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Item.h"
#include "util.h"

#include <QTextBlock>
#include <QMessageBox>
#include <QFileDialog>
#include <ostream>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <iostream>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    worker = new Analyzer();
    ui->setupUi(this);
    ui->grammarInput->setPlaceholderText("请在此输入文法(请勿输入中文字符)");
    ui->sentenceInput->setPlaceholderText("请在此输入要分析的句子(以 $ 结尾)\n输入前请确认已经进行了文法分析(请勿输入中文字符)");
    ui->outputTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->stackTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->setWindowTitle("SLR(1)");
}

MainWindow::~MainWindow()
{
    delete ui;
    delete worker;
}



void MainWindow::on_anaBtn_clicked()
{
    worker->clear();
    QTextDocument *document = Q_NULLPTR;
    document = ui->grammarInput->document();
    for (QTextBlock textBlock = document->begin(); textBlock != document->end(); textBlock = textBlock.next()) {
        std::string tmp = textBlock.text().toStdString();
        if (tmp.empty()) {
            continue;
        }
        worker->addGrammar(tmp);
    }
    worker->genFirst();
    worker->genFollow();
    worker->genDFA();
    worker->checkSLR1();
    if (worker->getRRConflict() == 0) {
        worker->genSLR1Table();
    }
    worker->debugShow();
}


void MainWindow::on_firstBtn_clicked()
{
    if (worker->getAna() == 0) {
        QMessageBox::warning(this, "warning", "请先输入文法进行分析");
        return;
    }
    ui->outputTable->clear();
    char m_begin = worker->getBegin();
    char n_begin = worker->getNBegin();
    std::map<char, std::set<char>> first = worker->getFirst();
    std::map<char, std::set<char>> follow = worker->getFollow();
    ui->outputTable->setColumnCount(2);
    ui->outputTable->setRowCount((int)first.size() + (int)follow.size() + 5);
    int row = 0;
    for (auto &[lhs, rhs]: first) {
        if (n_begin != m_begin && lhs == n_begin) {
            continue;
        }
        if (!isupper(lhs)) {
            continue;
        }
        std::string tlhs = std::string(1, lhs);
        QString qlhs = "First(" + QString::fromStdString(tlhs) + ")";
        ui->outputTable->setItem(row, 0, new QTableWidgetItem(qlhs));
        std::string trhs = "{ ";
        int cnt = 0;
        for (auto c: rhs) {
            if (++cnt > 1) {
                trhs += ", ";
            }
            trhs += c;
        }
        trhs += " }";
        QString qrhs = QString::fromStdString(trhs);
        ui->outputTable->setItem(row, 1, new QTableWidgetItem(qrhs));
        ++row;
    }
    ++row;
    for (auto &[lhs, rhs]: follow) {
        if (n_begin != m_begin && lhs == n_begin) {
            continue;
        }
        if (!isupper(lhs)) {
            continue;
        }
        std::string tlhs = std::string(1, lhs);
        QString qlhs = "Follow(" + QString::fromStdString(tlhs) + ")";
        ui->outputTable->setItem(row, 0, new QTableWidgetItem(qlhs));
        std::string trhs = "{ ";
        int cnt = 0;
        for (auto c: rhs) {
            if (++cnt > 1) {
                trhs += ", ";
            }
            trhs += c;
        }
        trhs += " }";
        QString qrhs = QString::fromStdString(trhs);
        ui->outputTable->setItem(row, 1, new QTableWidgetItem(qrhs));
        ++row;
    }

}


void MainWindow::on_dfaBtn_clicked()
{
    if (worker->getAna() == 0) {
        QMessageBox::warning(this, "warning", "请先输入文法进行分析");
        return;
    }
    ui->outputTable->clear();
    std::set<char> alphabet = worker->getAlphabet();
    std::map<char, int> mp;
    int col = 0;
    ui->outputTable->setColumnCount(alphabet.size() + 5);
    for (auto c: alphabet) {
        mp[c] = ++col;
        std::string sc = std::string(1, c);
        QString qsc = QString::fromStdString(sc);
        ui->outputTable->setItem(0, col, new QTableWidgetItem(qsc));
    }


    std::map<int, std::map<char, int>> graph = worker->getGraph();
    std::map<int, std::vector<Item>> property = worker->getProperty();

    int nodes = worker->getNodes();
    ui->outputTable->setRowCount(nodes + 5);

    for (int i = 0; i <= nodes; i++) {
        QString qs = QString::fromStdString(std::to_string(i) + " {");
        int cnt = 0;
        for (auto &item: property[i]) {
            std::string s;
            if (++cnt > 1) {
                s += ", ";
            }
            s += std::string(1, item.first) + "->" + util::combineDot(item);
            qs += QString::fromStdString(s);
        }
        qs += "}";
        ui->outputTable->setItem(i + 1, 0, new QTableWidgetItem(qs));
        for (auto &[weight, to]: graph[i]) {
            ui->outputTable->setItem(i + 1, mp[weight], new QTableWidgetItem(QString::number(to)));
        }
    }
}


void MainWindow::on_judgeBtn_clicked()
{
    if (worker->getAna() == 0) {
        QMessageBox::warning(this, "warning", "请先输入文法进行分析");
        return;
    }
    QString ans;
    if (worker->getSLR1() == 1) {
        ans = "文法是SLR(1)文法";
    }
    if (worker->getSRConflict() == 1) {
        std::string str = "不是SLR(1)文法，存在移进-归约冲突\n";
        std::map<int, std::set<std::pair<Item, Item>>> shift = worker->getShift();
        ans = "";
        for (auto it: shift) {
            str += "在状态 (" + std::to_string(it.first) + ") 存在移进-归约冲突项\n";
            for (auto [lhs, rhs]: it.second) {
                str += std::string(1, lhs.first) + "->" + util::combineDot(lhs) + "\n";
                str += std::string(1, rhs.first) + "->" + util::combineDot(rhs) + "\n";
            }
        }
        ans = QString::fromStdString(str);
    }

    if (worker->getRRConflict() == 1) {
        std::map<char, std::set<char>> follow = worker->getFollow();
        std::string str = "不是SLR(1)文法，存在归约-归约冲突\n";
        std::map<int, std::set<std::pair<Item, Item>>> reduce = worker->getReduce();
        for (auto it: reduce) {
            str += "在状态 (" + std::to_string(it.first) + ") 存在归约-归约冲突项\n";
            for (auto [lhs, rhs]: it.second) {
                str += std::string(1, lhs.first) + "->" + util::combineDot(lhs) + " ";
                str += "Follow(" + std::string(1, lhs.first) + ")={ " + util::combineFollow(follow[lhs.first]);
                str += std::string(1, rhs.first) + "->" + util::combineDot(rhs) + " ";
                str += "Follow(" + std::string(1, rhs.first) + ")={ " + util::combineFollow(follow[rhs.first]) + "\n";
            }
        }
        ans += QString::fromStdString(str);
    }
    QMessageBox::information(this, "info", ans);
}


void MainWindow::on_tableBtn_clicked()
{
    if (worker->getAna() == 0) {
        QMessageBox::warning(this, "warning", "请先输入文法进行分析");
        return;
    }
    if (worker->getRRConflict() == 1) {
        QMessageBox::warning(this, "warning", "不是SLR(1)文法，存在归约-归约冲突，无法写出SLR(1)分析表");
        return;
    }
    ui->outputTable->clear();
    std::map<int, std::map<char, std::pair<std::string, std::string>>> table = worker->getTable();
    std::set<char> alphabet = worker->getAlphabet();
    std::set<char> upper, lower;
    for (auto c: alphabet) {
        if (isupper(c)) {
            upper.insert(c);
        } else {
            lower.insert(c);
        }
    }

    ui->outputTable->setRowCount(table.size() + 5);
    ui->outputTable->setColumnCount(alphabet.size() + 5);
    int col = 0;
    ui->outputTable->setItem(0, 0, new QTableWidgetItem("状态"));
    ui->outputTable->setItem(0, 1, new QTableWidgetItem("输入"));
    std::map<char, int> mp;
    for (auto c: lower) {
        mp[c] = ++col;
        std::string s = std::string(1, c);
        ui->outputTable->setItem(1, col, new QTableWidgetItem(QString::fromStdString(s)));
    }
    mp['$'] = ++col;
    ui->outputTable->setItem(1, col, new QTableWidgetItem("$"));

    ui->outputTable->setItem(0, col + 1, new QTableWidgetItem("Goto"));
    for (auto c: upper) {
        mp[c] = ++col;
        std::string s = std::string(1, c);
        ui->outputTable->setItem(1, col, new QTableWidgetItem(QString::fromStdString(s)));
    }
    int n = table.size();
    std::cout << "DEBUG:\n";
    std::cout << "---------------------------------------------------\n";
    for (int i = 0; i < n; i++) {
        ui->outputTable->setItem(i + 2, 0, new QTableWidgetItem(QString::number(i)));
        for (auto &[weight, move]: table[i]) {
            if (mp[weight] == 0) {
                std::cout << i << ": " << weight << "\n";
            }
            ui->outputTable->setItem(i + 2, mp[weight], new QTableWidgetItem(QString::fromStdString(move.first + " " + move.second)));
        }
    }
    std::cout << "---------------------------------------------------\n";

}


void MainWindow::on_sentenceBtn_clicked()
{
    if (worker->getAna() == 0) {
        QMessageBox::warning(this, "warning","请先进行文法分析");
        return;
    }
    if (worker->getRRConflict() == 1) {
        QMessageBox::warning(this, "warning","不是SLR(1)文法，存在归约-归约冲突，无法分析");
        return;
    }
    QTextDocument *document = Q_NULLPTR;
    document = ui->sentenceInput->document();

    std::string str;
    for (QTextBlock textBlock = document->begin(); textBlock != document->end(); textBlock = textBlock.next()) {
        std::string tmp = textBlock.text().toStdString();
        if (tmp.empty()) {
            continue;
        }
        for (auto c: tmp) {
            if (c == '\n' || c == ' ') {
                continue;
            }
            str += c;
        }
    }
    std::vector<std::vector<std::deque<std::string>>> ans = worker->analyze(str);
    ui->stackTable->clear();
    ui->stackTable->setRowCount(ans.size() + 5);
    ui->stackTable->setColumnCount(6);
    QStringList header;
    header << "步骤" << "分析栈" << "输入" << "动作";
    ui->stackTable->setHorizontalHeaderLabels(header);
    for (int i = 0; i < ans.size(); i++) {
        ui->stackTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        for (int j = 0; j < ans[i].size(); j++) {
            std::string str;
            for (auto it: ans[i][j]) {
                str += it + " ";
            }
            ui->stackTable->setItem(i, j + 1, new QTableWidgetItem(QString::fromStdString(str)));
        }
    }
}



void MainWindow::on_openGrammar_triggered()
{
    openFile(ui->grammarInput);
}

std::string MainWindow::ReadFile(std::string filename) {
    std::string buf = "";
    std::ifstream file;
    file.open(filename, std::ios::in);
    if (!file) {
        exit(1);
    }
    char ch;
    while (true) {
        ch = file.get();
        if (file.eof()) {
            break;
        }
        buf += ch;
    }
    file.close();
    return buf;
}

void MainWindow::saveFile(QPlainTextEdit *target)
{
    qDebug()<<"保存文件\n";
    QString filePath = QFileDialog::getSaveFileName(this, tr("保存当前文件"), "", tr("文件(*.txt)"));
    qDebug()<<filePath;
    std::ofstream outfile;
    std::string res = "";
    QTextDocument *document = Q_NULLPTR;
    document = target->document();
    for (QTextBlock textBlock = document->begin(); textBlock != document->end(); textBlock = textBlock.next()) {
        std::string tmp = textBlock.text().toStdString();
        if (tmp.empty()) {
            continue;
        }
        res += tmp + "\n";
    }
    outfile.open(std::string(filePath.toLocal8Bit()), std::ios::binary);
    if (res.empty()) {
        QMessageBox::warning(this,"错误","没有可存储的信息");
        return;
    }
    if (outfile << res) {
        QMessageBox::about(this,"消息","文件存储成功");
        return ;
    }
    else {
        QMessageBox::warning(this,"错误","文件存储失败");
    }
}

void MainWindow::openFile(QPlainTextEdit *target)
{
    QString filePath = QFileDialog::getOpenFileName(this, "请选择文件…","", "txt (*.txt)"); //获取文件路径
    std::string FileName =  std::string(filePath.toLocal8Bit());
    if(FileName=="") {
        return;
    }
    target->clear();
    std::string res = ReadFile(FileName);
    QString strQ = QString::fromStdString(res);
    target->appendPlainText(strQ);
    QTextCursor textCursor = target->textCursor();
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    target->setTextCursor(textCursor);
}

void MainWindow::on_saveGrammar_triggered()
{
    saveFile(ui->grammarInput);
}


void MainWindow::on_openSentence_triggered()
{
    openFile(ui->sentenceInput);
}


void MainWindow::on_saveSentence_triggered()
{
    saveFile(ui->sentenceInput);
}

