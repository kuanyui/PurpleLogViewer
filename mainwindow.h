#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QDir>
#include <QTreeWidget>

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
    void addTreeWidgetItem(QString fullFilePathPath);
    bool itemExist(QTreeWidgetItem &item, QString &pattern);
    bool itemExist(QTreeWidget &tree_widget, QString &pattern);
};

#endif // MAINWINDOW_H
