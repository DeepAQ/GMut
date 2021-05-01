#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnReload_clicked();
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_comboCfg_currentIndexChanged(int);
    void on_chkAutoStart_clicked();

private:
    Ui::MainWindow *ui;
    QProcess *process;
    void loadConfig();
    void refreshAutoStart();
    void addAutoStart();
    void removeAutoStart();
    void processStateChanged(QProcess::ProcessState);
    void processReadyReadOutput();
    void processReadyReadError();
};

#endif // MAINWINDOW_H
