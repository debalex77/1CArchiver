#include "dynamicpluginform.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QFormLayout>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include <src/lineeditpassword.h>
#include <src/utils.h>

DynamicPluginForm::DynamicPluginForm(const QJsonObject &schema, QWidget *parent)
    : QWidget(parent),
      m_schema(schema)
{
    buildForm();
    updateVisibility(); /** functioneaza si inainte de show() -> se bazeaza pe isHidden() */
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
        if (!w || w->isHidden())
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

void DynamicPluginForm::setValues(const QVariantMap &values)
{
    for (auto it = values.begin(); it != values.end(); ++it) {

        const QString &key  = it.key();
        const QVariant &val = it.value();

        QWidget *w = m_fields.value(key, nullptr);
        if (!w)
            continue;

        if (auto *le = qobject_cast<QLineEdit*>(w)) {
            if (key == "password") {
                le->setText(decryptPassword(val.toString()));
            } else {
                le->setText(val.toString());
            }
        }
        else if (auto *cb = qobject_cast<QComboBox*>(w)) {
            cb->setCurrentText(val.toString());
        }
        else if (auto *chk = qobject_cast<QCheckBox*>(w)) {
            chk->setChecked(val.toBool());
        }
    }
    updateVisibility();
}

void DynamicPluginForm::setValue(const QString &key, const QVariant &value)
{
    if (!m_fields.contains(key))
        return;

    QWidget *w = m_fields.value(key);
    if (!w)
        return;

    // QLineEdit / Password
    if (auto *le = qobject_cast<QLineEdit*>(w)) {
        le->setText(value.toString());
        return;
    }

    // QComboBox (enum)
    if (auto *cb = qobject_cast<QComboBox*>(w)) {
        int idx = cb->findText(value.toString());
        if (idx >= 0)
            cb->setCurrentIndex(idx);
        return;
    }

    // QCheckBox
    if (auto *ch = qobject_cast<QCheckBox*>(w)) {
        ch->setChecked(value.toBool());
        return;
    }

    // QSpinBox
    if (auto *sb = qobject_cast<QSpinBox*>(w)) {
        sb->setValue(value.toInt());
        return;
    }

    // QDoubleSpinBox
    if (auto *dsb = qobject_cast<QDoubleSpinBox*>(w)) {
        dsb->setValue(value.toDouble());
        return;
    }

    // QDateEdit
    if (auto *de = qobject_cast<QDateEdit*>(w)) {
        if (value.canConvert<QDate>())
            de->setDate(value.toDate());
        return;
    }

    // QTimeEdit
    if (auto *te = qobject_cast<QTimeEdit*>(w)) {
        if (value.canConvert<QTime>())
            te->setTime(value.toTime());
        return;
    }

    // QDateTimeEdit
    if (auto *dte = qobject_cast<QDateTimeEdit*>(w)) {
        if (value.canConvert<QDateTime>())
            dte->setDateTime(value.toDateTime());
        return;
    }

    // fallback – nimic de setat
}

void DynamicPluginForm::updateVisibility()
{
    auto *form = qobject_cast<QFormLayout*>(layout());
    if (!form)
        return;

    bool anyChanged = false;

    for (auto it = m_fieldDefs.begin(); it != m_fieldDefs.end(); ++it) {
        const QString &id = it.key();
        const QJsonObject &def = it.value();

        bool visible = true;

        if (def.contains("visible_if")) {
            QJsonObject cond = def["visible_if"].toObject();

            for (auto c = cond.begin(); c != cond.end(); ++c) {
                QVariant actual   = fieldValue(c.key());
                QVariant expected = c.value().toVariant();

                /** valoare inexistenta -> nu blocheaza */
                if (!actual.isValid())
                    continue;

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
                const int expType = expected.typeId();
#else
                const int expType = expected.type();
#endif
                if (expType == QMetaType::Bool) {
                    if (actual.toBool() != expected.toBool()) {
                        visible = false;
                        break;
                    }
                } else {
                    if (actual.toString() != expected.toString()) {
                        visible = false;
                        break;
                    }
                }
            }
        }

        QWidget *editor = m_fields.value(id);
        if (!editor)
            continue;

        /** isHidden() reflecta starea explicita a widget-ului,
         *  isVisible() ar fi false cat timp dialogul nu e afisat */
        if (editor->isHidden() == visible) {
            editor->setVisible(visible);

            QWidget *labelWidget = form->labelForField(editor);
            if (labelWidget)
                labelWidget->setVisible(visible);

            anyChanged = true;
        }
    }

    if (anyChanged) {
        layout()->invalidate();
        layout()->activate();
        emit sizeChanged();   /** QDialog va face adjustSize() */
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

    if (type == "bool" || type == "checkbox") {
        auto *chk = new QCheckBox(this);
        chk->setChecked(field.value("default").toBool(false));
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
