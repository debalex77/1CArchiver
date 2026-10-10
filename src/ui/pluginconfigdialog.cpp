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

    m_form = new DynamicPluginForm(m_schema, this);
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
    if (!m_form->validate(&error))
    {
        QMessageBox::warning(this, tr("Invalid configuration"), error);
        return;
    }

    if (!saveConfig())
        return; /** ramanem in dialog */

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

bool PluginConfigDialog::saveConfig()
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

    /** fisierul vechi -> se elimina daca la editare s-a schimbat server/baza */
    QString oldConfigFile;

    if (m_pluginId == "mssql") {

        const QString baseDir =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + "/plugins/" + m_pluginId;

        QDir().mkpath(baseDir);

        /** server + baza: aceeasi BD pe servere diferite -> fisiere diferite */
        const QString server = obj.value("server").toString().trimmed();
        const QString dbName = obj.value("database").toString().trimmed();
        const QString target = QDir::toNativeSeparators(
            baseDir + "/" + safeFileName(server + "_" + dbName) + ".json");

        if (m_configFile.isEmpty()) {
            /** nou */
            m_configFile = target;
        } else if (QDir::toNativeSeparators(m_configFile)
                       .compare(target, Qt::CaseInsensitive) != 0) {
            /** editare cu alt server/baza -> nu suprascriem alta configurare */
            if (QFile::exists(target)) {
                QMessageBox::warning(this, tr("Invalid configuration"),
                                     tr("Există deja o configurare pentru serverul '%1' și baza '%2'.")
                                         .arg(server, dbName));
                return false;
            }
            oldConfigFile = m_configFile;
            m_configFile  = target;
        }
    }

    QDir().mkpath(QFileInfo(m_configFile).absolutePath());

    QFile f(m_configFile);
    if (!f.open(QIODevice::WriteOnly)
        || f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented)) < 0) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Nu pot salva configurarea: %1")
                                  .arg(QDir::toNativeSeparators(m_configFile)));
        if (!oldConfigFile.isEmpty())
            m_configFile = oldConfigFile;
        return false;
    }
    f.close();

    if (!oldConfigFile.isEmpty())
        QFile::remove(oldConfigFile);

    /** !!! dupa ce au fost salvate datele e necesar de emis signal
     *  cu transmiterea datelor in tabela */
    QVariantMap dbInfo;
    dbInfo["typeDB"]     = obj.value("typeDB").toString();
    dbInfo["database"]   = obj.value("database").toString();
    dbInfo["server"]     = obj.value("server").toString();
    dbInfo["config"]     = m_configFile;
    dbInfo["configured"] = obj.value("configured").toBool();
    emit onAddedDatabase(dbInfo);

    return true;
}

QString PluginConfigDialog::schemaPath() const
{
    return QString(":/plugins/%1/config_%1.json").arg(m_pluginId);
}

void PluginConfigDialog::setDefaults(const QVariantMap &values)
{
    /** doar pentru configurare noua - nu suprascriem valorile salvate */
    if (m_config.isEmpty() && m_form)
        m_form->setValues(values);
}
