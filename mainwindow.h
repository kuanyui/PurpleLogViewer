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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void searchButtonClicked();
    void processOutputHandler();
    void processFinished();

private:
    Ui::MainWindow *ui;
    QProcess *agProcess;

    void addTreeWidgetItem(QString path);
};

#endif // MAINWINDOW_H
