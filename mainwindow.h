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
    void updateAccountSelector();
    void updateFriendSelector();

private:
    Ui::MainWindow *ui;
    QProcess *agProcess;

    QString getKeyword();
    // path selector
    QDir qdir;
    QString getCurrentPath(bool fullPath = true);
    void setupPathSelector();

    // tree widget
    void addTreeWidgetItem(QString path);
};

#endif // MAINWINDOW_H
