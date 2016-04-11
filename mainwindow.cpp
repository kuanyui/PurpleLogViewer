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
#include <QUrl>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>

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
    // Signals ===============================
    changeButtonToSearch();
    // process
    connect(agProcess, SIGNAL(readyRead()),   this, SLOT(processOutputHandler()));
    connect(agProcess, SIGNAL(finished(int)), this, SLOT(processFinished()));
    connect(agProcess, SIGNAL(finished(int)), this, SLOT(setupDefaultHighlightKeywordFromSearch()));
    // text browser
    connect(ui->tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(tryToOpenThisLogFile(QTreeWidgetItem*,int)));
    connect(ui->tree_widget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(highlightKeyword()));
    connect(ui->highlight_keyword, SIGNAL(textChanged(QString)), this, SLOT(highlightKeyword(QString)));
    // highlight search
    connect(ui->highlight_keyword_next, SIGNAL(clicked(bool)), this, SLOT(nextHighlight()));
    connect(ui->highlight_keyword_previous, SIGNAL(clicked(bool)), this, SLOT(previousHighlight()));

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
    QString keyword = ui->keyword->text();
    if (keyword.isEmpty()){
        QStringList patterns = QStringList();
        patterns << "*.html";
        QDirIterator it(getCurrentPath(), patterns, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            addTreeWidgetItem(it.next());
            ui->statusBar->showMessage(it.next());
        }
        ui->tree_widget->sortItems(0, Qt::DescendingOrder);
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
    ui->tree_widget->sortItems(0, Qt::DescendingOrder);
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


// ==========================================================
// Text Browser
// ==========================================================

void MainWindow::tryToOpenThisLogFile(QTreeWidgetItem *item, int column){
    if (item->childCount() == 0) {
        QString html_file   = item->text(0);
        QString friend_chat = item->parent()->text(0);
        QString account     = item->parent()->parent()->text(0);
        QString protocol    = item->parent()->parent()->parent()->text(0);
        QString path = LOG_ROOT + protocol + "/" + account + "/" + friend_chat + "/" + html_file;
        QUrl url = QUrl::fromLocalFile(path);
        ui->text_browser->setSource(url);
    }
}
// ==========================================================
// Highlight
// ==========================================================

void MainWindow::setupDefaultHighlightKeywordFromSearch(){
    QString search_keyword = ui->keyword->text();
    ui->highlight_keyword->setText(search_keyword);
}

void MainWindow::highlightKeyword(){
    highlightKeyword(ui->highlight_keyword->text());
}

  // [Example] https://doc.qt.io/archives/4.6/uitools-textfinder.html

void MainWindow::highlightKeyword(QString keyword){
    QTextDocument *document = ui->text_browser->document();
    ui->text_browser->undo();   // try to undo, no matter has hightlighted or not.
    ui->text_browser->setUndoRedoEnabled(false);
    ui->text_browser->setUndoRedoEnabled(true);

    if (!keyword.isEmpty()) {
        QTextCursor highlightCursor(document);
        QTextCursor cursor(document);
        m_highlightedPositions.clear(); // record matched position
        m_highlightedIndex = 0;
        cursor.beginEditBlock(); // Begin Edit ----------------------------
        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document->find(keyword, highlightCursor);
            QTextCharFormat highlightFormat (highlightCursor.charFormat());
            highlightFormat.setBackground(Qt::yellow);
            highlightFormat.setForeground(Qt::black);
            if (!highlightCursor.isNull()) {
                highlightCursor.mergeCharFormat(highlightFormat);
                m_highlightedPositions.append(highlightCursor);
            }
        }
        cursor.endEditBlock(); // End Edit --------------------------------
    }

    if (m_highlightedPositions.isEmpty()){
        ui->highlight_keyword->setStyleSheet("color: #fff;background-color: #f88");
    } else {
        ui->highlight_keyword->setStyleSheet("color: #000;background-color: #8f8");
        ui->text_browser->setTextCursor(m_highlightedPositions.first()); // jump to the first cursor
    }
}


void MainWindow::nextHighlight(){
    m_highlightedIndex++;
    m_highlightedIndex = m_highlightedIndex % m_highlightedPositions.length();
    ui->text_browser->setTextCursor(m_highlightedPositions.at(m_highlightedIndex));
}

void MainWindow::previousHighlight(){
    m_highlightedIndex--;
    if (m_highlightedIndex < 0)
        m_highlightedIndex = m_highlightedPositions.length() - 1;
    ui->text_browser->setTextCursor(m_highlightedPositions.at(m_highlightedIndex));
}
