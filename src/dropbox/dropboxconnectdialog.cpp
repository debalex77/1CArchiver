#include "dropboxconnectdialog.h"
#include "ui_dropboxconnectdialog.h"

#include "connectordropbox.h"
#include "dropboxoauth2_pkce.h"
#include "src/globals.h"

#include <QMessageBox>

/*
 * Constructor UI
 */
DropboxConnectDialog::DropboxConnectDialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::DropboxConnectDialog),
    m_connector(new ConnectorDropbox(this))
{
    ui->setupUi(this);

    // Connect login button
    connect(ui->btnLogin, &QPushButton::clicked,
            this, &DropboxConnectDialog::onLoginClicked);

    // Connect test button
    connect(ui->btnTest, &QPushButton::clicked,
            this, &DropboxConnectDialog::onTestClicked);
}

/*
 * Destructor
 */
DropboxConnectDialog::~DropboxConnectDialog()
{
    delete ui;
}

/*
 * Pornește fluxul OAuth2 PKCE
 */
void DropboxConnectDialog::onLoginClicked()
{
    auto *oauth = new DropboxOAuth2_PKCE(this);

    // Semnale PKCE
    connect(oauth, &DropboxOAuth2_PKCE::loginSucceeded,
            this, &DropboxConnectDialog::onLoginSuccess);

    connect(oauth, &DropboxOAuth2_PKCE::loginFailed,
            this, &DropboxConnectDialog::onLoginFailed);

    oauth->startLoginFlow();

    // QMessageBox::information(this, "Dropbox",
    //                          "Browser opened. Please complete login.");
}

/*
 * Login succes → Salvăm tokenurile + anunțăm UI
 */
void DropboxConnectDialog::onLoginSuccess()
{
    // Re-încărcăm tokenurile salvate de PKCE din QSettings
    ConnectorDropbox tmp;
    ui->lblStatus->setText("Connected to Dropbox ✔");
    globals::activate_syncDropbox = true;
    globals::loginSuccesDropbox = "Last login successful: " +
                                  QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm");
    emit loginSuccesDropbox();
    // QMessageBox::information(this, "Dropbox", "Login successful !!!!");
}

/*
 * Login fail
 */
void DropboxConnectDialog::onLoginFailed(const QString &msg)
{
    Q_UNUSED(msg);
    ui->lblStatus->setText("Login failed ✖");
    // QMessageBox::warning(this, "Dropbox Login Failed", msg);
}

/*
 * Test button
 */
void DropboxConnectDialog::onTestClicked()
{
    connect(m_connector, &ConnectorDropbox::testFinished,
            this, &DropboxConnectDialog::onTestFinished);

    ui->lblStatus->setText("Testing...");

    m_connector->testUpload();
}

/*
 * Test finished
 */
void DropboxConnectDialog::onTestFinished(bool ok, const QString &msg)
{
    Q_UNUSED(msg);
    if (ok)
    {
        ui->lblStatus->setText("Test upload OK ✔");
        // QMessageBox::information(this, "Dropbox Test", msg);
    }
    else
    {
        ui->lblStatus->setText("Test upload failed ✖");
        // QMessageBox::warning(this, "Dropbox Test Failed", msg);
    }
}
