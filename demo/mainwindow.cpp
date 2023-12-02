#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextBlock>
#include <QMessageBox>
#include <vector>
#include <queue>
#include <string>
#include <map>
#include <set>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    worker = new Analyzer();
    ui->setupUi(this);
    ui->grammarInput->setPlaceholderText("请在此输入文法");
    ui->sentenceInput->setPlaceholderText("请在此输入一行要分析的句子，输入前请确认已经进行了文法分析");
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
    if (worker->getSLR1() == 1) {
        worker->genSLR1Table();
    } else {
        // 弹窗，警告
    }
    worker->debugShow();
}


void MainWindow::on_firstBtn_clicked()
{
    ui->outputTable->clear();
    char m_begin = worker->getBegin();
    char n_begin = worker->getNBegin();
    std::map<char, std::set<char>> first = worker->getFirst();
    std::map<char, std::set<char>> follow = worker->getFollow();
    ui->outputTable->setColumnCount(2);
    ui->outputTable->setRowCount((int)first.size() + (int)follow.size() + 2);
    int row = 0;
    for (auto &[lhs, rhs]: first) {
        if (n_begin != m_begin && lhs == n_begin) {
            continue;
        }
        if (!std::isupper(lhs)) {
            continue;
        }
        std::string tlhs = std::string(1, lhs);
        QString qlhs = "First(" + QString::fromStdString(tlhs) + ")";
        ui->outputTable->setItem(row, 0, new QTableWidgetItem(qlhs));
        std::string trhs = "{";
        int cnt = 0;
        for (auto c: rhs) {
            if (++cnt > 1) {
                trhs += ",";
            }
            trhs += c;
        }
        trhs += "}";
        QString qrhs = QString::fromStdString(trhs);
        ui->outputTable->setItem(row, 1, new QTableWidgetItem(qrhs));
        ++row;
    }
    ++row;
    for (auto &[lhs, rhs]: follow) {
        if (n_begin != m_begin && lhs == n_begin) {
            continue;
        }
        if (!std::isupper(lhs)) {
            continue;
        }
        std::string tlhs = std::string(1, lhs);
        QString qlhs = "Follow(" + QString::fromStdString(tlhs) + ")";
        ui->outputTable->setItem(row, 0, new QTableWidgetItem(qlhs));
        std::string trhs = "{";
        int cnt = 0;
        for (auto c: rhs) {
            if (++cnt > 1) {
                trhs += ",";
            }
            trhs += c;
        }
        trhs += "}";
        QString qrhs = QString::fromStdString(trhs);
        ui->outputTable->setItem(row, 1, new QTableWidgetItem(qrhs));
        ++row;
    }

}


void MainWindow::on_dfaBtn_clicked()
{
    ui->outputTable->clear();
    std::set<char> alphabet = worker->getAlphabet();
    std::map<char, int> mp;
    int col = 0;
    ui->outputTable->setColumnCount(alphabet.size() + 2);
    for (auto c: alphabet) {
        mp[c] = ++col;
        std::string sc = std::string(1, c);
        QString qsc = QString::fromStdString(sc);
        ui->outputTable->setItem(0, col, new QTableWidgetItem(qsc));
    }


    std::map<int, std::map<char, int>> graph = worker->getGraph();
    int nodes = worker->getNodes();
    ui->outputTable->setRowCount(nodes + 2);

    for (int i = 0; i <= nodes; i++) {
        ui->outputTable->setItem(i + 1, 0, new QTableWidgetItem(QString::number(i)));
        for (auto &[weight, to]: graph[i]) {
            ui->outputTable->setItem(i + 1, mp[weight], new QTableWidgetItem(QString::number(to)));
        }
    }
}


void MainWindow::on_judgeBtn_clicked()
{
    int sign = worker->getSLR1();
    if (sign == 1) {

    }
}


void MainWindow::on_tableBtn_clicked()
{

}


void MainWindow::on_sentenceBtn_clicked()
{
    if (worker->getAna() == 0) {
        QMessageBox::warning(this,"错误","请先进行文法分析");
        return;
    }
    QTextDocument *document = Q_NULLPTR;
    document = ui->sentenceInput->document();
    for (QTextBlock textBlock = document->begin(); textBlock != document->end(); textBlock = textBlock.next()) {
        std::string tmp = textBlock.text().toStdString();
        if (tmp.empty()) {
            continue;
        }
        std::vector<std::vector<std::deque<std::string>>> ans = worker->analyze(tmp);
        break;
    }
}

void MainWindow::clearUp()
{

}

