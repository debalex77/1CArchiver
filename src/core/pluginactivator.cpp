#include "pluginactivator.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <QVBoxLayout>
#include <QFile>

#include <src/ui/pluginconfigdialog.h>
#pragma comment(lib, "dwmapi.lib")
static void enableDarkTitlebar(QWidget* w) {
    HWND hwnd = (HWND)w->winId();
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));   // Dark TitleBar
}
#endif

PluginActivator::PluginActivator(QWidget *parent)
    : QDialog(parent)
{
    /** activarea titleBar Dark */
    if (globals::isDark)
        enableDarkTitlebar(this);

    setupUI();  /** construim forma */
    updateUI(); /** actualizam forma */

    /** connections */
    connect(btnMSSQL, &SwitchButton::toggled, this, &PluginActivator::onClickMSSQL);
    connect(btnRsync, &SwitchButton::toggled, this, &PluginActivator::onClickRsync);
    connect(btnOneDrive, &SwitchButton::toggled, this, &PluginActivator::onClickOneDrive);

    connect(btnConfigMSSQL, &QPushButton::clicked, this, &PluginActivator::onClickConfigMSSQL);
}

PluginActivator::~PluginActivator()
{

}

void PluginActivator::setupUI()
{
    setWindowTitle(tr("Pluginuri opționale"));
    setWindowIcon(QIcon(":/icons/icons/plugin.png"));

    QVBoxLayout *v = new QVBoxLayout(this);

    //-------------------------------------------------
    // --- MSSQL
    //-------------------------------------------------

    auto *layout_mssql = new QHBoxLayout;
    layout_mssql->setContentsMargins(10,10,10,2);
    layout_mssql->setSpacing(10);

    lbl_mssql = new QLabel(this);
    lbl_mssql->setStyleSheet("font-size: 14px; font-weight: bold;");
    lbl_mssql->setText(tr("Plugin MSSQL"));

    btnMSSQL = new SwitchButton(this);
    btnMSSQL->setChecked(globals::pl_mssql);

    desc_mssql = new QLabel(this);
    desc_mssql->setStyleSheet("font-size: 11px;");
    desc_mssql->setText(tr("Activarea pluginului pentru baze de date MSSQL"));

    btnConfigMSSQL = new QPushButton(this);
    btnConfigMSSQL->setText(tr("Add database"));

    status_mssql = new QLabel(this);
    status_mssql->setStyleSheet("font-size: 11px; font-style: italic; color: #7acfcf;");
    checkPluginMSSQL();

    layout_mssql->addWidget(btnMSSQL);
    layout_mssql->addWidget(desc_mssql);
    layout_mssql->addStretch();
    layout_mssql->addWidget(btnConfigMSSQL);

    QFrame* line_mssql = new QFrame(this);
    line_mssql->setFrameShape(QFrame::HLine);
    line_mssql->setFrameShadow(QFrame::Plain);
    line_mssql->setFixedHeight(1);

    v->addWidget(lbl_mssql);
    v->addLayout(layout_mssql);
    v->addWidget(status_mssql);
    v->addWidget(line_mssql);

    //-------------------------------------------------
    // --- RSYNC
    //-------------------------------------------------
    auto *layout_rsync = new QHBoxLayout;
    layout_rsync->setContentsMargins(10,10,10,2);
    layout_rsync->setSpacing(10);

    lbl_rsync = new QLabel(this);
    lbl_rsync->setStyleSheet("font-size: 14px; font-weight: bold;");
    lbl_rsync->setText(tr("Plugin RSYNC"));

    btnRsync = new SwitchButton(this);
    btnRsync->setChecked(globals::pl_rsync);

    desc_rsync = new QLabel(this);
    desc_rsync->setStyleSheet("font-size: 11px;");
    desc_rsync->setText(tr("Activarea pluginului pentru sincronizarea arhivelor<br> "
                           "cu ajutorul RSYNC"));

    btnConfigRsync = new QPushButton(this);
    btnConfigRsync->setText(tr("Configurarea"));

    status_rsync = new QLabel(this);
    status_rsync->setStyleSheet("font-size: 11px; font-style: italic; color: #7acfcf;");
    checkPluginRsync();

    layout_rsync->addWidget(btnRsync);
    layout_rsync->addWidget(desc_rsync);
    layout_rsync->addStretch();
    layout_rsync->addWidget(btnConfigRsync);

    QFrame* line_rsync = new QFrame(this);
    line_rsync->setFrameShape(QFrame::HLine);
    line_rsync->setFrameShadow(QFrame::Plain);
    line_rsync->setFixedHeight(1);

    v->addWidget(lbl_rsync);
    v->addLayout(layout_rsync);
    v->addWidget(status_rsync);
    v->addWidget(line_rsync);

    //-------------------------------------------------
    // --- ONEDRIVE
    //-------------------------------------------------
    auto *layout_onedrive = new QHBoxLayout;
    layout_onedrive->setContentsMargins(10,10,10,2);
    layout_onedrive->setSpacing(10);

    lbl_onedrive = new QLabel(this);
    lbl_onedrive->setStyleSheet("font-size: 14px; font-weight: bold;");
    lbl_onedrive->setText(tr("Plugin OneDrive"));

    btnOneDrive = new SwitchButton(this);
    btnOneDrive->setChecked(globals::pl_onedrive);

    desc_onedrive = new QLabel(this);
    desc_onedrive->setStyleSheet("font-size: 11px;");
    desc_onedrive->setText(tr("Activarea pluginului pentru sincronizarea arhivelor<br> "
                              "cu ajutorul OneDrive"));

    btnConfigOneDrive = new QPushButton(this);
    btnConfigOneDrive->setText(tr("Configurarea"));

    status_onedrive = new QLabel(this);
    status_onedrive->setStyleSheet("font-size: 11px; font-style: italic; color: #7acfcf;");
    checkPluginOneDrive();

    layout_onedrive->addWidget(btnOneDrive);
    layout_onedrive->addWidget(desc_onedrive);
    layout_onedrive->addStretch();
    layout_onedrive->addWidget(btnConfigOneDrive);

    QFrame* line_onedrive = new QFrame(this);
    line_onedrive->setFrameShape(QFrame::HLine);
    line_onedrive->setFrameShadow(QFrame::Plain);
    line_onedrive->setFixedHeight(1);

    v->addWidget(lbl_onedrive);
    v->addLayout(layout_onedrive);
    v->addWidget(status_onedrive);
    v->addWidget(line_onedrive);
    v->addStretch();
}

void PluginActivator::updateUI()
{
    status_mssql->setVisible(btnMSSQL->isChecked());
    status_rsync->setVisible(btnRsync->isChecked());
    status_onedrive->setVisible(btnOneDrive->isChecked());

    btnConfigMSSQL->setEnabled(btnMSSQL->isChecked());
    btnConfigRsync->setEnabled(btnRsync->isChecked());
    btnConfigOneDrive->setEnabled(btnOneDrive->isChecked());

    this->adjustSize();
}

void PluginActivator::onClickMSSQL(bool on)
{
    globals::pl_mssql = on;
    if (on)
        checkPluginMSSQL();
    updateUI();
}

void PluginActivator::onClickRsync(bool on)
{
    globals::pl_rsync = on;
    if (on)
        checkPluginRsync();
    updateUI();
}

void PluginActivator::onClickOneDrive(bool on)
{
    globals::pl_onedrive = on;
    if (on)
        checkPluginOneDrive();
    updateUI();
}

void PluginActivator::onClickConfigMSSQL()
{
    PluginConfigDialog *config_dlg_mssql
        = new PluginConfigDialog("mssql",
                                 QString(),
                                 this);
    connect(config_dlg_mssql, &PluginConfigDialog::onAddedDatabase, this, &PluginActivator::addedDatabaseMSSQL);
    config_dlg_mssql->exec();
}

void PluginActivator::checkPluginMSSQL()
{
    // QFile file(QCoreApplication::applicationDirPath() + "/plugins/plugin_mssql.dll");
    // if (!file.exists())
    //     status_mssql->setText(tr("Biblioteca plugin-lui MSSQL nu este incărcată"));
    // else
    //     status_mssql->setText(tr("Biblioteca plugin-lui MSSQL este determinat"));
    status_mssql->setText(tr("Se află în procesul de dezvoltare !!!"));
}

void PluginActivator::checkPluginRsync()
{
    status_rsync->setText(tr("Se află în procesul de dezvoltare !!!"));
}

void PluginActivator::checkPluginOneDrive()
{
    status_onedrive->setText(tr("Se află în procesul de dezvoltare !!!"));
}
