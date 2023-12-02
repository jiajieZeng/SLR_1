#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPlainTextEdit>
#include "analyzer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    std::string ReadFile(std::string filename);
    void saveFile(QPlainTextEdit *target);
    void openFile(QPlainTextEdit *target);

private slots:
    void on_anaBtn_clicked();

    void on_firstBtn_clicked();

    void on_dfaBtn_clicked();

    void on_judgeBtn_clicked();

    void on_tableBtn_clicked();

    void on_sentenceBtn_clicked();

    void on_openGrammar_triggered();

    void on_saveGrammar_triggered();

    void on_openSentence_triggered();

    void on_saveSentence_triggered();

private:
    Ui::MainWindow *ui;
    Analyzer *worker;
};
#endif // MAINWINDOW_H
