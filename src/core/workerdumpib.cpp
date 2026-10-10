#include "workerdumpib.h"
#include "src/utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVersionNumber>

static QJsonObject loadJsonObject(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = "Cannot open config file: " + QDir::toNativeSeparators(path);
        return {};
    }

    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error)
            *error = pe.error != QJsonParseError::NoError
                         ? pe.errorString()
                         : QStringLiteral("Invalid JSON structure");
        return {};
    }

    return doc.object();
}

WorkerDumpIB::WorkerDumpIB(QObject *parent)
    : QObject(parent)
{
}

WorkerDumpIB::~WorkerDumpIB()
{
    cancel();
}

QString WorkerDumpIB::findPlatform()
{
    QStringList roots;
    for (const char *var : {"ProgramW6432", "ProgramFiles", "ProgramFiles(x86)"}) {
        const QString p = qEnvironmentVariable(var);
        if (!p.isEmpty() && !roots.contains(p, Qt::CaseInsensitive))
            roots << p;
    }

    QString best;
    QVersionNumber bestVer;

    for (const QString &root : std::as_const(roots)) {
        const QDir dir(root + "/1cv8");
        const QStringList versions = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        for (const QString &v : versions) {
            const QVersionNumber ver = QVersionNumber::fromString(v);
            if (ver.isNull())
                continue; /** ex. "common", "srvinfo" */

            const QString exe = dir.filePath(v + "/bin/1cv8.exe");
            if (QFile::exists(exe) && ver > bestVer) {
                bestVer = ver;
                best    = exe;
            }
        }
    }

    return QDir::toNativeSeparators(best);
}

void WorkerDumpIB::setConfigFile(const QString &path)
{
    QString err;
    const QJsonObject cfg = loadJsonObject(path, &err);
    if (cfg.isEmpty()) {
        /** apelat inainte de connect() -> semnalul s-ar pierde; il emitem in process() */
        m_configError = "Config error: " + err;
        return;
    }

    m_platform = cfg.value("platform").toString().trimmed();
    m_server1c = cfg.value("server1c").toString().trimmed();
    m_infobase = cfg.value("infobase").toString().trimmed();
    m_user     = cfg.value("user").toString().trimmed();
    m_pass     = decryptPassword(cfg.value("password").toString());
}

void WorkerDumpIB::setDbFolder(const QString &folder)
{
    m_dbFolder = folder;
}

void WorkerDumpIB::setOutputDt(const QString &dtPath)
{
    m_outputDt = dtPath;
}

void WorkerDumpIB::process()
{
    if (!m_configError.isEmpty()) {
        finish(false, m_configError);
        return;
    }

    if (m_platform.isEmpty())
        m_platform = findPlatform();

    if (m_platform.isEmpty() || !QFile::exists(m_platform)) {
        finish(false, m_platform.isEmpty()
                          ? tr("Platforma 1C (1cv8.exe) nu a fost găsită.")
                          : tr("1cv8.exe nu a fost găsit: %1")
                                .arg(QDir::toNativeSeparators(m_platform)));
        return;
    }

    // -------------------------------------------------
    // 1. Conexiunea la baza 1C
    // -------------------------------------------------
    QStringList args;
    args << "DESIGNER";

    if (!m_dbFolder.isEmpty()) {
        /** baza de tip fisier -> /F"folder" */
        args << "/F" << QDir::toNativeSeparators(QDir::cleanPath(m_dbFolder));
    } else {
        /** baza de server 1C -> /S"server\baza" */
        if (m_server1c.isEmpty() || m_infobase.isEmpty()) {
            finish(false, tr("Config export .dt: lipsește serverul 1C sau denumirea bazei."));
            return;
        }
        args << "/S" << (m_server1c + "\\" + m_infobase);
    }

    /** platforma nu accepta alta metoda de transmitere a parolei (vizibila in lista proceselor) */
    if (!m_user.isEmpty())
        args << "/N" << m_user;
    if (!m_pass.isEmpty())
        args << "/P" << m_pass;

    // -------------------------------------------------
    // 2. Exportul + log-ul platformei
    // -------------------------------------------------
    m_outLog = QDir(QDir::tempPath()).filePath(
        QFileInfo(m_outputDt).completeBaseName() + "_dumpib.log");
    QFile::remove(m_outLog);
    QFile::remove(m_outputDt);

    args << "/DumpIB" << QDir::toNativeSeparators(m_outputDt)
         << "/Out" << QDir::toNativeSeparators(m_outLog)
         << "/DisableStartupDialogs"
         << "/DisableStartupMessages";

    m_proc = new QProcess(this);

    connect(m_proc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &WorkerDumpIB::onProcessFinished);
    connect(m_proc, &QProcess::errorOccurred,
            this, &WorkerDumpIB::onProcessError);

    emit log(tr("📤 Export .dt: %1 DESIGNER /DumpIB ...")
                 .arg(QDir::toNativeSeparators(m_platform)));
    m_proc->start(m_platform, args);
}

void WorkerDumpIB::cancel()
{
    if (m_proc && m_proc->state() != QProcess::NotRunning)
        m_proc->kill();
}

void WorkerDumpIB::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const QString outLog = readOutLog();
    if (!outLog.isEmpty())
        emit log(tr("Export .dt (log platformă): %1").arg(outLog));

    const QFileInfo dt(m_outputDt);
    const bool ok = status == QProcess::NormalExit
                    && exitCode == 0
                    && dt.exists()
                    && dt.size() > 0;

    if (ok) {
        finish(true, QString());
        return;
    }

    /** fisier .dt incomplet -> nu il pastram */
    QFile::remove(m_outputDt);
    finish(false, tr("1cv8.exe a returnat codul %1").arg(exitCode));
}

void WorkerDumpIB::onProcessError(QProcess::ProcessError error)
{
    /** restul erorilor sunt urmate de finished() */
    if (error == QProcess::FailedToStart)
        finish(false, tr("Nu pot porni 1cv8.exe: %1").arg(m_proc->errorString()));
}

QString WorkerDumpIB::readOutLog() const
{
    QFile f(m_outLog);
    if (!f.open(QIODevice::ReadOnly))
        return QString();

    const QByteArray raw = f.readAll();
    f.close();
    QFile::remove(m_outLog);

    /** platforma scrie UTF-8 cu BOM sau in codificarea ANSI a sistemului */
    const QString text = raw.startsWith("\xEF\xBB\xBF")
                             ? QString::fromUtf8(raw.mid(3))
                             : QString::fromLocal8Bit(raw);
    return text.trimmed();
}

void WorkerDumpIB::finish(bool ok, const QString &error)
{
    if (m_done)
        return;
    m_done = true;

    emit finished(ok, ok ? m_outputDt : QString(), error);
}
