#include "pluginconfigdialog.h"
#include "dynamicpluginform.h"
#include "src/utils.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QVersionNumber>

PluginConfigDialog::PluginConfigDialog(const QString &pluginId,
                                       const QString &configFile,
                                       QWidget *parent)
    : QDialog(parent),
    m_pluginId(pluginId),
    m_configFile(configFile)
{
    setWindowTitle(tr("Plugin configuration"));
    resize(420, 300);

    loadSchema();
    loadConfig();

    if (m_pluginId == "export_1c")
        m_platform1CPath = getPlatform1CPath();

    m_form = new DynamicPluginForm(m_schema, this);
    if (!m_platform1CPath.isEmpty())
        m_form->setValue("platform", m_platform1CPath);

    connect(m_form, &DynamicPluginForm::sizeChanged, this, [this]() {
        adjustSize();
    });

    if (!m_config.isEmpty())
        m_form->setValues(m_config.toVariantMap());

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    if (auto *okBtn = buttons->button(QDialogButtonBox::Ok))
        okBtn->setMinimumWidth(80);

    if (auto *cancelBtn = buttons->button(QDialogButtonBox::Cancel))
        cancelBtn->setMinimumWidth(80);

    connect(buttons, &QDialogButtonBox::accepted,
            this, &PluginConfigDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_form);
    layout->addWidget(buttons);
}

void PluginConfigDialog::onAccept()
{
    QString error;
    if (!m_form->validate(&error)) {
        QMessageBox::warning(this, tr("Invalid configuration"), error);
        return;
    }

    /** export_1c pentru o singura baza -> e necesar numele bazei (folosit si ca nume de fisier .json) */
    if (m_pluginId == "export_1c") {
        const QVariantMap v = m_form->values();
        if (!v.value("applyAllDB").toBool() && export1CDbName(v).isEmpty()) {
            QMessageBox::warning(this, tr("Invalid configuration"),
                                 tr("Indicați denumirea bazei de date sau calea spre baza de date."));
            return;
        }
    }

    saveConfig();
    accept();
}

void PluginConfigDialog::loadSchema()
{
    QFile f(schemaPath());
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Cannot load plugin schema."));
        reject();
        return;
    }

    m_schema = QJsonDocument::fromJson(f.readAll()).object();
}

void PluginConfigDialog::loadConfig()
{
    QFile f(m_configFile);
    if (!f.exists())
        return;

    if (!f.open(QIODevice::ReadOnly))
        return;

    m_config = QJsonDocument::fromJson(f.readAll()).object();
}

void PluginConfigDialog::saveConfig()
{
    QVariantMap values = m_form->values();

    QJsonObject obj = m_config;
    for (auto it = values.begin(); it != values.end(); ++it) {
        if (it.key() == "password") {
            const QString plain = it.value().toString();
            if (!plain.isEmpty())
                obj[it.key()] = encryptPassword(plain);
        } else {
            obj[it.key()] = QJsonValue::fromVariant(it.value());
        }
    }

    obj["configured"] = true;

    /** recalculam la fiecare salvare (si la editare), nu doar la crearea config-ului */
    const bool applyAllBases =
        m_pluginId == "export_1c" && obj.value("applyAllDB").toBool(false);
    obj["export1C_global_settings"] = applyAllBases;

    /** daca nu e indicat -> nou */
    if (m_configFile.isEmpty()) {

        const QString baseDir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + "/plugins/" + m_pluginId;

        QDir dir;
        dir.mkpath(baseDir);

        if (m_pluginId == "mssql") {
            const QString dbName = obj.value("database").toString().trimmed();
            m_configFile = dir.toNativeSeparators(baseDir + "/" + dbName + ".json");

        } else if (m_pluginId == "export_1c") {
            if (applyAllBases) {
                m_configFile = dir.toNativeSeparators(baseDir + "/global_all_bases.json");
            } else {
                const QString dbName = export1CDbName(values);
                m_configFile = dir.toNativeSeparators(baseDir + "/" + dbName + ".json");
            }
        }
    }

    QFile f(m_configFile);
    if (!f.open(QIODevice::WriteOnly))
        return;

    f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));

    /** !!! dupa ce au fost salvate datele e necesar de emis signal
     *  cu transmiterea datelor in tabela */
    QVariantMap dbInfo;
    dbInfo["typeDB"]     = obj.value("typeDB").toString();
    dbInfo["database"]   = obj.value("database").toString();
    dbInfo["server"]     = obj.value("server").toString();
    dbInfo["config"]     = m_configFile;
    dbInfo["configured"] = obj.value("configured").toBool();
    dbInfo["export1C_global_settings"] = obj["export1C_global_settings"].toBool();
    emit onAddedDatabase(dbInfo);

}

QString PluginConfigDialog::getPlatform1CPath() const
{
#ifdef Q_OS_WIN
    // PATH
    QString p = QStandardPaths::findExecutable("1cv8.exe");
    if (!p.isEmpty())
        return QDir::toNativeSeparators(p);

    // Directoare standard
    QStringList baseDirs = {
        qEnvironmentVariable("ProgramFiles(x86)"),
        qEnvironmentVariable("ProgramFiles")
    };

    QString bestPath;
    QVersionNumber bestVersion;

    for (const QString &base : baseDirs) {
        if (base.isEmpty())
            continue;

        QDir root(base + "/1cv8");
        if (!root.exists())
            continue;

        for (const QString &verDir :
             root.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {

            QVersionNumber ver = QVersionNumber::fromString(verDir);
            if (ver.isNull())
                continue;

            QString exe = root.filePath(verDir + "/bin/1cv8.exe");
            if (!QFile::exists(exe))
                continue;

            if (ver > bestVersion) {
                bestVersion = ver;
                bestPath = exe;
            }
        }
    }

    if (!bestPath.isEmpty())
        return QDir::toNativeSeparators(bestPath);
#endif

    return QString(); // nu a fost găsit
}

QString PluginConfigDialog::export1CDbName(const QVariantMap &values)
{
    /** "database" are prioritate, altfel numele folderului din "dbPath" (baze de tip File) */
    QString name = values.value("database").toString().trimmed();
    if (name.isEmpty()) {
        const QString path = values.value("dbPath").toString().trimmed();
        if (!path.isEmpty())
            name = QFileInfo(QDir::cleanPath(path)).fileName();
    }
    return name;
}

QString PluginConfigDialog::schemaPath() const
{
    if (m_pluginId == "mssql")
        return QString(":/plugins/%1/config_mssql.json").arg(m_pluginId);
    else if (m_pluginId == "export_1c")
        return QString(":/plugins/%1/config_1c.json").arg(m_pluginId);
    else
        return QString();
}
