#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QTreeWidget>

#define LOG_ROOT QDir::homePath() + QString("/.purple/logs/")
#define CBOX_EMPTY_STR "--All--"

// http://log.noiretaya.com/200

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    agProcess = new QProcess(this);
    setupPathSelector();
    // Setup process
    connect(agProcess, SIGNAL(finished(int)),
            this,      SLOT(processFinished()));
    connect(agProcess, SIGNAL(readyRead()),
            this,      SLOT(processOutputHandler()));
    // click button
    connect(ui->search_button, SIGNAL(clicked()),
            this,              SLOT(searchButtonClicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
    if (agProcess->state() == QProcess::Running)
    {
        agProcess->kill();
        agProcess->waitForFinished();
    }
    delete agProcess;
}


QString MainWindow::getKeyword(){
    return ui->keyword->text();
}

void MainWindow::searchButtonClicked() {
    QStringList args;
    args << "--files-with-matches" << "--ackmate" << getKeyword() << getCurrentPath();
    agProcess->start("ag", args);
}


void MainWindow::processOutputHandler()
{
    while ( agProcess->canReadLine() ) {
        QString path = QString(agProcess->readLine()).mid(1); // mid(1) due to "--ackmate" argument
        addTreeWidgetItem(path);
    }
}


void MainWindow::processFinished()
{
    QMessageBox::information(this, "Title", "Ag process finished!");
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
        ui->account_box->addItems(qdir.entryList());
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
        ui->friend_box->addItems(qdir.entryList());
    }
}

QString MainWindow::getCurrentPath()
{
    QString protocol = ui->protocol_box->currentText();
    QString account = ui->account_box->currentText();
    QString friend_chat = ui->friend_box->currentText();
    if (protocol == CBOX_EMPTY_STR) { return LOG_ROOT ;};
    if (account == CBOX_EMPTY_STR) { return LOG_ROOT + protocol ;};
    if (friend_chat == CBOX_EMPTY_STR) { return LOG_ROOT + protocol + "/" + account; };
    return LOG_ROOT + protocol + "/" + account + "/"  + friend_chat ;
}

// ==========================================================
// QTreeWidget
// ==========================================================


void MainWindow::addTreeWidgetItem(QString path)
{
    qDebug() << path;
}
