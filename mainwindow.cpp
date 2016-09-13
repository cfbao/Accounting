#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ledger.h"
#include "ledgerview.h"
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QString>
#include <QFont>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QStandardPaths>
#include <QDesktopWidget>

QFont MainWindow::textFont = MainWindow::defaultTextFont();
QFont MainWindow:: numFont = MainWindow::defaultNumFont();

MainWindow::MainWindow(QString filePath, QWidget *parent) : QMainWindow(parent)
{
    // setup ui
    ui = (new Ui::MainWindow);
    ui->setupUi(this);
    ui->centralWidget->setFont(textFont);

    // setup ledgerView
    ui->ledgerView->setLedger(Q_NULLPTR);
    connect(ui->ledgerView,&LedgerView::filesDropped,this,&MainWindow::filesDropped);

    openingMsg = new QLabel(tr("Opening File..."),ui->ledgerView);
    QFont font = QApplication::font(); font.setPointSize(14);
    openingMsg->setFont(font);
    openingMsg->setAlignment(Qt::AlignCenter);
    openingMsg->adjustSize();
    QGridLayout* layout = new QGridLayout;
    layout->addWidget(openingMsg);
    delete ui->ledgerView->layout();
    ui->ledgerView->setLayout(layout);

    ui->totalBox->setReadOnly(true);
    ui->netBox  ->setReadOnly(true);
    ui->debtBox ->setReadOnly(true);
    ui->loanBox ->setReadOnly(true);
    ui->totalBox->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->netBox  ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->debtBox ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->loanBox ->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->totalBox->setFont(numFont);
    ui->netBox  ->setFont(numFont);
    ui->debtBox ->setFont(numFont);
    ui->loanBox ->setFont(numFont);

    // setup Actions
    ui->actionNew->setShortcut(QKeySequence::New);
    ui->actionOpen->setShortcut(QKeySequence::Open);
    ui->actionSave->setShortcut(QKeySequence::Save);
    ui->actionSaveAs->setShortcut(QKeySequence::SaveAs);
    ui->actionExit->setShortcut(QKeySequence::Quit);
    ui->actionClose->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionSaveAs->setEnabled(false);
    setWindowModified(false);

    // set window
    setWindowTitle(qApp->applicationName());
    // set file path
    currentFile.setFile(filePath);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::initialize(bool defaultDir)
{
    show();

    ledger = Q_NULLPTR;
    if( currentFile.isDir() ){
        QDir::setCurrent(currentFile.absoluteFilePath());
        currentFile.setFile("");
        on_actionOpen_triggered();
    } else if( currentFile.fileOpened() )
        openFile(currentFile.absoluteFilePath());
    else {
        if( defaultDir ){
            QStringList docPaths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
            int i = 0;
            while( i<docPaths.length() && !QDir::setCurrent(docPaths.at(i)) )
                i++;
            if( i==docPaths.length() )
                QDir::setCurrent(QDir::homePath());
        }
        on_actionNew_triggered();
    }
}

void MainWindow::filesDropped(const QList<QUrl> &urls)
{
    foreach (const QUrl & url, urls) {
        QString filePath = url.toLocalFile();
        if(QFileInfo(filePath).isDir()){
            QDir::setCurrent(filePath);
            on_actionOpen_triggered();
        } else
            openFile(filePath);
    }
}

void MainWindow::openFile(const QString &filePath)
{
    if(filePath.isEmpty()) return;
    QFile inputFile(filePath);
    if(inputFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        if( ledger && ledger->isValidFile() && !(currentFile.isNewFile() && !isWindowModified()) ){
            MainWindow* w = new MainWindow(filePath);
            int offset = std::abs(geometry().y() - frameGeometry().y());
            w->move( pos().x()+offset, pos().y()+offset );
            if(!qApp->desktop()->availableGeometry(this).contains(w->frameGeometry()))
                w->move(qApp->desktop()->availableGeometry(this).topLeft());
            w->initialize(false);
            return;
        }
        setWindowTitle(tr("Opening \"")+QFileInfo(filePath).fileName()+"\"... - "+qApp->applicationName());
        QTextStream inputStream(&inputFile);
        inputStream.setCodec("UTF-8");
        inputStream.setAutoDetectUnicode(true);
        ui->actionNew->setEnabled(false);
        ui->actionOpen->setEnabled(false);
        ui->actionSave->setEnabled(false);
        ui->actionSaveAs->setEnabled(false);
        ui->actionClose->setEnabled(false);
        setupLedger(&inputStream);
        inputFile.close();

        setPath(filePath);
        setWindowModified(false);
        ui->actionNew->setEnabled(true);
        ui->actionOpen->setEnabled(true);
        ui->actionSave->setEnabled(false);
        ui->actionSaveAs->setEnabled( ledger->isValidFile() );
        ui->actionClose->setEnabled(true);
        currentFile.setNewFile(false);
    }
    else{
        QMessageBox::warning(this, "", tr("Could not open file"));
    }
}

/* Save file at "filePath"
   return true if successfully saved
   return false if not saved */
bool MainWindow::saveFile(const QString &filePath)
{
    if(filePath.isEmpty()) return false;
    QFile outputFile(filePath);
    if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        setWindowTitle(QFileInfo(filePath).fileName()+"[*] - "+qApp->applicationName()+tr(" - Saving..."));
        QTextStream outputStream(&outputFile);
        outputStream.setCodec("UTF-8");
        ledger->writeCsv(outputStream);
        outputFile.close();

        setPath(filePath);
        ui->actionSaveAs->setEnabled(true);
        ui->actionClose->setEnabled(true);
        currentFile.setNewFile(false);
        return true;
    } else{
        QMessageBox::critical(this, tr("Error"), tr("Could not save file"));
        return false;
    }
}

// input "filePath" must be VALID file path
void MainWindow::setPath(const QString & filePath)
{
    currentFile.setValidFile(filePath);
    QDir::setCurrent(currentFile.dirPath());
    setWindowTitle(currentFile.fileName()+"[*] - "+qApp->applicationName());
}

void MainWindow::setupLedger(QTextStream * inputStream)
{
    delete ledger;
    ledger = (new Ledger(ui->totalBox,ui->netBox,ui->debtBox,ui->loanBox));
    QAction* undoAction = ledger->undoStack->createUndoAction(ui->menu_Edit,tr("&Undo"));
    QAction* redoAction = ledger->undoStack->createRedoAction(ui->menu_Edit,tr("&Redo"));
    ui->totalBox->setText("");
    ui->netBox  ->setText("");
    ui->debtBox ->setText("");
    ui->loanBox ->setText("");
    ui->menu_Edit->clear();
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction->setShortcut(QKeySequence::Redo);
    ui->menu_Edit->addAction(undoAction);
    ui->menu_Edit->addAction(redoAction);
    ui->menu_Edit->addSeparator();
    ui->menu_Edit->addActions(ui->ledgerView->rowActions());

    openingMsg->show();
    ledger->readFile(inputStream);
    openingMsg->hide();
    ui->ledgerView->setLedger(ledger);

    connect(ledger,&Ledger::ledgerChanged,this,&MainWindow::setWindowModified);
    connect(ledger,&Ledger::ledgerChanged,ui->actionSave,&QAction::setEnabled);
}

bool MainWindow::confirmSaveClose()
{
    if(isWindowModified()){
        QMessageBox::StandardButton button =
                QMessageBox::question(this,tr("Save"),QString(tr("Save file ")+currentFile.fileName()+tr("?")),
                                      QMessageBox::StandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel),QMessageBox::Yes);
        if(button==QMessageBox::Yes)
            return on_actionSave_triggered();
        else if(button==QMessageBox::No)
            return true;
        else
            return false;
    }
    return true;
}

void MainWindow::on_actionNew_triggered()
{
    if(ledger && ledger->isValidFile()){
        MainWindow* w = new MainWindow();
        int offset = std::abs(geometry().y() - frameGeometry().y());
        w->move( pos().x()+offset, pos().y()+offset );
        if(!qApp->desktop()->availableGeometry(this).contains(w->frameGeometry()))
            w->move(qApp->desktop()->availableGeometry(this).topLeft());
        w->initialize(false);
        return;
    }
    currentFile.setFile(QDir::current(),tr("New Sheet")+".csv");
    setWindowTitle(currentFile.fileName()+"[*] - "+qApp->applicationName());
    setupLedger();
    ui->actionClose->setEnabled(true);
    ui->actionSave->setEnabled(false);
    ui->actionSaveAs->setEnabled(true);
    setWindowModified(false);
    currentFile.setNewFile(true);
}

void MainWindow::on_actionOpen_triggered()
{
    QString filePath = QFileDialog::getOpenFileName
            (this,QString(),QDir::currentPath(),tr("CSV Files")+" (*.csv);;"+tr("All Files")+" (*.*)",0,QFileDialog::DontResolveSymlinks);
    openFile(filePath);
}

bool MainWindow::on_actionSave_triggered()
{
    if(currentFile.isNewFile()){
        QString filePath = QFileDialog::getSaveFileName
                (this,QString(), currentFile.absoluteFilePath(),tr("CSV Files")+" (*.csv);;"+tr("All Files")+" (*.*)",0,QFileDialog::DontResolveSymlinks);
        return saveFile(filePath);
    }
    else return saveFile(currentFile.absoluteFilePath());
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString filePath = QFileDialog::getSaveFileName
                (this,QString(), QDir::currentPath(),tr("CSV Files")+" (*.csv)",0,QFileDialog::DontResolveSymlinks);
    saveFile(filePath);
}

void MainWindow::on_actionClose_triggered()
{
    if(!confirmSaveClose()) return;
    currentFile.setFile("");
    setWindowTitle(qApp->applicationName());
    ui->ledgerView->setLedger(Q_NULLPTR);
    delete ledger; ledger = Q_NULLPTR;
    ui->totalBox->setText("");
    ui->netBox  ->setText("");
    ui->debtBox ->setText("");
    ui->loanBox ->setText("");
    ui->actionClose->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionSaveAs->setEnabled(false);
    setWindowModified(false);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(confirmSaveClose())
        event->accept();
    else
        event->ignore();
}

