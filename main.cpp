#include "src/mainwindow.h"
#include "src/thememanager.h"
#include "src/globals.h"
#include <QApplication>
#include <QMessageBox>
#include <QSystemTrayIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /** autorun pu Task Scheduler */
    const auto args = QCoreApplication::arguments();
    const bool autoRun = args.contains("--autorun");
    if (autoRun) {

        globals::isAutorun = true;

        MainWindow *w = new MainWindow;
        w->setAttribute(Qt::WA_DontShowOnScreen, true);

        auto *tray = new QSystemTrayIcon(&a);
        tray->setIcon(QIcon(":/icons/icons/backup.png"));
        tray->show();

        /** mesaj pu finalizarea arhivarii */
        QObject::connect(w, &MainWindow::allJobsFinished,
                         tray,
                         [tray, &a]() {

                             /** prezentam msg de finalizarea arhivarii - 15 secunde */
                             tray->showMessage(
                                 QObject::tr("Arhivare finalizată"),
                                 QObject::tr("Arhivarea bazelor de date 1C a fost finalizată cu succes."),
                                 QSystemTrayIcon::Information,
                                 15000
                                 );

                             /** inchidem aplicaia peste 5 secunde */
                             QTimer::singleShot(5000, &a, &QCoreApplication::quit);
                         });

        /** Mesaj inițial in tray */
        tray->showMessage(
            QObject::tr("Atenție"),
            QObject::tr("Peste 1 minut va fi inițiată arhivarea bazelor de date 1C."),
            QSystemTrayIcon::Information,
            10000
            );

        /** Mesaj pu atentie */
        QMessageBox msg;
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle(QObject::tr("Atenție"));
        msg.setText(QObject::tr(
            "Peste 1 minut va fi inițiată arhivarea bazelor de date 1C:Enterprise.\n"
            "Pentru o arhivare corectă este necesar să închideți toate bazele de date 1C."
            ));
        msg.setStandardButtons(QMessageBox::Ok);
        msg.setWindowModality(Qt::ApplicationModal);
        QTimer::singleShot(15000, &msg, &QMessageBox::accept); /** auto-close peste 15 secunde */
        msg.exec();

        /** lansam startBackup() dupa ce utilizatorul a vizualizat mesaje */
        QTimer::singleShot(60000, w, &MainWindow::startBackup);

        return a.exec();
    }

    /** determinam stilul */
    QFile f(ThemeManager::isDark()
                ? ":/styles/dark_style.qss"
                : ":/styles/main_style.qss");

    /** incarcam stilul aplicatiei */
    if (f.open(QFile::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));

    MainWindow w;
    w.show();
    return a.exec();
}
