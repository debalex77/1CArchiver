#include "scheduledtaskdialog.h"

#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimeEdit>
#include <QPushButton>
#include <QProcess>
#include <QCoreApplication>

#include <windows.h>

#include "src/switchbutton.h"

static bool runSchtasksElevated(const QString &arguments)
{
    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"runas";              // solicită UAC
    sei.lpFile = L"schtasks.exe";
    sei.lpParameters = (LPCWSTR)arguments.utf16();
    sei.nShow = SW_HIDE;

    return ShellExecuteExW(&sei);
}

ScheduledTaskDialog::ScheduledTaskDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Programare backup"));
    setModal(true);
    resize(360, 280);

    auto *mainLayout = new QVBoxLayout(this);

    // --- Titlu
    auto *lblTitle = new QLabel(
        tr("Selectează zilele și ora pentru backup automat:"), this);
    lblTitle->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(lblTitle);

    // --- Status
    m_lblStatus = new QLabel(this);
    m_lblStatus->setAlignment(Qt::AlignCenter);
    m_lblStatus->setStyleSheet("font-weight: bold; padding: 6px;");
    mainLayout->addWidget(m_lblStatus);

    // --- Zilele săptămânii
    auto *daysLayout = new QGridLayout;

    const QStringList daysText = {
        tr("Luni"), tr("Marți"), tr("Miercuri"),
        tr("Joi"), tr("Vineri"), tr("Sâmbătă"), tr("Duminică")
    };

    const QStringList dayArgs = {
        "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"
    };

    for (int i = 0; i < daysText.size(); ++i) {
        auto *lbl = new QLabel(daysText[i], this);
        auto *sw  = new SwitchButton(this);
        sw->setProperty("dayArg", dayArgs[i]);

        auto *row = new QHBoxLayout;
        row->addStretch();
        row->addWidget(lbl);
        row->addWidget(sw);

        daysLayout->addLayout(row, i, 0);
        m_daySwitches.append(sw);
    }

    mainLayout->addLayout(daysLayout);

    // --- Ora
    auto *timeLayout = new QHBoxLayout;
    auto *lblTime = new QLabel(tr("Ora pornirii:"), this);

    m_timeEdit = new QTimeEdit(QTime(22, 0), this);
    m_timeEdit->setDisplayFormat("HH:mm");

    timeLayout->addWidget(lblTime);
    timeLayout->addWidget(m_timeEdit);
    timeLayout->addStretch();

    mainLayout->addLayout(timeLayout);

    // --- Butoane
    auto *btnLayout = new QHBoxLayout;

    m_btnCreate = new QPushButton(tr("Creează task"), this);
    m_btnCancel = new QPushButton(tr("Anulează"), this);

    m_btnEnable  = new QPushButton(tr("Activează"), this);
    m_btnDisable = new QPushButton(tr("Dezactivează"), this);
    m_btnDelete  = new QPushButton(tr("Șterge task"), this);

    btnLayout->addStretch();
    btnLayout->addWidget(m_btnCreate);
    btnLayout->addWidget(m_btnCancel);

    btnLayout->insertWidget(0, m_btnEnable);
    btnLayout->insertWidget(1, m_btnDisable);
    btnLayout->insertWidget(2, m_btnDelete);

    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    // --- Conexiuni
    connect(m_btnCreate, &QPushButton::clicked,
            this, &ScheduledTaskDialog::onCreateTask);
    connect(m_btnCancel, &QPushButton::clicked,
            this, &ScheduledTaskDialog::onCancel);

    connect(m_btnEnable,  &QPushButton::clicked,
            this, &ScheduledTaskDialog::enableTask);
    connect(m_btnDisable, &QPushButton::clicked,
            this, &ScheduledTaskDialog::disableTask);

    connect(m_btnDelete, &QPushButton::clicked,
            this, &ScheduledTaskDialog::deleteTaskUi);

    // --- Determinarea existentei task-lui
    detectExistingTask();
}

void ScheduledTaskDialog::onCreateTask()
{
    const QString days = buildDaysArgument();
    if (days.isEmpty()) {
        QMessageBox::warning(this,
                             tr("Eroare"),
                             tr("Selectează cel puțin o zi."));
        return;
    }

    if (taskExists()) {
        const auto ret = QMessageBox::question(
            this,
            tr("Task existent"),
            tr("Un task de backup există deja.\n"
               "Dorești să îl recreezi cu noile setări?"),
            QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::No)
            return;

        if (!deleteTaskElevated()) {
            QMessageBox::critical(this,
                                  tr("Eroare"),
                                  tr("Nu a fost posibilă ștergerea task-ului existent."));
            return;
        }
    }

    const QString time     = m_timeEdit->time().toString("HH:mm");
    const QString taskName = "1CArchiver Backup";
    const QString appPath  = applicationPath();

    const QString args =
        QString(
            "/create "
            "/tn \"%1\" "
            "/sc weekly "
            "/d %2 "
            "/st %3 "
            "/tr \"\\\"%4\\\" --autorun\" "
            "/rl HIGHEST "
            "/f"
            ).arg(taskName, days, time, appPath);

    if (!createTaskElevated(args)) {
        QMessageBox::critical(this,
                              tr("Eroare"),
                              tr("Nu a fost posibilă crearea task-ului.\n"
                                 "Confirmă solicitarea UAC."));
        return;
    }

    QMessageBox::information(this,
                             tr("Succes"),
                             tr("Task-ul a fost creat cu succes."));
    accept();
}

void ScheduledTaskDialog::onCancel()
{
    reject();
}

void ScheduledTaskDialog::enableTask()
{
    runSchtasksElevated("/change /tn \"1CArchiver Backup\" /enable");
    m_taskEnabled = true;
    updateStatusUi();
}

void ScheduledTaskDialog::disableTask()
{
    runSchtasksElevated("/change /tn \"1CArchiver Backup\" /disable");
    m_taskEnabled = false;
    updateStatusUi();
}

void ScheduledTaskDialog::deleteTaskUi()
{
    if (QMessageBox::question(
            this,
            tr("Confirmare"),
            tr("Sigur dorești să ștergi task-ul programat?"))
        != QMessageBox::Yes)
        return;

    runSchtasksElevated("/delete /tn \"1CArchiver Backup\" /f");

    m_taskExists = false;
    m_taskEnabled = false;
    updateStatusUi();
}

void ScheduledTaskDialog::detectExistingTask()
{
    // --- Reset implicit
    m_taskExists  = false;
    m_taskEnabled = false;

    QProcess proc;
    proc.start("schtasks",
               {
                   "/query",
                   "/tn", "1CArchiver Backup",
                   "/xml"
               });

    if (!proc.waitForFinished(5000) || proc.exitCode() != 0) {
        // Task inexistent
        updateUiNoTask();
        updateStatusUi();
        return;
    }

    // Task există
    m_taskExists = true;

    const QString xml = QString::fromUtf8(proc.readAllStandardOutput());

    // 1. Activ / Dezactivat
    // -------------------------------------------------
    m_taskEnabled = !xml.contains("<Enabled>false</Enabled>",
                                  Qt::CaseInsensitive);

    // 2. Ora pornirii
    // <StartBoundary>2025-12-14T22:00:00</StartBoundary>
    // -------------------------------------------------
    {
        QRegularExpression rxTime(
            "<StartBoundary>[^T]*T(\\d{2}:\\d{2})");
        const auto match = rxTime.match(xml);
        if (match.hasMatch()) {
            const QTime t =
                QTime::fromString(match.captured(1), "HH:mm");
            if (t.isValid())
                m_timeEdit->setTime(t);
        }
    }

    // 3. Zilele săptămânii
    // -------------------------------------------------
    for (auto *sw : std::as_const(m_daySwitches)) {
        const QString dayArg =
            sw->property("dayArg").toString();

        const bool checked =
            xml.contains(dayArg, Qt::CaseInsensitive);

        sw->setChecked(checked);
    }

    // 4. Actualizare UI finală
    // -------------------------------------------------
    updateStatusUi();
}

void ScheduledTaskDialog::applyTaskSettingsFromXml(const QString &xml)
{
    // Ora
    QRegularExpression rxTime("<StartBoundary>.*T(\\d{2}:\\d{2})");
    auto m = rxTime.match(xml);
    if (m.hasMatch())
        m_timeEdit->setTime(QTime::fromString(m.captured(1), "HH:mm"));

    // Zilele
    for (auto *sw : std::as_const(m_daySwitches)) {
        const QString day = sw->property("dayArg").toString();
        sw->setChecked(xml.contains(day, Qt::CaseInsensitive));
    }

    // Status
    setWindowTitle(tr("Programare backup (task existent)"));
}

void ScheduledTaskDialog::updateUiNoTask()
{
    for (auto *sw : std::as_const(m_daySwitches))
        sw->setChecked(false);

    m_timeEdit->setTime(QTime(22, 0));
    setWindowTitle(tr("Programare backup (task inexistent)"));
}

void ScheduledTaskDialog::updateStatusUi()
{
    if (!m_taskExists) {
        m_lblStatus->setText(tr("Task inexistent"));
        m_lblStatus->setStyleSheet(
            "background:#d9534f;color:white;font-weight:bold;padding:6px;");
        m_btnEnable->setEnabled(false);
        m_btnDisable->setEnabled(false);
        m_btnDelete->setEnabled(false);
        return;
    }

    if (m_taskEnabled) {
        m_lblStatus->setText(tr("Task activ"));
        m_lblStatus->setStyleSheet(
            "background:#5cb85c;color:white;font-weight:bold;padding:6px;");
        m_btnEnable->setEnabled(false);
        m_btnDisable->setEnabled(true);
    } else {
        m_lblStatus->setText(tr("Task dezactivat"));
        m_lblStatus->setStyleSheet(
            "background:#f0ad4e;color:black;font-weight:bold;padding:6px;");
        m_btnEnable->setEnabled(true);
        m_btnDisable->setEnabled(false);
    }

    m_btnDelete->setEnabled(true);
}

QString ScheduledTaskDialog::buildDaysArgument() const
{
    QStringList selectedDays;
    for (auto *sw : m_daySwitches) {
        if (sw->isChecked())
            selectedDays << sw->property("dayArg").toString();
    }
    return selectedDays.join(",");
}

QString ScheduledTaskDialog::applicationPath() const
{
    return QCoreApplication::applicationFilePath()
    .replace('/', '\\');
}

bool ScheduledTaskDialog::taskExists() const
{
    QProcess proc;
    proc.start("cmd.exe",
               { "/c", "schtasks /query /tn \"1CArchiver Backup\"" });
    proc.waitForFinished();
    return proc.exitCode() == 0;
}

bool ScheduledTaskDialog::deleteTaskElevated() const
{
    return runSchtasksElevated(
        "/delete /tn \"1CArchiver Backup\" /f");
}

bool ScheduledTaskDialog::createTaskElevated(const QString &args) const
{
    return runSchtasksElevated(args);
}
