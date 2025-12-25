#include "dropboxconnectdialog.h"
#include "ui_dropboxconnectdialog.h"

#include "connectordropbox.h"
#include "dropboxoauth2_pkce.h"
#include "src/globals.h"

#include <QMessageBox>

DropboxConnectDialog::DropboxConnectDialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DropboxConnectDialog),
    m_connector(new ConnectorDropbox(this))
{
    ui->setupUi(this);

    connect(ui->btnLogin, &QPushButton::clicked,
            this, &DropboxConnectDialog::onLoginClicked);

    connect(ui->btnTest, &QPushButton::clicked,
            this, &DropboxConnectDialog::onTestClicked);
}

DropboxConnectDialog::~DropboxConnectDialog()
{
    delete ui;
}

void DropboxConnectDialog::onLoginClicked()
{
    auto *oauth = new DropboxOAuth2_PKCE(this);

    /** Semnale PKCE */
    connect(oauth, &DropboxOAuth2_PKCE::loginSucceeded,
            this, &DropboxConnectDialog::onLoginSuccess);

    connect(oauth, &DropboxOAuth2_PKCE::loginFailed,
            this, &DropboxConnectDialog::onLoginFailed);

    oauth->startLoginFlow();
}

void DropboxConnectDialog::onLoginSuccess()
{
    /** Re-încărcăm tokenurile salvate de PKCE din QSettings */
    ConnectorDropbox tmp;
    ui->lblStatus->setText("Connected to Dropbox ✔");
    globals::activate_syncDropbox = true;
    globals::loginSuccesDropbox = "Last login successful: " +
                                  QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm");
    emit loginSuccesDropbox();
}

void DropboxConnectDialog::onLoginFailed(const QString &msg)
{
    Q_UNUSED(msg);
    ui->lblStatus->setText("Login failed ✖");
}

void DropboxConnectDialog::onTestClicked()
{
    connect(m_connector, &ConnectorDropbox::testFinished,
            this, &DropboxConnectDialog::onTestFinished);

    ui->lblStatus->setText("Testing...");

    m_connector->testUpload();
}

void DropboxConnectDialog::onTestFinished(bool ok, const QString &msg)
{
    Q_UNUSED(msg);
    if (ok)
        ui->lblStatus->setText("Test upload OK ✔");
    else
        ui->lblStatus->setText("Test upload failed ✖");
}
