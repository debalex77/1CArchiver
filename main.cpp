#include "src/mainwindow.h"
#include "src/thememanager.h"
#include "src/globals.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const auto args = QCoreApplication::arguments();
    const bool autoRun = args.contains("--autorun");
    if (autoRun) {
        MainWindow *w = new MainWindow;
        w->setAttribute(Qt::WA_DontShowOnScreen, true);

        QObject::connect(w, &MainWindow::allJobsFinished,
            &a, &QCoreApplication::quit);

        QTimer::singleShot(0, w, &MainWindow::startBackup);

        return a.exec();
    }

    /** determinam stilul */
    QFile f(ThemeManager::isDark()
                ? ":/styles/dark_style.qss"
                : ":/styles/main_style.qss");

    // incarcam stilul aplicatiei
    if (f.open(QFile::ReadOnly))
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));

    MainWindow w;
    w.show();
    return a.exec();
}
