#include "pluginconfigdialog.h"
#include "dynamicpluginform.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDir>
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

    // Preumple câmpurile din config existent
    for (auto it = m_config.begin(); it != m_config.end(); ++it)
        m_form->findChild<QWidget*>(it.key());

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
    for (auto it = values.begin(); it != values.end(); ++it)
        obj[it.key()] = QJsonValue::fromVariant(it.value());

    obj["configured"] = true;

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
        }
    }

    QFile f(m_configFile);
    if (!f.open(QIODevice::WriteOnly))
        return;

    f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));

    /** !!! dupa ce au fost salvate datele e necesar de emis signal cu transmiterea datelor
      in tabela */
    QVariantMap dbInfo;
    dbInfo["typeDB"]     = obj.value("typeDB").toString();
    dbInfo["database"]   = obj.value("database").toString();
    dbInfo["server"]     = obj.value("server").toString();
    dbInfo["config"]     = m_configFile;
    dbInfo["configured"] = obj.value("configured").toBool();
    emit onAddedDatabase(dbInfo);

}

QString PluginConfigDialog::schemaPath() const
{
    return QString(":/plugins/%1/config_mssql.json").arg(m_pluginId);
}
