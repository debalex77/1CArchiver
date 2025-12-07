#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "switchbutton.h"
#include "globals.h"
#include "lineeditpassword.h"

#include <QEvent>
#include <QDialog>
#include <QLabel>
#include <QObject>

class AppSettings : public QDialog
{
    Q_OBJECT
public:
    explicit AppSettings(QWidget* parent = nullptr);
    ~AppSettings();

    void setChecked(bool on);
    bool isChecked() const;

private:
    QLabel* lbl_setArchivePassword;
    QLabel* lbl_backupExtFiles;
    QLabel* lbl_fileSHA256;
    QLabel* lbl_closeApp;
    QLabel* lbl_pwd;
    SwitchButton* btn_setArchivePassword;
    SwitchButton* btn_backupExtFiles;
    SwitchButton* btn_createFileSHA256;
    SwitchButton* btn_closeApp;

    LineEditPassword* edit_pwd;

    void setupUI();
    void updateUI();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // APPSETTINGS_H
