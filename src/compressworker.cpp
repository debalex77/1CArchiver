#include "compressworker.h"
#include <QFileInfo>
#include <QCoreApplication>
#include "src/globals.h"

using namespace bit7z;

CompressWorker::CompressWorker(QString input,
                               QString output,
                               int level,
                               bool isFolder,
                               uint64_t totalSize,
                               const QString &password) :
    m_input(input),
    m_output(output),
    m_level(level),
    m_isFolder(isFolder),
    m_totalSize(totalSize),
    m_password(password)
{

}

void CompressWorker::process() {
    try {

        /*
         * --- 1. Variant (relativ cu indicarea aplicatiei)
         * QString dllPath = QCoreApplication::applicationDirPath() + "/7z.dll";
         * bit7z::Bit7zLibrary lib{ dllPath.toStdWString() };
         *
         * --- 2. Variant (Indicarea nemijlocita unde e instalat 7-zip)
         * Bit7zLibrary lib{"C:/Program Files/7-Zip/7z.dll" };
         *
         * --- 3. Variant (Windows gaseste automat)
         */
        bit7z::Bit7zLibrary lib;
        BitFileCompressor compressor{ lib, BitFormat::SevenZip };

        //--- setam parola
        if (!m_password.isEmpty())
            compressor.setPassword(m_password.toStdString());

        //--- setam compresia
        compressor.setCompressionLevel(static_cast<BitCompressionLevel>(m_level));

        //--- ne conectam la progresul
        compressor.setProgressCallback([this](uint64_t done) {
            int pct = int(double(done) / double(m_totalSize) * 100.0);
            if (pct > 100) pct = 100;
            emit progress(pct);
            return true;
        });

        //--- compresia folderul-ui sau a fisierului
        if (m_isFolder) {
            compressor.compressDirectory(
                m_input.toStdString(),
                m_output.toStdString()
                );
        } else {
            compressor.compressFile(
                m_input.toStdString(),
                m_output.toStdString()
                );
        }

        emit backupCreated(m_output);
        emit finished(true, m_output, QString());

    } catch (const BitException& ex) {
        // emit error(QString::fromStdString(ex.what()));
        // emit finished(false, m_output);
        const QString msg = QString::fromStdString(ex.what());
        emit error(msg);
        emit finished(false, m_output, msg);
    }
}
