#ifndef SCHEDULEDTASKDIALOG_H
#define SCHEDULEDTASKDIALOG_H

#include <QDialog>
#include <QList>

class QLabel;
class QPushButton;
class QTimeEdit;
class SwitchButton;

class ScheduledTaskDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ScheduledTaskDialog(QWidget *parent = nullptr);

private slots:
    void onCreateTask();
    void onCancel();

    void enableTask();
    void disableTask();
    void deleteTaskUi();

private:
    void detectExistingTask();
    void applyTaskSettingsFromXml(const QString &xml);
    void updateUiNoTask();
    void updateStatusUi();

    QString buildDaysArgument() const;
    QString applicationPath() const;
    bool taskExists() const;
    bool deleteTaskElevated() const;
    bool createTaskElevated(const QString &args) const;

private:
    QList<SwitchButton*> m_daySwitches;
    QTimeEdit   *m_timeEdit;
    QPushButton *m_btnCreate;
    QPushButton *m_btnCancel;

    QLabel *m_lblStatus = nullptr;

    QPushButton *m_btnEnable  = nullptr;
    QPushButton *m_btnDisable = nullptr;
    QPushButton *m_btnDelete  = nullptr;
    bool m_taskExists  = false;
    bool m_taskEnabled = false;
};

#endif // SCHEDULEDTASKDIALOG_H
