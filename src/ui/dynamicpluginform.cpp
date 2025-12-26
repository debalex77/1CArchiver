#include "dynamicpluginform.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QLineEdit>

#include <src/lineeditpassword.h>

DynamicPluginForm::DynamicPluginForm(const QJsonObject &schema, QWidget *parent)
    : QWidget(parent),
      m_schema(schema)
{
    buildForm();
    updateVisibility();
}

bool DynamicPluginForm::validate(QString *errorMessage) const
{
    for (auto it = m_fieldDefs.begin(); it != m_fieldDefs.end(); ++it)
    {
        QJsonObject def = it.value();
        QString id = it.key();

        if (!def.value("required").toBool())
            continue;

        QWidget *w = m_fields.value(id);
        if (!w->isVisible())
            continue;

        QVariant val = fieldValue(id);
        if (val.toString().trimmed().isEmpty())
        {
            if (errorMessage)
                *errorMessage = tr("Field \"%1\" is required.")
                                    .arg(def.value("label").toString());
            return false;
        }
    }
    return true;
}

QVariantMap DynamicPluginForm::values() const
{
    QVariantMap map;
    for (auto it = m_fields.begin(); it != m_fields.end(); ++it)
        map[it.key()] = fieldValue(it.key());

    return map;
}

void DynamicPluginForm::updateVisibility()
{
    for (auto it = m_fieldDefs.begin(); it != m_fieldDefs.end(); ++it) {
        QString id = it.key();
        QJsonObject def = it.value();

        if (!def.contains("visible_if")) {
            m_fields[id]->setVisible(true);
            continue;
        }

        QJsonObject cond = def.value("visible_if").toObject();
        bool visible = true;

        for (auto c = cond.begin(); c != cond.end(); ++c) {
            QVariant actual = fieldValue(c.key());
            if (actual.toString() != c.value().toString()) {
                visible = false;
                break;
            }
        }

        m_fields[id]->setVisible(visible);
    }
}

void DynamicPluginForm::buildForm()
{
    auto *layout = new QFormLayout;
    setLayout(layout);

    QJsonArray fields =
        m_schema.value("ui").toObject()
            .value("fields").toArray();

    for (const QJsonValue &v : std::as_const(fields)) {
        QJsonObject def = v.toObject();
        QString id      = def.value("id").toString();
        QString label   = def.value("label").toString();

        QWidget *w = createField(def);
        if (!w)
            continue;

        m_fields[id]    = w;
        m_fieldDefs[id] = def;

        layout->addRow(label, w);

        if (qobject_cast<QComboBox*>(w)) {
            connect(w, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(updateVisibility()));
        }
        if (qobject_cast<QCheckBox*>(w)) {
            connect(w, SIGNAL(toggled(bool)),
                    this, SLOT(updateVisibility()));
        }
    }
}

QWidget *DynamicPluginForm::createField(const QJsonObject &field)
{
    QString type = field.value("type").toString();

    if (type == "string") {
        auto *le = new QLineEdit(this);

        if (field.contains("placeholder"))
            le->setPlaceholderText(field.value("placeholder").toString());

        if (field.contains("default"))
            le->setText(field.value("default").toString());

        return le;
    }

    if (type == "password") {
        auto *le = new LineEditPassword(this);

        if (field.contains("placeholder"))
            le->setPlaceholderText(field.value("placeholder").toString());

        return le;
    }

    if (type == "enum") {
        auto *cb = new QComboBox(this);
        QJsonArray values = field.value("values").toArray();
        for (const QJsonValue &v : std::as_const(values))
            cb->addItem(v.toString());

        if (field.contains("default"))
            cb->setCurrentText(field.value("default").toString());

        return cb;
    }

    if (type == "bool") {
        auto *chk = new QCheckBox(this);
        return chk;
    }

    return nullptr;
}

QVariant DynamicPluginForm::fieldValue(const QString &id) const
{
    QWidget *w = m_fields.value(id, nullptr);
    if (!w)
        return {};

    if (auto *le = qobject_cast<QLineEdit*>(w))
        return le->text();

    if (auto *cb = qobject_cast<QComboBox*>(w))
        return cb->currentText();

    if (auto *chk = qobject_cast<QCheckBox*>(w))
        return chk->isChecked();

    return {};
}
