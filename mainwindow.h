#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QDir>
#include <QTreeWidget>
#include <QTextCursor>

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
    void stopButtonClicked();
    void processOutputHandler();
    void processFinished();
    // path selector
    void updateAccountSelector();
    void updateFriendSelector();
    // text browser
    void tryToOpenThisLogFile(QTreeWidgetItem *item, int column);
    void setupDefaultHighlightKeywordFromSearch();
    // highlight
    void highlightKeyword(QString keyword);
    void highlightKeyword();
    void nextHighlight();
    void previousHighlight();
private:
    Ui::MainWindow *ui;
    QProcess *agProcess;

    // Ui
    void changeButtonToSearch();
    void changeButtonToStop();

    // path selector
    QDir qdir;
    QString getCurrentPath(bool fullPath = true);
    void setupPathSelector();

    // tree widget
    void addTreeWidgetItem(const QString logFilePath);
    QTreeWidgetItem* itemExist(QTreeWidgetItem *item, const QString &pattern);
    QTreeWidgetItem* itemExist(QTreeWidget *tree_widget, const QString &pattern);

    // Highlight
    QList<QTextCursor> m_highlightedPositions;
    int m_highlightedIndex = 0;
    void showHighlightIndexMessage();
};

#endif // MAINWINDOW_H
