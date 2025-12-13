#include "appsettings.h"
#include <QHBoxLayout>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <QMessageBox>
#include <QPushButton>

#include <src/dropbox/dropboxconnectdialog.h>
#pragma comment(lib, "dwmapi.lib")

static void enableDarkTitlebar(QWidget* w) {
    HWND hwnd = (HWND)w->winId();
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));   // Dark TitleBar
}
#endif

AppSettings::AppSettings(QWidget *parent) : QDialog(parent)
{
    QString highlightColor = globals::isDark ? " style='color:#61AFEF'"
                                             : "";

    setupUI();
    updateUI();

    connect(btn_setArchivePassword, &SwitchButton::toggled, this, [&](bool on) {
        globals::setArchivePassword = on;
        updateUI();
    });

    connect(btn_backupExtFiles, &SwitchButton::toggled, this, [&](bool on) {
        globals::backupExtFiles = on;
    });

    connect(btn_createFileSHA256, &SwitchButton::toggled, this, [&](bool on) {
        globals::createFileSHA256 = on;
    });

    connect(btn_syncDropbox, &SwitchButton::toggled,
            this, [this, highlightColor](bool on)
            {
                globals::syncDropbox = on;
                globals::activate_syncDropbox = !on;

                if (!globals::activate_syncDropbox && on) {
                    auto* conDlgDropbox = new DropboxConnectDialog(this);

                    connect(conDlgDropbox, &DropboxConnectDialog::loginSuccesDropbox,
                            this, [this, highlightColor]()
                            {
                                lbl_syncDropbox->setText(
                                    tr("Sincronizarea cu <b><span %1>Dropbox</span></b><br>"
                                       "După comprimarea şi arhivare backup-le se sincronizează<br>"
                                       "cu <b><span %1>Dropbox</span></b>.%2")
                                        .arg(
                                            highlightColor,
                                            globals::loginSuccesDropbox.isEmpty()
                                                ? QString()
                                                : "<br><br><span style='color:#7acfcf'>"
                                                      + globals::loginSuccesDropbox
                                                      + "</span>"
                                            )
                                    );
                            });

                    conDlgDropbox->exec();
                    conDlgDropbox->deleteLater();
                }
            });

    // connect(btn_syncGoogleDrive, &SwitchButton::toggled, this, [&](bool on) {
    //     globals::syncGoogleDrive = on;
    //     if (! globals::activate_syncGoogleDrive && on) {

    //     }
    // });

    connect(btn_closeApp, &SwitchButton::toggled, this, [&](bool on) {
        globals::questionCloseApp = on;
    });

    if (globals::isDark)
        enableDarkTitlebar(this);
}

AppSettings::~AppSettings()
{

}

void AppSettings::setChecked(bool on)
{
    btn_backupExtFiles->setChecked(on);
}

bool AppSettings::isChecked() const
{
    return btn_backupExtFiles->isChecked();
}

void AppSettings::setupUI()
{
    setWindowTitle(tr("Setarile aplicatiei"));
    setWindowIcon(QIcon(":/icons/icons/settings.png"));

    QVBoxLayout* v = new QVBoxLayout(this);

    // ----- setarea parolei -----
    auto* layout_pwd = new QHBoxLayout;
    layout_pwd->setContentsMargins(10,10,10,2);
    layout_pwd->setSpacing(10);

    lbl_setArchivePassword = new QLabel(this);
    lbl_setArchivePassword->setStyleSheet("font-size: 12px;");
    lbl_setArchivePassword->setText(tr("Setarea parolei la arhive.<br>"
                                       "La salvarea parolei se criptează <b><span%1>(AES-like XOR + hashed key)</span></b>")
                                        .arg(highlightColor));

    btn_setArchivePassword = new SwitchButton(this);
    btn_setArchivePassword->setChecked(globals::setArchivePassword);

    layout_pwd->addWidget(lbl_setArchivePassword);
    layout_pwd->addStretch();
    layout_pwd->addWidget(btn_setArchivePassword);

    auto* layout_set_pwd = new QHBoxLayout;
    layout_set_pwd->setContentsMargins(10,2,10,10);
    layout_set_pwd->setSpacing(10);

    lbl_pwd = new QLabel(this);
    lbl_pwd->setStyleSheet("font-size: 12px;");
    lbl_pwd->setText(tr("Parola:"));
    edit_pwd = new LineEditPassword(this);
    edit_pwd->setMinimumWidth(200);
    if (globals::setArchivePassword && ! globals::archivePassword.isEmpty())
        edit_pwd->setText(globals::archivePassword);

    layout_set_pwd->addStretch();
    layout_set_pwd->addWidget(lbl_pwd);
    layout_set_pwd->addWidget(edit_pwd);

    // ----- separator0 -----
    auto* layout_spacer0 = new QHBoxLayout;
    layout_spacer0->setContentsMargins(10,0,10,10);
    layout_spacer0->setSpacing(10);

    QFrame* line0 = new QFrame(this);
    line0->setFrameShape(QFrame::HLine);
    line0->setFrameShadow(QFrame::Plain);
    line0->setFixedHeight(1);

    layout_spacer0->addWidget(line0);

    // ----- fisiere externe -----
    auto* layout = new QHBoxLayout;
    layout->setContentsMargins(10,10,10,10);
    layout->setSpacing(10);

    lbl_backupExtFiles = new QLabel(this);
    lbl_backupExtFiles->setStyleSheet("font-size: 12px;");
    lbl_backupExtFiles->setText(tr("Arhivarea fișierelor externe a bazelor de date <br>"
                                   "include directorii ex.: <b><span%1>ExtDb, ExtForms etc.</span></b>")
                                    .arg(highlightColor));

    btn_backupExtFiles = new SwitchButton(this);
    btn_backupExtFiles->setChecked(globals::backupExtFiles);

    layout->addWidget(lbl_backupExtFiles);
    layout->addStretch();
    layout->addWidget(btn_backupExtFiles);

    // ----- separator1 -----
    auto* layout_spacer = new QHBoxLayout;
    layout_spacer->setContentsMargins(10,0,10,10);
    layout_spacer->setSpacing(10);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setFixedHeight(1);

    layout_spacer->addWidget(line);

    // ----- fisierul .sha256 -----
    auto* layout_sha = new QHBoxLayout;
    layout_sha->setContentsMargins(10,10,10,10);
    layout_sha->setSpacing(10);

    lbl_fileSHA256 = new QLabel(this);
    lbl_fileSHA256->setStyleSheet("font-size: 12px;");
    lbl_fileSHA256->setText(tr("Generarea automată a sumelor <b><span%1>SHA-256</span></b><br>"
                               "pentru fișierele arhivate").arg(highlightColor));

    btn_createFileSHA256 = new SwitchButton(this);
    btn_createFileSHA256->setChecked(globals::createFileSHA256);

    layout_sha->addWidget(lbl_fileSHA256);
    layout_sha->addStretch();
    layout_sha->addWidget(btn_createFileSHA256);

    // ----- separator2 -----
    auto* layout_spacer1 = new QHBoxLayout;
    layout_spacer1->setContentsMargins(10,0,10,10);
    layout_spacer1->setSpacing(10);

    QFrame* line1 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Plain);
    line1->setFixedHeight(1);

    layout_spacer1->addWidget(line1);

    // ----- sincronizarea cu Dropbox -----
    auto* layout_syncDropbox = new QHBoxLayout;
    layout_syncDropbox->setContentsMargins(10,10,10,10);
    layout_syncDropbox->setSpacing(10);

    lbl_syncDropbox = new QLabel(this);
    lbl_syncDropbox->setStyleSheet("font-size: 12px;");
    lbl_syncDropbox->setText(
        tr("Sincronizarea cu <b><span %1>Dropbox</span></b><br>"
           "După comprimarea şi arhivare backup-le se sincronizează<br>"
           "cu <b><span %1>Dropbox</span></b>.%2")
            .arg(
                highlightColor,
                globals::loginSuccesDropbox.isEmpty()
                    ? QString()
                    : "<br><br><span style='color:#7acfcf'>"
                          + globals::loginSuccesDropbox
                          + "</span>"
                )
        );

    btn_syncDropbox = new SwitchButton(this);
    btn_syncDropbox->setChecked(globals::syncDropbox);

    layout_syncDropbox->addWidget(lbl_syncDropbox);
    layout_syncDropbox->addStretch();
    layout_syncDropbox->addWidget(btn_syncDropbox);

    // ----- separator3 -----
    auto* layout_spacer2 = new QHBoxLayout;
    layout_spacer2->setContentsMargins(10,0,10,10);
    layout_spacer2->setSpacing(10);

    QFrame* line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    line2->setFrameShadow(QFrame::Plain);
    line2->setFixedHeight(1);

    layout_spacer2->addWidget(line2);

    // ----- sincronizarea cu GoogleDrive -----
    // auto* layout_syncGoogleDrive = new QHBoxLayout;
    // layout_syncGoogleDrive->setContentsMargins(10,10,10,10);
    // layout_syncGoogleDrive->setSpacing(10);

    // lbl_syncGoogleDrive = new QLabel(this);
    // lbl_syncGoogleDrive->setStyleSheet("font-size: 12px;");
    // lbl_syncGoogleDrive->setText(tr("Sincronizarea cu <b><span%1>GoogleDrive</span></b><br>"
    //                                 "După comprimarea şi arhivare backup-le se sincronizează<br>"
    //                                 "cu <b><span%1>GoogleDrive</span><b>.").arg(highlightColor));

    // btn_syncGoogleDrive = new SwitchButton(this);
    // btn_syncGoogleDrive->setChecked(globals::syncGoogleDrive);

    // layout_syncGoogleDrive->addWidget(lbl_syncGoogleDrive);
    // layout_syncGoogleDrive->addStretch();
    // layout_syncGoogleDrive->addWidget(btn_syncGoogleDrive);

    // // ----- separator4 -----
    // auto* layout_spacer3 = new QHBoxLayout;
    // layout_spacer3->setContentsMargins(10,0,10,10);
    // layout_spacer3->setSpacing(10);

    // QFrame* line3 = new QFrame(this);
    // line3->setFrameShape(QFrame::HLine);
    // line3->setFrameShadow(QFrame::Plain);
    // line3->setFixedHeight(1);

    // layout_spacer3->addWidget(line3);

    // ----- inchiderea aplicatiei -----
    auto* layout_closeApp = new QHBoxLayout;
    layout_closeApp->setContentsMargins(10,10,10,10);
    layout_closeApp->setSpacing(10);

    lbl_closeApp = new QLabel(this);
    lbl_closeApp->setStyleSheet("font-size: 12px;");
    lbl_closeApp->setText(tr("Interogarea la închiderea aplicaţiei"));

    btn_closeApp = new SwitchButton(this);
    btn_closeApp->setChecked(globals::questionCloseApp);

    layout_closeApp->addWidget(lbl_closeApp);
    layout_closeApp->addStretch();
    layout_closeApp->addWidget(btn_closeApp);

    // ----- separator5 -----
    auto* layout_spacer4 = new QHBoxLayout;
    layout_spacer4->setContentsMargins(10,0,10,10);
    layout_spacer4->setSpacing(10);

    QFrame* line4 = new QFrame(this);
    line4->setFrameShape(QFrame::HLine);
    line4->setFrameShadow(QFrame::Plain);
    line4->setFixedHeight(1);

    layout_spacer4->addWidget(line4);

    // ----- adaugăm în layout principal -----

    v->addLayout(layout_pwd);
    v->addLayout(layout_set_pwd);
    v->addLayout(layout_spacer0);
    v->addLayout(layout);
    v->addLayout(layout_spacer);
    v->addLayout(layout_sha);
    v->addLayout(layout_spacer1);
    v->addLayout(layout_syncDropbox);
    v->addLayout(layout_spacer2);
    // v->addLayout(layout_syncGoogleDrive);
    // v->addLayout(layout_spacer3);
    v->addLayout(layout_closeApp);
    v->addLayout(layout_spacer4);

    // ----- tema -----
    if (globals::isDark) {
        enableDarkTitlebar(this);
        line0->setStyleSheet("background-color: #444; border: none;");
        line->setStyleSheet("background-color: #444; border: none;");
        line1->setStyleSheet("background-color: #444; border: none;");
        line2->setStyleSheet("background-color: #444; border: none;");
    } else {
        line0->setStyleSheet("background-color: #d0d0d0; border: none;");
        line->setStyleSheet("background-color: #d0d0d0; border: none;");
        line1->setStyleSheet("background-color: #d0d0d0; border: none;");
        line2->setStyleSheet("background-color: #d0d0d0; border: none;");
    }

    this->adjustSize();
}

void AppSettings::updateUI()
{
    lbl_pwd->setVisible(btn_setArchivePassword->isChecked());
    edit_pwd->setVisible(btn_setArchivePassword->isChecked());
}

void AppSettings::closeEvent(QCloseEvent *event)
{
    // golim parola
    if (! btn_setArchivePassword->isChecked() && ! edit_pwd->text().isEmpty()){
        globals::setArchivePassword = false;
        edit_pwd->setText("");
        globals::archivePassword = "";
    }

    // verificam daca este activata setarea parolei + parola empty
    if (btn_setArchivePassword->isChecked() && edit_pwd->text().isEmpty()) {
        QMessageBox msg(
            QMessageBox::Question,
            tr("Verificarea parolei"),
            tr("Este activată setarea parolei pentru arhive,<br>"
               "iar parola nu este indicată !!! Continuăm ?"),
            QMessageBox::NoButton,
            this
            );

        QPushButton* yesButton = msg.addButton(tr("Da"), QMessageBox::YesRole);
        QPushButton* noButton  = msg.addButton(tr("Nu"), QMessageBox::NoRole);

        msg.exec();
        if (msg.clickedButton() == yesButton) {
            event->accept();
        } else if (msg.clickedButton() == noButton){
            event->ignore();
        }
    }

    // setam textul
    if (btn_setArchivePassword->isChecked() && ! edit_pwd->text().isEmpty()) {
        globals::archivePassword = edit_pwd->text();
        event->accept();
    }

    event->accept();
}
