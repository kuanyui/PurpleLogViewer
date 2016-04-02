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


void MainWindow::searchButtonClicked() {
    QStringList args;
    args << "--files-with-matches" << "--ackmate" << "a b c" << LOG_ROOT + "irc/kuanyui@irc.freenode.org";
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
    protocol_dir = QDir(LOG_ROOT);
    account_dir = QDir();
    friend_dir = QDir();

    connect(ui->protocol_box, SIGNAL( currentIndexChanged(int) ),
            ui->account_box, SLOT( clear() ));
    connect(ui->protocol_box, SIGNAL( currentIndexChanged(int) ),
            ui->friend_box, SLOT( clear() ));
    connect(ui->account_box, SIGNAL( currentIndexChanged(int) ),
            ui->friend_box, SLOT( clear() ));

    for (QString protocol : protocol_dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot)){
        ui->protocol_box->addItem(protocol);
    }

}



void MainWindow::updateAccountsSelector()
{

}


void MainWindow::updateChatroomsSelector()
{

}

// ==========================================================
// QTreeWidget
// ==========================================================


void MainWindow::addTreeWidgetItem(QString path)
{
    qDebug() << path;
}
