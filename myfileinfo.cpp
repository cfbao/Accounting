#include "myfileinfo.h"
#include <QRegularExpression>

QString MyFileInfo::dirPath(const QString &file)
{
    QString processedFileName = file;
    processedFileName.remove( QRegularExpression("^\"") );
    processedFileName.remove( QRegularExpression("\"$") );
    processedFileName.remove( QRegularExpression("\\$") );
    processedFileName.remove( QRegularExpression("/$") );

    if(!exists(processedFileName)) return QString("");
    QFileInfo tmp(processedFileName);
    if(tmp.isDir()) return tmp.absoluteFilePath();
    else return tmp.absolutePath();
}

QString MyFileInfo::dirPath()
{
    if( filePath().isEmpty() || !exists() ) return QString("");
    if( isDir() ) return absoluteFilePath();
    else return absolutePath();
}

int MyFileInfo::setValidFile(const QString &file)
/* return 0 if "file" doesn't exist
   return 1 if "file" exist and is a file
   return 2 if "file" exist but is not a file (likely a directory)
*/
{
    QString processedFileName = file;
    processedFileName.remove( QRegularExpression("^\"") );
    processedFileName.remove( QRegularExpression("\"$") );
    processedFileName.remove( QRegularExpression("\\$") );
    processedFileName.remove( QRegularExpression("/$") );
    if(exists(processedFileName)){
        setFile(processedFileName);
        if(isFile())
            return 1;
        else
            return 2;
    } else
        return 0;
}

int MyFileInfo::checkValidFile(QString &file)
/* remove any starting/ending quotes and ending slash of "file"
   return 0 if "file" doesn't exist
   return 1 if "file" exist and is a file
   return 2 if "file" exist but is not a file (likely a directory)*/
{
    file.remove( QRegularExpression("^\"") );
    file.remove( QRegularExpression("\"$") );
    file.remove( QRegularExpression("\\$") );
    file.remove( QRegularExpression("/$") );
    if(exists(file)){
        if(QFileInfo(file).isFile())
            return 1;
        else
            return 2;
    } else
        return 0;
}
