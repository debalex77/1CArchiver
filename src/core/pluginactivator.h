#ifndef PLUGINACTIVATOR_H
#define PLUGINACTIVATOR_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

#include <src/globals.h>
#include <src/switchbutton.h>

class PluginActivator : public QDialog
{
    Q_OBJECT
public:
    explicit PluginActivator(QWidget* parent = nullptr);
    ~PluginActivator();

    void setupUI();  /** construim forma */
    void updateUI(); /** actualizam forma */

signals:
    void addedDatabaseMSSQL(const QVariantMap &dbInfo);

private slots:
    void onClickMSSQL(bool on);
    void onClickRsync(bool on);
    void onClickOneDrive(bool on);

    void onClickConfigMSSQL();

private:
    QLabel *lbl_mssql; /** titlu principal */
    QLabel *lbl_rsync;
    QLabel *lbl_onedrive;

    QLabel *desc_mssql; /** descrierea */
    QLabel *desc_rsync;
    QLabel *desc_onedrive;

    QLabel *status_mssql; /** ststus plugin-lui */
    QLabel *status_rsync;
    QLabel *status_onedrive;

    SwitchButton *btnMSSQL    = nullptr; /** switch button pu activarea plugin */
    SwitchButton *btnRsync    = nullptr;
    SwitchButton *btnOneDrive = nullptr;

    QPushButton *btnConfigMSSQL    = nullptr; /** butoane pu adaugarea BD */
    QPushButton *btnConfigRsync    = nullptr;
    QPushButton *btnConfigOneDrive = nullptr;

    //-----------------------------------------
    void checkPluginMSSQL();
    void checkPluginRsync();
    void checkPluginOneDrive();
};

#endif // PLUGINACTIVATOR_H
