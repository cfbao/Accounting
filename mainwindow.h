#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QFileInfo>
#include <QFont>
#include <QUrl>
#include <QLabel>
#include "ledger.h"
#include "myfileinfo.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QString filePath = QString(), QWidget *parent = 0);
    ~MainWindow();
    void initialize(bool defaultDir = true);
    static QFont textFont, numFont;
    static QFont defaultTextFont() { QFont font("Calibri",11); font.insertSubstitution("Calibri","Microsoft YaHei"); return font; }
    static QFont defaultNumFont() { return QFont("Lucida Console",11); }
public slots:
    void filesDropped(const QList<QUrl> &urls);
protected:
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
private slots:
    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    bool on_actionSave_triggered();

    void on_actionSaveAs_triggered();

    void on_actionClose_triggered();

    void on_actionExit_triggered() { close(); }

    void setWindowClean(bool clean) { setWindowModified(!clean); }
private:
    Ui::MainWindow *ui;
    Ledger * ledger;
    MyFileInfo currentFile;
    QLabel* openingMsg;
    void openFile(const QString &filePath);
    bool saveFile(const QString &filePath);
    void setPath(const QString &filePath);
    void setupLedger(QTextStream *inputStream=Q_NULLPTR);
    bool confirmSaveClose();
};

#endif // MAINWINDOW_H
