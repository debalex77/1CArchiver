#include "aboutdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QIcon>

#include "version.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Despre aplicație"));
    setModal(true);
    resize(520, 320);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->setSpacing(12);

    // --- Header (icon + titlu)
    auto *headerLay = new QHBoxLayout;

    auto *icon = new QLabel;
    icon->setPixmap(QIcon(":/icons/icons/backup.png").pixmap(64, 64));
    headerLay->addWidget(icon);

    auto *titleLay = new QVBoxLayout;

    auto *lblTitle = new QLabel(tr("<b>1CArchiver</b>"));
    lblTitle->setStyleSheet("font-size: 18px;");
    titleLay->addWidget(lblTitle);

    auto *lblVersion = new QLabel(
        tr("Versiune: %1").arg(VER));
    lblVersion->setStyleSheet("color: gray;");
    titleLay->addWidget(lblVersion);

    headerLay->addLayout(titleLay);
    headerLay->addStretch();

    mainLay->addLayout(headerLay);

    // --- Descriere
    auto *lblDesc = new QLabel(
        tr("1CArchiver este o aplicație destinată arhivării automate a "
           "bazelor de date 1C:Enterprise, cu suport pentru verificare "
           "integritate (SHA-256), programare și sincronizare opțională "
           "cu servicii externe."));
    lblDesc->setWordWrap(true);
    mainLay->addWidget(lblDesc);

    // --- Licență
    auto *lblLicenseTitle = new QLabel(tr("<b>Licență</b>"));
    mainLay->addWidget(lblLicenseTitle);

    auto *lblLicense = new QLabel(
        tr("Această aplicație este distribuită sub licență "
           "<a href=\"https://github.com/debalex77/1CArchiver/blob/master/LICENSE\" "
           "style=\"color:#1E90FF; text-decoration:none;\">MIT</a>.<br>"
           "Utilizarea, modificarea și redistribuirea sunt permise "
           "în conformitate cu termenii acestei licențe."));

    lblLicense->setTextFormat(Qt::RichText);
    lblLicense->setOpenExternalLinks(true);
    lblLicense->setWordWrap(true);
    mainLay->addWidget(lblLicense);

    // --- Credite iconițe
    auto *lblIcons = new QLabel(
        tr("Iconițele utilizate în aplicație sunt furnizate de "
           "<a href=\"https://www.flaticon.com/\" "
           "style=\"color:#1E90FF; text-decoration:none;\">Flaticon</a>."));
    lblIcons->setOpenExternalLinks(true);
    lblIcons->setStyleSheet("color: gray;");
    mainLay->addWidget(lblIcons);

    mainLay->addStretch();

    // --- Buton OK
    auto *btnOk = new QPushButton(tr("OK"));
    btnOk->setDefault(true);

    auto *btnLay = new QHBoxLayout;
    btnLay->addStretch();
    btnLay->addWidget(btnOk);

    mainLay->addLayout(btnLay);

    connect(btnOk, &QPushButton::clicked,
            this, &QDialog::accept);
}

