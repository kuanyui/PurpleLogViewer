#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QTreeWidget>
#include <QDirIterator>

#define CBOX_EMPTY_STR "--All--"
#define SEP QDir::separator()
const QString LOG_ROOT = QDir::homePath() + QString("/.purple/logs/");

// http://log.noiretaya.com/200

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    agProcess = new QProcess(this);
    setupPathSelector();
    // Setup process
    connect(agProcess, SIGNAL(finished(int)), this, SLOT(processFinished()));
    connect(agProcess, SIGNAL(readyRead()),   this, SLOT(processOutputHandler()));
    // click button
    changeButtonToSearch();
}

MainWindow::~MainWindow()
{
    if (agProcess->state() == QProcess::Running)
    {
        agProcess->close();
        agProcess->waitForFinished();
    }
    delete agProcess;
    delete ui;
}


QString MainWindow::getKeyword(){
    return ui->keyword->text();
}

// ==========================================================
// QProcess
// ==========================================================

void MainWindow::changeButtonToSearch(){
    // replace slot (stop -> search)
    ui->search_button->setText("Search");
    ui->search_button->disconnect();
    connect(ui->search_button, SIGNAL(clicked()), this, SLOT(searchButtonClicked()));
}

void MainWindow::changeButtonToStop(){
    // replace slot (search -> stop)
    ui->search_button->setText("Stop");
    ui->search_button->disconnect();
    connect(ui->search_button, SIGNAL(clicked()), this, SLOT(stopButtonClicked()));
}

void MainWindow::searchButtonClicked()
{
    ui->tree_widget->clear();
    QString keyword = getKeyword();
    if (keyword.isEmpty()){
        QStringList patterns = QStringList();
        patterns << "*.html";
        QDirIterator it(getCurrentPath(), patterns, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            addTreeWidgetItem(it.next());
            ui->statusBar->showMessage(it.next());
        }
    } else {
        QStringList args;
        args << "--files-with-matches" << "--ackmate" << keyword << getCurrentPath();
        agProcess->start("ag", args);
        changeButtonToStop();
    }
}

void MainWindow::stopButtonClicked()
{
    agProcess->terminate();

}

void MainWindow::processOutputHandler()
{
    while ( agProcess->canReadLine() ) {
        QString raw = QString(agProcess->readLine());
        // mid(1) due to "--ackmate" argument
        QString path = raw.mid(1, raw.size()-2);
        addTreeWidgetItem(path);
        ui->statusBar->showMessage(QString("%1 found").arg(path));
    }
}


void MainWindow::processFinished()
{
    ui->statusBar->showMessage("Done!");
    changeButtonToSearch();
}

// ==========================================================
// Path Selector: Protocal / Account / Chatroom
// ==========================================================

void MainWindow::setupPathSelector()
{
    qdir = QDir(LOG_ROOT);
    qdir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

    connect(ui->protocol_box, SIGNAL( currentTextChanged(QString) ),
            ui->account_box, SLOT( clear() ));
    connect(ui->protocol_box, SIGNAL( currentTextChanged(QString) ),
            ui->friend_box, SLOT( clear() ));
    connect(ui->account_box, SIGNAL( currentTextChanged(QString) ),
            ui->friend_box, SLOT( clear() ));

    connect(ui->protocol_box, SIGNAL( currentTextChanged(QString) ),
            this, SLOT(updateAccountSelector()));
    connect(ui->account_box, SIGNAL( currentTextChanged(QString) ),
            this, SLOT(updateFriendSelector()));

    ui->protocol_box->addItem(CBOX_EMPTY_STR);
    ui->protocol_box->addItems(qdir.entryList());

}

void MainWindow::updateAccountSelector()
{
    ui->account_box->addItem(CBOX_EMPTY_STR);
    QString protocol = ui->protocol_box->currentText();
    if (protocol != CBOX_EMPTY_STR)
    {
        qdir.cd(LOG_ROOT + protocol);
        qDebug() << qdir.currentPath();
        qDebug() << qdir.entryList();
        ui->account_box->addItems(qdir.entryList(QDir::NoFilter, QDir::Size)); // Sort by file/dir amount
    }
}


void MainWindow::updateFriendSelector()
{
    ui->friend_box->addItem(CBOX_EMPTY_STR);
    QString protocol = ui->protocol_box->currentText();
    QString account = ui->account_box->currentText();
    if (account != CBOX_EMPTY_STR)
    {
        qdir.cd(LOG_ROOT + protocol + "/" + account);
        ui->friend_box->addItems(qdir.entryList(QDir::NoFilter, QDir::Size)); // Sort
    }
}

QString MainWindow::getCurrentPath(bool fullPath)
{
    QString protocol = ui->protocol_box->currentText();
    QString account = ui->account_box->currentText();
    QString friend_chat = ui->friend_box->currentText();
    QString root ("");
    if (fullPath) { root = LOG_ROOT; };
    if (protocol == CBOX_EMPTY_STR) { return root ;};
    if (account == CBOX_EMPTY_STR) { return root + protocol ;};
    if (friend_chat == CBOX_EMPTY_STR) { return root + protocol + "/" + account; };
    return root + protocol + "/" + account + "/"  + friend_chat ;
}

// ==========================================================
// QTreeWidget
// ==========================================================


void MainWindow::addTreeWidgetItem(const QString logFilePath)  // log file path
{
    QStringList pro_acc_fri_file = logFilePath.split(SEP);
    QString protocol_str = pro_acc_fri_file[pro_acc_fri_file.size() -4];
    QString account_str = pro_acc_fri_file[pro_acc_fri_file.size() -3];
    QString friend_str = pro_acc_fri_file[pro_acc_fri_file.size() -2];
    QString filename_str = pro_acc_fri_file[pro_acc_fri_file.size() - 1];
    // Protocol
    QTreeWidgetItem *protocol_item = itemExist(ui->tree_widget, protocol_str);
    if (protocol_item == nullptr){
        protocol_item = new QTreeWidgetItem(ui->tree_widget);
        protocol_item->setText(0, protocol_str);
        protocol_item->setFont(0, QFont("Helvetica", 12, QFont::Bold));
    }
    // Account
    QTreeWidgetItem *account_item = itemExist(protocol_item, account_str);
    if (account_item == nullptr){
        account_item = new QTreeWidgetItem(protocol_item);
        account_item->setText(0, account_str);
    }
    // Friend
    QTreeWidgetItem *friend_item = itemExist(account_item, friend_str);
    if (friend_item == nullptr){
        friend_item = new QTreeWidgetItem(account_item);
        friend_item->setText(0, friend_str);
    }
    // Log file itself
    QTreeWidgetItem *log_file_item = new QTreeWidgetItem(friend_item);
    log_file_item->setText(0, filename_str);
}

QTreeWidgetItem* MainWindow::itemExist(QTreeWidget *tree_widget, const QString &pattern)
{
    QTreeWidgetItemIterator itemIterator(tree_widget);
    while (*itemIterator)  {
        if ((*itemIterator)->text(0) == pattern){
            return (*itemIterator);
        }
        ++itemIterator;
    }
    return nullptr;
}

QTreeWidgetItem* MainWindow::itemExist(QTreeWidgetItem *item, const QString &pattern)
{
    QTreeWidgetItemIterator itemIterator(item);
    while (*itemIterator)  {
        if ((*itemIterator)->text(0) == pattern){
            return (*itemIterator);
        }
        ++itemIterator;
    }
    return nullptr;
}
