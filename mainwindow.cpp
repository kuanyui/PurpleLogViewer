#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QProcess>
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
    args << "-l" << "kuanyui" << "/home/kuanyui/.purple/logs/irc/kuanyui@irc.freenode.org/";
    agProcess->start("ag", args);
}


void MainWindow::processOutputHandler()
{
    //QByteArray raw_output = agProcess->readAllStandardOutput();
    QMessageBox::information(this, "Title", "shit");
    //QMessageBox::information(this, "Title", QString(raw_output.data()));
    //ui->html_browser->append(QString(raw_output.data()));
}


void MainWindow::processFinished()
{
    QMessageBox::information(this, "Title", "Ag process finished!");
}
