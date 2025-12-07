#pragma once
#include <QObject>
#include <QString>
#include <bit7z/bit7zlibrary.hpp>
#include <bit7z/bitfilecompressor.hpp>
#include <bit7z/bitformat.hpp>

class CompressWorker : public QObject {
    Q_OBJECT
public:
    CompressWorker(QString input,
                   QString output,
                   int level,
                   bool isFolder,
                   uint64_t totalSize,
                   const QString& password = QString());

signals:
    void progress(int pct);
    void finished(bool ok, QString output);
    void error(QString msg);

public slots:
    void process();

private:
    QString  m_input;
    QString  m_output;
    int      m_level;
    bool     m_isFolder;
    uint64_t m_totalSize;
    QString  m_password;
};


