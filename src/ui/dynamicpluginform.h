/*****************************************************************************
 * 1CArchiver is a Qt/C++ application designed for fast, reliable,
 * and automated backup of 1C:Enterprise file-based databases.
 * Copyright (c) 2024-2026 Codreanu Alexandru - All Rights Reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *****************************************************************************/

#ifndef DYNAMICPLUGINFORM_H
#define DYNAMICPLUGINFORM_H

#include <QWidget>
#include <QJsonObject>
#include <QMap>
#include <QVariantMap>

/*
 * DynamicPluginForm
 * -----------------
 * Widget UI generic construit din .json:
 *  - construieste elemente UI
 *  - gestioneaza required/visible_if
 *  - colecteaza valorile într-un QVariantMap
 */
class DynamicPluginForm : public QWidget
{
    Q_OBJECT
public:
    explicit DynamicPluginForm(const QJsonObject &schema,
                               QWidget *parent = nullptr);

    bool validate(QString *errorMessage = nullptr) const; /** verificam daca tot e completat */
    QVariantMap values() const;                           /** valorile introduse de utilizator */
    void setValues(const QVariantMap &values);            /** setam valorile din QVariantMap */

private slots:
    void updateVisibility(); /** actualizam vizibilitatea campurilor (visible_if) */

private:
    void buildForm();
    QWidget *createField(const QJsonObject &field);

    QVariant fieldValue(const QString &id) const;

    QJsonObject m_schema;


    QMap<QString, QWidget*>    m_fields;    /** id -> widget */
    QMap<QString, QJsonObject> m_fieldDefs; /** id -> descriere .json */
};

#endif // DYNAMICPLUGINFORM_H
