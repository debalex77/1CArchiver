#include "pluginactivator.h"
#include "src/notify/telegramnotifier.h"

#include <QMessageBox>
#include <QSysInfo>

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
    connect(btnExport1C, &SwitchButton::toggled, this, &PluginActivator::onClickExport1C);
    connect(btnTelegram, &SwitchButton::toggled, this, &PluginActivator::onClickTelegram);

    connect(btnConfigMSSQL, &QPushButton::clicked, this, &PluginActivator::onClickConfigMSSQL);
    connect(btnConfigTelegram, &QPushButton::clicked, this, &PluginActivator::onClickConfigTelegram);
    connect(btnTestTelegram, &QPushButton::clicked, this, &PluginActivator::onClickTestTelegram);
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
    // --- INFO atentie utilizatori
    //-------------------------------------------------
    auto *layout_info = new QHBoxLayout;
    layout_info->setContentsMargins(10,10,10,10);
    layout_info->setSpacing(10);

    lblInfo = new QLabel(
        tr("⚠ Pluginurile opționale sunt destinate utilizatorilor avansați.\n"
           "Activarea acestora poate modifica comportamentul aplicației.\n"
           "Activați doar pluginurile pe care le înțelegeți și le utilizați."),
        this);

    lblInfo->setWordWrap(true);
    lblInfo->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    lblInfo->setStyleSheet(
        globals::isDark
            ? "QLabel {"
              "background-color: #3a2f1a;"
              "border: 1px solid #6b5b2a;"
              "color: #f5e6b3;"
              "padding: 8px;"
              "border-radius: 4px;"
              "}"
            : "QLabel {"
              "background: #fff3cd;"
              "border: 1px solid #ffeeba;"
              "color: #664d03;"
              "padding: 8px;"
              "border-radius: 4px;"
              "}"
        );

    layout_info->addWidget(lblInfo);

    QFrame* line_info = new QFrame(this);
    line_info->setFrameShape(QFrame::HLine);
    line_info->setFrameShadow(QFrame::Plain);
    line_info->setFixedHeight(1);

    v->addLayout(layout_info);
    v->addWidget(line_info);

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
    desc_mssql->setText(
        tr("Activarea pluginului pentru baze de date<br>"
           "Microsoft SQL Server (compatibil cu versiunile 2012<br>"
           "și mai noi).")
        );

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
    // --- EXPORT 1C (.dt)
    //-------------------------------------------------
    auto *layout_export1c = new QHBoxLayout;
    layout_export1c->setContentsMargins(10,10,10,2);
    layout_export1c->setSpacing(10);

    lbl_export1c = new QLabel(this);
    lbl_export1c->setStyleSheet("font-size: 14px; font-weight: bold;");
    lbl_export1c->setText(tr("Plugin Export 1C (.dt)"));

    btnExport1C = new SwitchButton(this);
    btnExport1C->setChecked(globals::pl_export1c);

    desc_export1c = new QLabel(this);
    desc_export1c->setStyleSheet("font-size: 11px;");
    desc_export1c->setText(
        tr("Exportul bazelor în format .dt (în locul .1CD / .bak)<br>"
           "cu ajutorul platformei 1C (1cv8.exe DESIGNER /DumpIB).")
        );

    status_export1c = new QLabel(this);
    status_export1c->setStyleSheet("font-size: 11px; font-style: italic; color: #7acfcf;");
    checkPluginExport1C();

    layout_export1c->addWidget(btnExport1C);
    layout_export1c->addWidget(desc_export1c);
    layout_export1c->addStretch();

    QFrame* line_export1c = new QFrame(this);
    line_export1c->setFrameShape(QFrame::HLine);
    line_export1c->setFrameShadow(QFrame::Plain);
    line_export1c->setFixedHeight(1);

    v->addWidget(lbl_export1c);
    v->addLayout(layout_export1c);
    v->addWidget(status_export1c);
    v->addWidget(line_export1c);

    //-------------------------------------------------
    // --- TELEGRAM
    //-------------------------------------------------
    auto *layout_telegram = new QHBoxLayout;
    layout_telegram->setContentsMargins(10,10,10,2);
    layout_telegram->setSpacing(10);

    lbl_telegram = new QLabel(this);
    lbl_telegram->setStyleSheet("font-size: 14px; font-weight: bold;");
    lbl_telegram->setText(tr("Plugin Telegram"));

    btnTelegram = new SwitchButton(this);
    btnTelegram->setChecked(globals::pl_telegram);

    desc_telegram = new QLabel(this);
    desc_telegram->setStyleSheet("font-size: 11px;");
    desc_telegram->setText(
        tr("Trimiterea rezumatului și a logului arhivării<br>"
           "în Telegram (prin bot).")
        );

    btnConfigTelegram = new QPushButton(this);
    btnConfigTelegram->setText(tr("Configurarea"));

    btnTestTelegram = new QPushButton(this);
    btnTestTelegram->setText(tr("Test"));

    status_telegram = new QLabel(this);
    status_telegram->setStyleSheet("font-size: 11px; font-style: italic; color: #7acfcf;");
    checkPluginTelegram();

    layout_telegram->addWidget(btnTelegram);
    layout_telegram->addWidget(desc_telegram);
    layout_telegram->addStretch();
    layout_telegram->addWidget(btnConfigTelegram);
    layout_telegram->addWidget(btnTestTelegram);

    QFrame* line_telegram = new QFrame(this);
    line_telegram->setFrameShape(QFrame::HLine);
    line_telegram->setFrameShadow(QFrame::Plain);
    line_telegram->setFixedHeight(1);

    v->addWidget(lbl_telegram);
    v->addLayout(layout_telegram);
    v->addWidget(status_telegram);
    v->addWidget(line_telegram);

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
    status_export1c->setVisible(btnExport1C->isChecked());
    status_telegram->setVisible(btnTelegram->isChecked());

    btnConfigMSSQL->setEnabled(btnMSSQL->isChecked());
    btnConfigRsync->setEnabled(btnRsync->isChecked());
    btnConfigOneDrive->setEnabled(btnOneDrive->isChecked());
    btnConfigTelegram->setEnabled(btnTelegram->isChecked());
    btnTestTelegram->setEnabled(btnTelegram->isChecked());

    this->adjustSize();
}

void PluginActivator::onClickMSSQL(bool on)
{
    globals::pl_mssql = on;

    emit activatePlugin("mssql", on);

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

void PluginActivator::onClickExport1C(bool on)
{
    globals::pl_export1c = on;

    emit activatePlugin("export_1c", on);

    if (on)
        checkPluginExport1C();
    updateUI();
}

void PluginActivator::onClickTelegram(bool on)
{
    globals::pl_telegram = on;

    emit activatePlugin("telegram", on);

    if (on)
        checkPluginTelegram();
    updateUI();
}

void PluginActivator::onClickConfigTelegram()
{
    PluginConfigDialog config_dlg_telegram("telegram",
                                           TelegramNotifier::configPath(),
                                           this);
    config_dlg_telegram.exec();
    checkPluginTelegram();
}

void PluginActivator::onClickTestTelegram()
{
    auto *notifier = new TelegramNotifier(this); /** eliberat la finished sau odata cu dialogul */

    QString err;
    if (!notifier->loadConfig(&err)) {
        notifier->deleteLater();
        QMessageBox::warning(this, tr("Telegram"), err);
        return;
    }

    btnTestTelegram->setEnabled(false);

    connect(notifier, &TelegramNotifier::finished,
            this, [this, notifier](bool ok, const QString &error) {
                notifier->deleteLater();
                btnTestTelegram->setEnabled(btnTelegram->isChecked());

                if (ok)
                    QMessageBox::information(this, tr("Telegram"),
                                             tr("Mesajul de test a fost trimis."));
                else
                    QMessageBox::warning(this, tr("Telegram"),
                                         tr("Mesajul nu a fost trimis: %1").arg(error));
            });

    notifier->send(tr("1CArchiver: mesaj de test (%1)").arg(QSysInfo::machineHostName()));
}

void PluginActivator::onClickConfigMSSQL()
{
    PluginConfigDialog config_dlg_mssql("mssql",
                                        QString(),
                                        this);
    connect(&config_dlg_mssql, &PluginConfigDialog::onAddedDatabase,
            this, &PluginActivator::addedDatabaseMSSQL, Qt::UniqueConnection);
    config_dlg_mssql.exec();
}

void PluginActivator::checkPluginMSSQL()
{
    status_mssql->setText(tr("Funcționalitate în stadiu de beta-testare."));
}

void PluginActivator::checkPluginRsync()
{
    status_rsync->setText(tr("Se află în procesul de dezvoltare !!!"));
}

void PluginActivator::checkPluginOneDrive()
{
    status_onedrive->setText(tr("Se află în procesul de dezvoltare !!!"));
}

void PluginActivator::checkPluginExport1C()
{
    status_export1c->setText(tr("Configurarea: click dreapta pe bază în tabel → «Configurare export .dt»."));
}

void PluginActivator::checkPluginTelegram()
{
    status_telegram->setText(QFile::exists(TelegramNotifier::configPath())
                                 ? tr("Configurat.")
                                 : tr("Nu este configurat - apăsați «Configurarea»."));
}
