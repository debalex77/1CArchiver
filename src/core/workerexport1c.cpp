#include "workerexport1c.h"
#include "src/utils.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

static QString export1CConfigDir()
{
    /** aceeasi locatie ca in PluginConfigDialog::saveConfig() */
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
           + "/plugins/export_1c";
}

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

WorkerExport1C::WorkerExport1C(QObject *parent)
    : QObject(parent)
{
}

WorkerExport1C::~WorkerExport1C()
{
    cancel();
}

QString WorkerExport1C::findConfig(const QString &dbName, const QString &dbFolder)
{
    const QDir dir(export1CConfigDir());

    /** 1. config individual: dupa denumirea din tabel, apoi dupa numele folderului */
    QStringList names;
    if (!dbName.trimmed().isEmpty())
        names << dbName.trimmed();

    const QString folderName = QFileInfo(QDir::cleanPath(dbFolder)).fileName();
    if (!folderName.isEmpty() && !names.contains(folderName))
        names << folderName;

    for (const QString &n : std::as_const(names)) {
        const QString p = dir.filePath(n + ".json");
        if (QFile::exists(p))
            return p;
    }

    /** 2. config global pentru toate bazele */
    const QString global = dir.filePath("global_all_bases.json");
    if (QFile::exists(global))
        return global;

    return QString();
}

bool WorkerExport1C::isServerConfig(const QString &configPath)
{
    const QJsonObject o = loadJsonObject(configPath, nullptr);
    return !o.value("applyAllDB").toBool(false)
           && o.value("typeDB").toString() == "MSSQL";
}

void WorkerExport1C::setConfigFile(const QString &path)
{
    QString err;
    const QJsonObject cfg = loadJsonObject(path, &err);
    if (cfg.isEmpty()) {
        /** semnalul nu e inca conectat aici -> eroarea se raporteaza in process() */
        m_configError = "Config error: " + err;
        return;
    }
    m_configError.clear();

    m_platform = cfg.value("platform").toString().trimmed();
    m_typeDB   = cfg.value("typeDB").toString();
    m_server   = cfg.value("server").toString().trimmed();
    m_database = cfg.value("database").toString().trimmed();
    m_dbPath   = cfg.value("dbPath").toString().trimmed();
    m_user     = cfg.value("user").toString();
    m_pass     = decryptPassword(cfg.value("password").toString());
    m_global   = cfg.value("applyAllDB").toBool(false);
}

void WorkerExport1C::setDbFolder(const QString &folder)
{
    m_dbFolder = folder;
}

void WorkerExport1C::setOutputDt(const QString &dtPath)
{
    m_outputDt = dtPath;
}

void WorkerExport1C::process()
{
    if (!m_configError.isEmpty()) {
        finish(false, m_configError);
        return;
    }

    if (m_platform.isEmpty() || !QFile::exists(m_platform)) {
        finish(false, tr("1cv8.exe nu a fost găsit: %1")
                          .arg(QDir::toNativeSeparators(m_platform)));
        return;
    }

    // -------------------------------------------------
    // Conexiunea la baza de date
    // -------------------------------------------------
    QStringList args;
    args << "DESIGNER";

    if (!m_global && m_typeDB == "MSSQL") {
        /** baza de server 1C -> /S"server\baza" */
        if (m_server.isEmpty() || m_database.isEmpty()) {
            finish(false, tr("Config export 1C: lipsește serverul sau denumirea bazei."));
            return;
        }
        args << "/S" << (m_server + "\\" + m_database);
    } else {
        /** baza de tip fisier -> /F"folder"; config-ul global foloseste folderul din tabel */
        const QString folder = (!m_global && !m_dbPath.isEmpty()) ? m_dbPath : m_dbFolder;
        if (folder.isEmpty()) {
            finish(false, tr("Config export 1C: lipsește calea spre baza de date."));
            return;
        }
        args << "/F" << QDir::toNativeSeparators(QDir::cleanPath(folder));
    }

    if (!m_user.isEmpty())
        args << "/N" << m_user;
    if (!m_pass.isEmpty())
        args << "/P" << m_pass;

    // -------------------------------------------------
    // Exportul + log-ul platformei
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
            &WorkerExport1C::onProcessFinished);
    connect(m_proc, &QProcess::errorOccurred,
            this, &WorkerExport1C::onProcessError);

    emit log(tr("📤 Export 1C: %1 DESIGNER /DumpIB ...")
                 .arg(QDir::toNativeSeparators(m_platform)));
    m_proc->start(m_platform, args);
}

void WorkerExport1C::cancel()
{
    if (m_proc && m_proc->state() != QProcess::NotRunning)
        m_proc->kill();
}

void WorkerExport1C::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const QString outLog = readOutLog();
    if (!outLog.isEmpty())
        emit log(tr("Export 1C (log platformă): %1").arg(outLog));

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

void WorkerExport1C::onProcessError(QProcess::ProcessError error)
{
    /** restul erorilor sunt urmate de finished() */
    if (error == QProcess::FailedToStart)
        finish(false, tr("Nu pot porni 1cv8.exe: %1").arg(m_proc->errorString()));
}

QString WorkerExport1C::readOutLog() const
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

void WorkerExport1C::finish(bool ok, const QString &error)
{
    if (m_done)
        return;
    m_done = true;

    emit finished(ok, ok ? m_outputDt : QString(), error);
}
