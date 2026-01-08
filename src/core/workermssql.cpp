#include "WorkerMssql.h"
#include "src/utils.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

static QVariantMap loadJsonConfig(const QString &path, QString *error)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (error)
            *error = "Cannot open config file";
        return {};
    }

    QJsonParseError pe;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);

    if (pe.error != QJsonParseError::NoError) {
        if (error)
            *error = pe.errorString();
        return {};
    }

    if (!doc.isObject()) {
        if (error)
            *error = "Invalid JSON structure";
        return {};
    }

    return doc.object().toVariantMap();
}

WorkerMssql::WorkerMssql(QObject *parent)
    : QObject(parent)
{
}

WorkerMssql::~WorkerMssql()
{
    cancel();
    closeProgressDb();
}

void WorkerMssql::setConfigFile(const QString &path)
{
    QString err;
    QVariantMap dbCfg = loadJsonConfig(path, &err);
    if (dbCfg.isEmpty()) {
        emit finished(false, QString(), "Config error: " + err);
        return;
    }
    m_server   = dbCfg.value("server").toString();
    m_database = dbCfg.value("database").toString();
    m_auth     = dbCfg.value("auth").toString();
    m_user     = dbCfg.value("user").toString();
    m_pass     = decryptPassword(dbCfg.value("password").toString());
}

void WorkerMssql::setOutputBak(const QString &bakPath)
{
    m_outputBak = bakPath;
}

void WorkerMssql::process()
{
    emit log(tr("MSSQL backup started"));

    // -------------------------------------------------
    // 1. Pornim sqlcmd (BACKUP DATABASE)
    // -------------------------------------------------
    m_proc = new QProcess(this);

    connect(m_proc,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &WorkerMssql::onSqlcmdFinished);

    QString sql =
        QString("BACKUP DATABASE [%1] TO DISK='%2' "
                "WITH INIT, COMPRESSION")
            .arg(m_database, m_outputBak);

    QStringList args;
    args << "-S" << m_server
         << "-b"
         << "-W"
         << "-Q" << sql;

    QString auth = m_auth;
    if (auth == "windows") {
        args << "-E";
    } else {
        args << "-U" << m_user
             << "-P" << m_pass;
    }

    emit log(tr("Executing sqlcmd..."));
    m_proc->start("sqlcmd", args);

    if (!m_proc->waitForStarted(3000)) {
        emit finished(false, QString(),
                      tr("Cannot start sqlcmd"));
        return;
    }

    // -------------------------------------------------
    // 2. Pornim progres REAL (SSMS-like)
    // -------------------------------------------------
    if (openProgressDb()) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);

        connect(m_timer, &QTimer::timeout,
                this, &WorkerMssql::pollProgress);

        m_timer->start();
    } else {
        emit log(tr("⚠ Cannot open MSSQL progress connection"));
    }
}

void WorkerMssql::cancel()
{
    if (m_proc && m_proc->state() != QProcess::NotRunning)
        m_proc->kill();

    if (m_timer)
        m_timer->stop();
}

void WorkerMssql::onSqlcmdFinished(int exitCode,
                                   QProcess::ExitStatus status)
{
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }

    closeProgressDb();

    const bool ok =
        (status == QProcess::NormalExit && exitCode == 0);

    if (ok)
        emit progress(100);

    emit finished(ok,
                  m_outputBak,
                  ok ? QString()
                     : tr("sqlcmd exited with code %1")
                           .arg(exitCode));
}

bool WorkerMssql::openProgressDb()
{
    if (QSqlDatabase::contains("mssql_progress"))
        m_db = QSqlDatabase::database("mssql_progress");
    else
        m_db = QSqlDatabase::addDatabase("QODBC",
                                         "mssql_progress");

    QString conn;
    conn += "Driver={SQL Server};";
    conn += "Server=" + m_server + ";";
    conn += "Trusted_Connection=Yes;";

    m_db.setDatabaseName(conn);
    if (m_db.open()) {
        return true;
    } else {
        emit log(tr("⚠ Cannot open MSSQL progress connection: %1")
                     .arg(m_db.lastError().text()));
        return false;
    }
}

void WorkerMssql::closeProgressDb()
{
    if (m_db.isValid()) {
        const QString name = m_db.connectionName();
        m_db.close();
        QSqlDatabase::removeDatabase(name);
    }
}

void WorkerMssql::pollProgress()
{
    if (!m_db.isOpen())
        return;

    QSqlQuery q(m_db);
    q.prepare(R"(
        SELECT percent_complete
        FROM sys.dm_exec_requests
        WHERE command = 'BACKUP DATABASE'
          AND database_id = DB_ID(:db)
    )");
    q.bindValue(":db", m_database);

    if (!q.exec())
        return;

    if (q.next()) {
        const int pct = qRound(q.value(0).toDouble());
        emit progress(pct);
    }
}
