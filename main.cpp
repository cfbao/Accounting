#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName(QObject::tr("AT Accountning"));

    // command line parsing
    QStringList cmdArg = QCoreApplication::arguments();
    QString filePath;
    if( cmdArg.length()<=1 )
        filePath = "";
    else
        filePath = cmdArg.last();

    MainWindow w(filePath);
    w.initialize(true);
    return a.exec();
}
