#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
#include <QDebug>
#include <QString>
#include <QStringList>
// http://log.noiretaya.com/200

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    agProcess = new QProcess(this);
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
    args << "--files-with-matches" << "--ackmate" << "a b c" << "/home/kuanyui/.purple/logs/irc/kuanyui@irc.freenode.org";
    agProcess->start("ag", args);
}


void MainWindow::processOutputHandler()
{
    while ( agProcess->canReadLine() ) {
        ui->html_browser->append(QString(agProcess->readLine()));
    }
}


void MainWindow::processFinished()
{
    QMessageBox::information(this, "Title", "Ag process finished!");
}
