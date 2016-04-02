#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QDir>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void searchButtonClicked();
    void processOutputHandler();
    void processFinished();
    // path selector
    void updateAccountsSelector();
    void updateChatroomsSelector();

private:
    Ui::MainWindow *ui;
    QProcess *agProcess;
    // path selector
    QDir protocol_dir;
    QDir account_dir;
    QDir friend_dir;
    void setupPathSelector();
    // tree widget
    void addTreeWidgetItem(QString path);
};

#endif // MAINWINDOW_H
