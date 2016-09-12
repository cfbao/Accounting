#ifndef MYFILEINFO_H
#define MYFILEINFO_H
#include <QFileInfo>

class MyFileInfo : public QFileInfo
{
public:
    MyFileInfo() { QFileInfo(); newFile = false; }
    bool fileOpened() { return !(filePath().isEmpty()); }
    bool isNewFile() { return newFile; }
    void setNewFile(bool newFile) { this->newFile = newFile; }
    QString dirPath();
    static QString dirPath(const QString &file);
    int setValidFile(const QString &file);
    static int checkValidFile(QString &file);
private:
    bool newFile;
};

#endif // MYFILEINFO_H
