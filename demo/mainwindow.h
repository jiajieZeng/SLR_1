#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private slots:
    void on_anaBtn_clicked();

    void on_firstBtn_clicked();

    void on_dfaBtn_clicked();

    void on_judgeBtn_clicked();

    void on_tableBtn_clicked();

    void on_sentenceBtn_clicked();

private:

    void clearUp();


private:
    Ui::MainWindow *ui;
    Analyzer *worker;
};
#endif // MAINWINDOW_H
