#include "mainwindow.h"
#include "compressworker.h"
#include "switchbutton.h"
#include "updatedialog.h"
#include "updatechecker.h"
#include "aboutdialog.h"
#include "ibaseparser.h"
#include "src/version.h"
#include "src/globals.h"
#include "src/utils.h"
#include "src/IBASEEntry.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDateTime>
#include <QCheckBox>
#include <QFileInfo>
#include <QJsonArray>
#include <QThread>
#include <QMenu>
#include <QMenuBar>
#include <QCryptographicHash>
#include <QTextBlock>
#include <QDirIterator>
#include <QSettings>

#include <src/core/WorkerMssql.h>
#include <src/core/pluginactivator.h>
#include <src/core/pluginmanager.h>
#include <src/dropbox/connectordropbox.h>
#include <src/dropbox/dropboxhealthchecker.h>
#include <src/dropbox/dropboxoauth2_pkce.h>
#include <src/scheduler/scheduledtaskdialog.h>
#include <src/ui/pluginconfigdialog.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

static void enableDarkTitlebar(QWidget* w) {
    HWND hwnd = (HWND)w->winId();
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));   // Dark TitleBar
}
#endif

static QString toWinPath(const QString &path)
{
    QString p = path;        // facem copie modificabilă
    p.replace('/', '\\');    // înlocuim separatorii
    return p;
}

static bool waitForFileReady(const QString &path, int timeoutMs = 5000)
{
    QElapsedTimer t;
    t.start();

    while (t.elapsed() < timeoutMs) {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)) {
            f.close();
            return true;
        }
        QThread::msleep(200);
    }
    return false;
}

uint64_t folderSize(const QString& path) {
    uint64_t total = 0;
    QDir dir(path);

    const QFileInfoList files = dir.entryInfoList(
        QDir::Files | QDir::Hidden | QDir::NoSymLinks
        );
    for (const QFileInfo& fi : files) {
        total += fi.size();
    }

    const QFileInfoList dirs = dir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot
        );
    for (const QFileInfo& fi : dirs) {
        total += folderSize(fi.absoluteFilePath());
    }

    return total;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_lib(nullptr),
    m_compressor(nullptr)
{
    // Titlu aplicatiei
    // ---------------------------------------------------------

    resize(920, 680);
    setWindowIcon(QIcon(":/icons/icons/backup.png"));
    setWindowTitle(tr("1CArchiver v%1 – Backup al bazelor de date 1C:Enterprise")
                       .arg(VER));

    // Tema si limba aplicatiei
    // ---------------------------------------------------------

    QHBoxLayout* topBar = new QHBoxLayout;
    topBar->setContentsMargins(0,0,0,0);
    topBar->setSpacing(10);

    /** --- about */
    btnAbout = new QToolButton(this);
    btnAbout->setToolTip(tr("Despre aplicația"));
    btnAbout->setIcon(QIcon(":/icons/icons/about.png"));
    connect(btnAbout, &QToolButton::clicked, this, [&](){
        AboutDialog dlg(this);
        dlg.exec();
    });

    /** --- plugins */
    btnPlugins = new QToolButton(this);
    btnPlugins->setToolTip(tr("Activarea plugin-lor"));
    btnPlugins->setIcon(QIcon(":/icons/icons/plugin.png"));
    connect(btnPlugins, &QToolButton::clicked, this, [&]() {

        PluginActivator *pl_activator = new PluginActivator(this);
        connect(pl_activator, &PluginActivator::activatePlugin,
                this, &MainWindow::onActivatePlugins, Qt::UniqueConnection);
        connect(pl_activator, &PluginActivator::addedDatabaseMSSQL,
                this, &MainWindow::onAddedDatabaseMSSQL, Qt::UniqueConnection);
        pl_activator->exec();
    });

    /** --- btn setari */
    btnSettings = new QToolButton(this);
    btnSettings->setToolTip(tr("Setările aplicației"));
    btnSettings->setIcon(QIcon(":/icons/icons/settings.png"));
    connect(btnSettings, &QToolButton::clicked, this, [&](){
        app_settings = new AppSettings(this);
        app_settings->setAttribute(Qt::WA_DeleteOnClose);
        connect(app_settings, &AppSettings::onActivateDropbox, [&]() {
            setPropertyVisible();
            if (globals::syncDropbox && globals::activate_syncDropbox)
                checkDropboxAtStartup();
        });
        app_settings->show();
    });

    /** --- tema */
    themeLabel = new QLabel(this);
    // themeLabel->setText(tr("Dark theme:"));

    themeSwitch = new SwitchButton(this);
    themeSwitch->setChecked(globals::isDark);

    connect(themeSwitch, &SwitchButton::toggled, this, [&](bool on) {
        globals::isDark = on;
        applyTheme();
    });

    /** --- limba */
    lblLang = new QLabel(this);
    // lblLang->setText(tr("Limba RO:"));
    lblSwitch = new SwitchButton(this);
    lblSwitch->setChecked(currentLang == "app_ro_RO");
    connect(lblSwitch, &SwitchButton::toggled, this, [&](bool on) {
        globals::currentLang = (on == true) ? "app_ro_RO" : "app_ru_RU";
        switchLanguage(globals::currentLang);
    });

    topBar->addStretch();
    topBar->addWidget(lblLang);
    topBar->addWidget(lblSwitch);
    topBar->addWidget(themeLabel);
    topBar->addWidget(themeSwitch);
    topBar->addWidget(btnSettings);
    topBar->addWidget(btnPlugins);
    topBar->addWidget(btnAbout);

    // central widget
    // ---------------------------------------------------------

    QWidget *w = new QWidget(this);
    setCentralWidget(w);

    QVBoxLayout *v = new QVBoxLayout(w);

    v->addLayout(topBar);

    // drumul spre setarile aplicatiei
    // ---------------------------------------------------------

    settingsFilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                           .filePath("settings.json");
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    // UI – tabel
    // ---------------------------------------------------------
    table = new QTableWidget(0, 4, this);
    table->setHorizontalHeaderLabels(
        {tr("Select"),
         tr("Denumire BD"),
         tr("Cale"),
         tr("Status")}
    );
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(table, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::onTableContextMenu);

    v->addWidget(table);

    // UI – butoane
    // ---------------------------------------------------------
    QHBoxLayout *btns = new QHBoxLayout;

    btnSelectAll    = new QPushButton(tr("Selectează toate"));
    btnWithDb       = new QPushButton(tr("Directoriu cu BD"));
    btnFolder       = new QPushButton(tr("Alege folder backup"));
    btnArchive      = new QPushButton(tr("Arhivează selectatele"));
    btnGenerateTask = new QPushButton(tr("Generează Task XML"));

    btnSelectAll->setToolTip(tr("Selectează toate <br>"
                                "bazele de date din tabel"));
    btnWithDb->setToolTip(tr("Calea spre directoriu,<br>"
                             "unde sunt baze de date 1C"));
    btnFolder->setToolTip(tr("Calea spre directoriu,<br>"
                             "unde v-a fi păstrate arhive"));
    btnArchive->setToolTip(tr("Lansarea arhivării/sincronizării"));
    btnGenerateTask->setToolTip(tr("Generarea task"));

    btnSelectAll->setMinimumWidth(140);
    btnWithDb->setMinimumWidth(140);
    btnFolder->setMinimumWidth(140);
    btnArchive->setMinimumWidth(140);
    btnGenerateTask->setMinimumWidth(140);

    btns->addWidget(btnSelectAll);
    btns->addWidget(btnWithDb);
    btns->addWidget(btnFolder);
    btns->addWidget(btnArchive);
    btns->addWidget(btnGenerateTask);
    btns->addStretch();

    connect(btnWithDb, &QPushButton::clicked, this, &MainWindow::onChooseDirWithDb);
    connect(btnGenerateTask, &QPushButton::clicked, this, [this]() {
        ScheduledTaskDialog dlg(this);
        dlg.exec();
    });

    // Combobox compresie
    // ---------------------------------------------------------
    comboCompression = new QComboBox(this);
    for (int i = 0; i <= 9; ++i)
        comboCompression->addItem(QString::number(i));
    comboCompression->setCurrentIndex(9);

    lblCompression = new QLabel(this);
    btns->addWidget(lblCompression);
    btns->addWidget(comboCompression);

    v->addLayout(btns);

    // UI – progres + status
    // ---------------------------------------------------------
    currentStatus = new QLabel("Arhivarea: ... așteptare", this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    v->addWidget(currentStatus);
    v->addWidget(progressBar);

    // UI – progres + status DROPBOX
    // ---------------------------------------------------------
    currentStatusDropbox = new QLabel(tr("Dropbox: verificare conexiune..."), this);
    progressBarDropbox = new QProgressBar(this);
    progressBarDropbox->setRange(0, 100);
    progressBarDropbox->setValue(0);

    btnAbortDropbox = new QPushButton(tr("Oprește Dropbox"), this);
    btnAbortDropbox->setEnabled(false);
    connect(btnAbortDropbox, &QPushButton::clicked,
            this, &MainWindow::clickedAbortDropbox, Qt::UniqueConnection);

    auto *layoutDropbox = new QHBoxLayout;
    layoutDropbox->addWidget(currentStatusDropbox);
    layoutDropbox->addStretch();
    layoutDropbox->addWidget(btnAbortDropbox);

    v->addLayout(layoutDropbox);
    v->addWidget(progressBarDropbox);

    // UI – log
    // ---------------------------------------------------------
    logBox = new QTextEdit(this);
    logBox->setReadOnly(true);
    v->addWidget(logBox);

    // Init folder backup
    // ---------------------------------------------------------
    backupFolder = QDir::homePath() + "/Backups_1C";
    QDir().mkpath(backupFolder);

    // CITIRE ibases.v8i
    // ---------------------------------------------------------
    autoDetectPaths1C();

    // Conectări
    // ---------------------------------------------------------
    connect(btnSelectAll, &QPushButton::clicked, this, &MainWindow::onSelectAll);
    connect(btnFolder,    &QPushButton::clicked, this, &MainWindow::onChooseBackupFolder);
    connect(btnArchive,   &QPushButton::clicked, this, &MainWindow::onStartArchive);

    // Incarcam datele din .json
    // ---------------------------------------------------------
    loadSettings();

    // Verificam prezenta 7z.dll
    // ---------------------------------------------------------
#ifndef QT_DEBUG
    check7ZipInstallation();
#endif

    // Timer progres bazat pe dimensiunea arhivei
    // ---------------------------------------------------------
    progressTimer = new QTimer(this);
    progressTimer->setInterval(100);

    connect(progressTimer, &QTimer::timeout, this, [this]() {
        QFileInfo fi(currentArchivePath);
        if (!fi.exists() || sourceFileSize <= 0) return;

        qint64 curr = fi.size();
        int pct = int(double(curr) / double(sourceFileSize) * 100.0);
        if (pct > 100) pct = 100;

        progressBar->setValue(pct);
        currentStatus->setText(QString(tr("Progres: %1%")).arg(pct));
    });

    // bottom bar
    // ---------------------------------------------------------

    auto* footerLayout = new QHBoxLayout;
    footerLayout->setContentsMargins(0, 10, 0, 5);
    footerLayout->setAlignment(Qt::AlignCenter);

    auto* infoApp = new QLabel(this);
    infoApp->setStyleSheet("color: #888; font-size: 11px;");

    const int startYear = 2025;
    const int currentYear = QDate::currentDate().year();

    QString years = (currentYear == startYear)
                        ? QString::number(startYear)
                        : QString("%1–%2").arg(startYear).arg(currentYear);

    infoApp->setText(QString("© %1 · 1carchiver.app@gmail.com").arg(years));
    footerLayout->addWidget(infoApp);

    v->addLayout(footerLayout);

    connect(this, &MainWindow::jobFinishedSignal, this, &MainWindow::startNextJob);

    enableDarkTitlebar(this);

}

MainWindow::~MainWindow()
{
}

void MainWindow::startBackup()
{
    onStartArchive();
}

void MainWindow::onActivatePlugins(const QString &IdPlugin, const bool on)
{
    if (IdPlugin == "mssql") {

        /** initial */
        if (menuAddDb) {
            btnWithDb->setMenu(nullptr);
            delete menuAddDb;
            menuAddDb = nullptr;
        }

        btnWithDb->disconnect();

        if (on && globals::pl_mssql) {
            createSubmenuMSSQL();
        } else {
            connect(btnWithDb, &QPushButton::clicked,
                    this, &MainWindow::onChooseDirWithDb, Qt::UniqueConnection);
        }
    }
}

// ======================================
// LOG
// ======================================

void MainWindow::log(const QString &msg)
{
    logBox->append(msg);
}

// ======================================
// Selectează toate
// ======================================

void MainWindow::onChooseDirWithDb()
{
    QString root = QFileDialog::getExistingDirectory(
        this, tr("Alege directorul cu baze 1C"), "C:/");

    if (!root.isEmpty()) {

        auto paths = find1CDBaseFolders(root);

        if (table->rowCount() > 0 && ! paths.isEmpty()) {
            auto ret = QMessageBox::question(
                this,
                tr("Confirmare"),
                tr("Tabelul conține deja date.\nDoriți să adăugați încă %1 baze?")
                    .arg(paths.size()),
                QMessageBox::Yes | QMessageBox::No
                );

            if (ret == QMessageBox::No)
                return;
            else
                log(tr("Sunt adaugate %1 baze de date din directoriu - %2")
                        .arg(QString::number(paths.size()),
                             toWinPath(root)));
        }

        for (const QString &path : std::as_const(paths)) {

            int row = table->rowCount();   // rând nou la final
            table->insertRow(row);

            // ===== Col 0: checkbox =====
            QWidget *cw    = new QWidget(table);
            QHBoxLayout *h = new QHBoxLayout(cw);
            QCheckBox *cb  = new QCheckBox(cw);

            h->addWidget(cb);
            h->setAlignment(Qt::AlignCenter);
            h->setContentsMargins(0,0,0,0);
            cw->setLayout(h);

            /** 0. checkbox */
            table->setCellWidget(row, 0, cw);

            /** 1. nume BD */
            QString dbName = QFileInfo(path).fileName(); // numele folderului
            auto *dbItem = new QTableWidgetItem(dbName);
            table->setItem(row, 1, dbItem);

            /** 2. path BD */
            auto *dbPath = new QTableWidgetItem(path);
            table->setItem(row, 2, dbPath);

            /** 3. status */
            QTableWidgetItem *statusItem = new QTableWidgetItem("");
            statusItem->setTextAlignment(Qt::AlignCenter);
            table->setItem(row, 3, statusItem);

            // metadata interna
            dbItem->setData(Qt::UserRole,     "one_file");
            dbItem->setData(Qt::UserRole + 1, true);
            dbItem->setData(Qt::UserRole + 2, "");
        }
    }
}

void MainWindow::onSelectAll()
{
    for (int i = 0; i < table->rowCount(); ++i)
    {
        QCheckBox *cb = table->cellWidget(i, 0)->findChild<QCheckBox *>();
        if (cb) {
            if (cb->isChecked()) {
                cb->setChecked(false);
            } else {
                cb->setChecked(true);
            }
        }
    }
}

// ======================================
// Alege folder backup
// ======================================

void MainWindow::onChooseBackupFolder()
{
    QString d = QFileDialog::getExistingDirectory(this, tr("Alege folder backup"));
    if (!d.isEmpty()) {
        backupFolder = d;
        log(tr("Folder backup setat: ") + toWinPath(backupFolder));
    }
}

// ======================================
// Începe arhivarea
// ======================================

void MainWindow::onStartArchive()
{
    /** atentionare daca exista baze FILE selectate */
    if (!globals::isAutorun) {

        bool hasFileDb = false;

        /** setam variabila - hasFileDb */
        for (int i = 0; i < table->rowCount(); ++i) {
            QCheckBox *cb = table->cellWidget(i,0)->findChild<QCheckBox *>();
            if (!cb || !cb->isChecked())
                continue;

            auto *item = table->item(i,1);
            if (!item)
                continue;

            QString typeDB = item->data(Qt::UserRole).toString();
            if (typeDB == "one_file") {
                hasFileDb = true;
                break;
            }
        }

        /** avertisment */
        if (hasFileDb) {
            QMessageBox msg(QMessageBox::Warning,
                            tr("Atenție"),
                            tr("Pentru o arhivare corectă este necesar să închideți<br>"
                               "toate bazele de date 1C.<br>"
                               "Doriți să continuați ?"),
                            QMessageBox::NoButton, this);
            QPushButton *yesButton = msg.addButton(tr("Da"), QMessageBox::YesRole);
            QPushButton *noButton  = msg.addButton(tr("Nu"), QMessageBox::NoRole);
            yesButton->setMinimumWidth(80);
            noButton->setMinimumWidth(80);
            msg.exec();
            if (msg.clickedButton() == noButton){
                return;
            }
        }
    }

    /** Dezactivarea UI button */
    btnSelectAll->setEnabled(false);
    btnArchive->setEnabled(false);
    btnFolder->setEnabled(false);
    comboCompression->setEnabled(false);

    /** construim job-ul */
    jobs.clear();
    currentJob = -1;

    for (int i = 0; i < table->rowCount(); ++i) {
        QCheckBox *cb = table->cellWidget(i,0)->findChild<QCheckBox *>();
        if (!cb || !cb->isChecked())
            continue;

        auto *itemDb = table->item(i,1);
        if (!itemDb)
            continue;

        QString typeDB = itemDb->data(Qt::UserRole).toString();

        BackupJob j;
        j.row     = i;
        j.dbName  = itemDb->text();
        j.spinner = nullptr;
        j.statusLabel = nullptr;

        if (typeDB == "one_file") {

            QString folder = table->item(i,2)->text();
            QString file1cd = QDir(folder).filePath("1Cv8.1CD"); //QString file1cd = folder + "/1Cv8.1CD";

            QFileInfo f(file1cd);
            if (!f.exists()) {
                log(tr("⛔ Nu găsesc 1Cv8.1CD în: ") + folder);
                continue;
            }

            j.typeDB      = typeDB;
            j.dbFolder    = folder;
            j.file1CD     = file1cd;
            j.archivePath = buildArchiveName(j.dbName);
            j.archiveWholeFolder = globals::backupExtFiles;

        } else if (typeDB == "mssql") {

            if (!globals::pl_mssql) {
                log(tr("⛔ Plugin MSSQL nu este activ: ") + j.dbName);
                continue;
            }

            QString configPath = itemDb->data(Qt::UserRole + 2).toString();

            if (configPath.isEmpty() || !QFileInfo::exists(configPath)) {
                log(tr("⛔ Config MSSQL lipsă pentru: ") + j.dbName);
                continue;
            }

            j.typeDB      = "mssql";
            j.configPath  = configPath;
            j.archivePath = buildArchiveNameMSSQL(j.dbName);

        } else {
            log(tr("⛔ Nu este determinat tipul bazei de date: ") + j.dbName);
            continue;
        }

        jobs.append(j);
    }

    if (jobs.isEmpty()) {
        log(tr("Nu sunt baze selectate pentru backup."));
        return;
    }

    log(QString(tr("Încep backup pentru %1 baze...")).arg(jobs.size()));
    startNextJob();
}

void MainWindow::switchLanguage(const QString &lang)
{
    if (currentLang == lang)
        return;

    currentLang = lang;

    // Dezinstalează traducerea veche
    qApp->removeTranslator(&translator);

    // Dacă limba este RO – nu încărcăm niciun fișier
    if (lang == "app_ro_RO") {
        retranslateUi();       // forțează textul implicit
        return;
    }

    // Încarcă noua traducere
    QString qmFile = ":/icons/translations/1CArchiver_" + lang + ".qm";
    if (translator.load(qmFile)) {
        qApp->installTranslator(&translator);
    } else {
        qDebug() << "Nu pot încărca traducerea:" << qmFile;
    }

    // Aplică traducerea la UI
    retranslateUi();
}

void MainWindow::applyTheme()
{
    if (globals::isDark) {
        QFile f(":/styles/dark_style.qss");
        f.open(QFile::ReadOnly);
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    } else {
        QFile f(":/styles/main_style.qss");
        f.open(QFile::ReadOnly);
        qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
    }
}

void MainWindow::clickedAbortDropbox()
{
    if (!m_dbxUploader)
        return;

    log("⛔ Dropbox: upload abortat de utilizator.");

    currentStatusDropbox->setText(tr("Dropbox: anulat"));
    progressBarDropbox->setValue(0);

    m_dbxUploader->abort();      // foarte important
    m_dbxUploader->deleteLater();
    m_dbxUploader = nullptr;

    btnAbortDropbox->setEnabled(false);

    // permitem continuarea fluxului
    m_waitingForDropbox = false;
    startNextJob();
}

// ======================================
// Meniu contextual și acțiunile
// ======================================

void MainWindow::onTableContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    QAction *actClear = menu.addAction(tr("🧹 Elimină toate rândurile"));
    QAction *actRemove = menu.addAction(tr("❌ Elimină rândul curent"));
    menu.addSeparator();
    QAction *actAuto = menu.addAction(tr("🔍 Detectare automată baze 1C"));

    QAction *selected = menu.exec(table->viewport()->mapToGlobal(pos));
    if (!selected)
        return;

    if (selected == actClear)
        clearAllRows();
    else if (selected == actRemove)
        removeCurrentRow();
    else if (selected == actAuto)
        autoDetectPaths1C();
}

void MainWindow::clearAllRows()
{
    if (table->rowCount() == 0)
        return;

    auto ret = QMessageBox::question(
        this,
        tr("Confirmare"),
        tr("Sigur doriți să eliminați toate rândurile?"),
        QMessageBox::Yes | QMessageBox::No
        );

    if (ret == QMessageBox::Yes)
        table->setRowCount(0);
}

void MainWindow::removeCurrentRow()
{
    int row = table->currentRow();
    if (row < 0)
        return;

    table->removeRow(row);
}

void MainWindow::autoDetectPaths1C()
{
    QString ibasesPath = QDir(QDir::homePath() + "/AppData/Roaming/1C/1CEStart")
    .filePath("ibases.v8i");

    QVector<IBASEEntry> bases =
        QVector<IBASEEntry>::fromList(IBASEParser::parse(ibasesPath));

    if (bases.isEmpty()) {
        log(tr("Nu am găsit nicio bază în ibases.v8i."));
    }

    table->setRowCount(bases.size());

    for (int i = 0; i < bases.size(); ++i)
    {
        // checkbox
        QWidget *cw    = new QWidget(this);
        QHBoxLayout *h = new QHBoxLayout(cw);
        QCheckBox *cb  = new QCheckBox(cw);
        h->addWidget(cb);
        h->setAlignment(Qt::AlignCenter);
        h->setContentsMargins(0,0,0,0);
        cw->setLayout(h);

        /** 0. checkbox */
        table->setCellWidget(i, 0, cw);

        /** 1. denumirea BD */
        auto *dbItem = new QTableWidgetItem(bases[i].displayName);
        table->setItem(i, 1, dbItem);

        /** 2. calea spre BD */
        auto *dbFilePath = new QTableWidgetItem(bases[i].filePath);
        table->setItem(i, 2, dbFilePath);

        /** 3. status */
        QTableWidgetItem *statusItem = new QTableWidgetItem("");
        statusItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 3, statusItem);

        /** metadata interna (FOARTE IMPORTANT) */
        dbItem->setData(Qt::UserRole,     "one_file"); /** typeDB -> "one_file" */
        dbItem->setData(Qt::UserRole + 1, true);       /** configured -> true or false */
        dbItem->setData(Qt::UserRole + 2, "");         /** path config JSON */
    }
}

void MainWindow::onAddedDatabaseMSSQL(const QVariantMap &dbInfo)
{
    /** --- Valorile primite */
    const QString typeDB  = dbInfo.value("typeDB").toString().trimmed();
    const QString dbName  = dbInfo.value("database").toString().trimmed();
    const QString server  = dbInfo.value("server").toString().trimmed();
    const QString cfgPath = dbInfo.value("config").toString().trimmed();
    const bool configured = dbInfo.value("configured").toBool();

    if (dbName.isEmpty() || server.isEmpty())
        return;

    /** --- Evitam duplicarea (database + server) */
    for (int r = 0; r < table->rowCount(); ++r) {
        auto *itemDb  = table->item(r, 1);
        auto *itemSrv = table->item(r, 2);
        if (!itemDb || !itemSrv)
            continue;

        if (itemDb->text() == dbName &&
            itemSrv->text() == server) {
            table->selectRow(r); /** deja exista -> selectam randul și iesim */
            return;
        }
    }

    /** --- Adaugam rand nou */
    const int row = table->rowCount();
    table->insertRow(row);

    /** --- Col 0: checkbox */
    QWidget *cw = new QWidget(table);
    auto *h     = new QHBoxLayout(cw);
    auto *cb    = new QCheckBox(cw);
    h->addWidget(cb);
    h->setAlignment(Qt::AlignCenter);
    h->setContentsMargins(0, 0, 0, 0);
    cw->setLayout(h);
    table->setCellWidget(row, 0, cw);

    /** --- Col 1: Database */
    auto *dbItem = new QTableWidgetItem(dbName);
    table->setItem(row, 1, dbItem);

    /** --- Col 2: Server */
    auto *srvItem = new QTableWidgetItem(server);
    table->setItem(row, 2, srvItem);

    /** --- Col 3: Status */
    auto *statusItem = new QTableWidgetItem(tr("Not configured"));
    statusItem->setTextAlignment(Qt::AlignCenter);
    statusItem->setForeground(Qt::darkYellow);
    table->setItem(row, 3, statusItem);

    /** --- Metadata interna (FOARTE IMPORTANT) */
    dbItem->setData(Qt::UserRole,        typeDB);     /** typeDB -> "mssql" */
    dbItem->setData(Qt::UserRole + 1,    configured); /** configured -> true or false */
    dbItem->setData(Qt::UserRole + 2,    cfgPath);    /** path config JSON */

    /** --- Selectam automat randul nou */
    table->selectRow(row);
}

void MainWindow::saveLogToFile()
{
    QFile file(backupFolder + "/log_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH.mm.ss") + ".log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << logBox->toPlainText();

    file.close();
}

void MainWindow::checkForUpdates()
{
    auto *checker = new UpdateChecker(this);

    connect(checker, &UpdateChecker::updateAvailable,
            this, [this](const QString &ver) {
                UpdateDialog dlg(ver, this);
                dlg.exec();
            });

    checker->checkForUpdates(VER);
}

// ======================================
// Actiunile submeniului pu MSSQL
// ======================================

void MainWindow::onAddMssqlDb()
{
    auto dialog_mssql = new PluginConfigDialog("mssql", QString(), this);
    connect(dialog_mssql, &PluginConfigDialog::onAddedDatabase, this,
            &MainWindow::onAddedDatabaseMSSQL, Qt::UniqueConnection);
    dialog_mssql->exec();
}

void MainWindow::onEditMssqlDb()
{
    const int row = table->currentRow();
    if (row < 0)
        return;

    auto *itemDb = table->item(row, 1);
    if (!itemDb)
        return;

    const QString configFile = itemDb->data(Qt::UserRole + 2).toString();
    if (configFile.isEmpty() || !QFile::exists(configFile)) {
        QMessageBox::warning(
            this,
            tr("MSSQL"),
            tr("Fișierul de configurare nu există."));
        return;
    }

    auto dialog_mssql = new PluginConfigDialog("mssql", configFile, this);
    dialog_mssql->exec();
}

void MainWindow::onRemoveMssqlDb()
{
    const int row = table->currentRow();
    if (row < 0)
        return;

    auto *itemDb = table->item(row, 1);
    if (!itemDb)
        return;

    const QString configFile = itemDb->data(Qt::UserRole + 2).toString();

    if (configFile.isEmpty())
        return;

    if (QMessageBox::question(
            this,
            tr("Confirmare"),
            tr("Eliminați configurarea MSSQL pentru această bază de date?"))
        != QMessageBox::Yes)
        return;

    if (QFile::exists(configFile) && !QFile::remove(configFile)) {
        QMessageBox::warning(
            this,
            tr("Eroare"),
            tr("Nu pot șterge fișierul de configurare."));
        return;
    }

    if (!QFile::exists(configFile))
        table->removeRow(row);
}

// ======================================
// Determinarea 7zip + determinarea bd 1C
// ======================================

void MainWindow::check7ZipInstallation()
{
    QString dll = QCoreApplication::applicationDirPath() + "/7z.dll";

    if (!QFile::exists(dll)) {
        QMessageBox::critical(
            this,
            tr("Componentă 7z.dll"),
            tr("7z.dll nu a fost găsit.\n"
               "Reinstalează aplicația.")
            );
        qApp->quit();
    }
}

QStringList MainWindow::find1CDBaseFolders(const QString &rootDir)
{
    QStringList result;

    QDirIterator it(rootDir,
                    QStringList() << "*.1CD",
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QFileInfo fi(it.next());
        result << QDir::toNativeSeparators(fi.absolutePath()); // folderul bd
    }

    result.removeDuplicates();
    return result;
}

// ======================================
// Nume arhivă
// ======================================

QString MainWindow::buildArchiveName(const QString &dbName) const
{
    return QDir(backupFolder).filePath(
        dbName + "_" +
        QDateTime::currentDateTime().toString("yyyy-MM-dd_HH.mm.ss") +
        ".7z");
}

QString MainWindow::buildArchiveNameMSSQL(const QString &dbName) const
{
    return QDir(backupFolder).filePath(
        dbName + "_mssql_" +
        QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") +
        ".bak");
}

void MainWindow::proceedWithArchive(BackupJob &job)
{
    //---------------------------------------------
    // ONE_FILE + DROPBOX + SHA256
    //---------------------------------------------
    /** dimensiune pentru progres */
    uint64_t totalBytes = job.archiveWholeFolder
                              ? folderSize(job.dbFolder)
                              : QFileInfo(job.file1CD).size();

    log(tr("📦 Initierea compresiei fisierului - %1").arg(toWinPath(job.file1CD)));

    /** initierea progressBar */
    progressBar->setValue(0);
    currentStatus->setText(tr("Arhivare: ..."));

    if (globals::syncDropbox) {
        progressBarDropbox->setValue(0);
        currentStatusDropbox->setText(tr("Încărcarea în Dropbox: ..."));
    }

    /** initierea spinner-lui in tabel */
    QLabel *lbl = new QLabel(this);
    lbl->setAlignment(Qt::AlignCenter);

    QMovie *mv = globals::isDark
                     ? new QMovie(":/icons/icons/spinner.gif")
                     : new QMovie(":/icons/icons/Fading balls.gif");

    mv->setScaledSize(QSize(20, 20));
    lbl->setMovie(mv);
    mv->start();

    table->setCellWidget(job.row, 3, lbl);

    QThread *t = new QThread;
    QString inputPath = job.archiveWholeFolder ? job.dbFolder : job.file1CD;

    auto *worker = new CompressWorker(
        inputPath,
        job.archivePath,
        comboCompression->currentIndex(),
        job.archiveWholeFolder,
        totalBytes,
        globals::archivePassword
        );

    worker->moveToThread(t);

    connect(t, &QThread::started, worker, &CompressWorker::process);

    connect(worker, &CompressWorker::progress, this,
            [this](int pct) {
                progressBar->setValue(pct);
                currentStatus->setText(
                    QString(tr("Progres: %1%")).arg(pct)
                    );
            });

    /** BACKUP FINALIZAT (doar arhivare) */
    connect(worker, &CompressWorker::finished, this,
            [=](bool ok, const QString &, const QString &err) {

                mv->stop();
                mv->deleteLater();
                lbl->deleteLater();

                table->removeCellWidget(job.row, 3);

                auto *it = new QTableWidgetItem(ok ? "✔" : "❌");
                it->setTextAlignment(Qt::AlignCenter);
                table->setItem(job.row, 3, it);

                log(ok
                        ? tr("✔ Backup finalizat")
                        : tr("❌ Backup eșuat: %1").arg(err));

                t->quit();
                if (!ok)
                    QMetaObject::invokeMethod(this, "startNextJob", Qt::QueuedConnection);
            });

    /** ARHIVA CREATA -> SHA -> DROPBOX */
    connect(worker, &CompressWorker::backupCreated, this,
            [this, &job](const QString &archivePath) {

                QFileInfo info(archivePath);
                double sizeMB = info.size() / 1024.0 / 1024.0;

                log(QString(tr("📦 Arhivă: %1 (%2 MB)"))
                        .arg(toWinPath(archivePath),
                             QString::number(sizeMB, 'f', 1)));

                /** eliminam fisierul .bak */
                if (globals::pl_mssql &&
                    ! job.fileBak.isEmpty() &&
                    QFile(job.archivePath).exists()) {
                    if (!QFile::remove(job.file1CD)) {
                        log(tr("⚠ Nu pot șterge fișierul: %1")
                                .arg(job.file1CD));
                    } else {
                        log(tr("✔ Eliminat fișierul: %1")
                                .arg(job.file1CD));
                        job.fileBak.clear();     /** IMPORTANT - eliminam path-ul catre fisier .bak */
                    }
                }

                /** crearea fisierului .sha256 */
                if (globals::createFileSHA256)
                    createSha256File(archivePath);

                /** activam sincronizarea cu Dropbox */
                if (globals::syncDropbox && globals::activate_syncDropbox) {
                    m_waitingForDropbox = true;

                    if (globals::createFileSHA256)
                        startDropboxUpload(
                            archivePath,
                            archivePath + ".sha256");
                    else
                        startDropboxUpload(archivePath);
                } else {
                    /** urmatorul job */
                    startNextJob();
                }
            });

    connect(t, &QThread::finished, worker, &QObject::deleteLater);
    connect(t, &QThread::finished, t, &QObject::deleteLater);

    t->start();
}

void MainWindow::proceedWithArchiveMssql(BackupJob &job)
{
    /** refolosim progresBar */
    progressBar->setValue(0);
    currentStatus->setText(tr("Backup MSSQL: ..."));

    /** spinner tabelei */
    QLabel *lbl = new QLabel(this);
    lbl->setAlignment(Qt::AlignCenter);

    QMovie *mv = globals::isDark
                     ? new QMovie(":/icons/icons/spinner.gif")
                     : new QMovie(":/icons/icons/Fading balls.gif");

    mv->setScaledSize(QSize(20, 20));
    lbl->setMovie(mv);
    mv->start();

    table->setCellWidget(job.row, 3, lbl);

    // ---------- Worker MSSQL ----------
    QThread *t = new QThread(this);
    WorkerMssql *worker = new WorkerMssql;

    worker->moveToThread(t);

    worker->setConfigFile(job.configPath);

    /** fisier .bak */
    QString bakPath = buildArchiveNameMSSQL(job.dbName);

    worker->setOutputBak(bakPath);

    connect(t, &QThread::started,
            worker, &WorkerMssql::process);

    connect(worker, &WorkerMssql::log,
            this, &MainWindow::log);

    connect(worker, &WorkerMssql::progress,
            progressBar, &QProgressBar::setValue);

    connect(worker, &WorkerMssql::finished,
            this,
            [=, &job](bool ok, const QString &err) {

                mv->stop();
                mv->deleteLater();
                lbl->deleteLater();
                table->removeCellWidget(job.row, 3);

                t->quit();

                if (!ok) {
                    auto *it = new QTableWidgetItem("❌");
                    it->setTextAlignment(Qt::AlignCenter);
                    table->setItem(job.row, 3, it);

                    log(tr("❌ Backup MSSQL eșuat: ") + err);
                    startNextJob();
                    return;
                }

                log(tr("✔ MSSQL backup finalizat, creat - %1")
                        .arg(toWinPath(bakPath)));

                if (!waitForFileReady(bakPath)) {
                    log(tr("❌ Fișierul .bak este blocat de MSSQL"));
                    startNextJob();
                    return;
                }

                /** transformam job-ul în ONE_FILE (refolosim functia fara multe complice) */
                job.typeDB = "one_file";                         /** tipul BD */
                job.file1CD = bakPath;                           /** file INPUT */
                job.fileBak = bakPath;                           /** IMPORTANT - file .bak pu eliminarea */
                job.archiveWholeFolder = false;                  /** arhiva cu fisierele externe */
                job.archivePath = buildArchiveName(job.dbName);  /** file OUTPUT */

                /** intram in pipeline-ul normal */
                proceedWithArchive(job);

            });

    connect(t, &QThread::finished,
            worker, &QObject::deleteLater);
    connect(t, &QThread::finished,
            t, &QObject::deleteLater);

    t->start();
}

// ======================================
// Pornește job
// ======================================

void MainWindow::startNextJob()
{
    /** daca asteptam Dropbox -> NU pornim alt job */
    if (m_waitingForDropbox)
        return;

    currentJob++;

    /** finalizarea job-lui */
    if (currentJob >= jobs.size()) {
        log("----------------------------------------------------------------------------");
        log(tr("Toate backup-urile finalizate."));
        currentStatus->setText("Gata.");
        progressBar->setValue(100);

        // Reactivarea UI button
        btnSelectAll->setEnabled(true);
        btnArchive->setEnabled(true);
        btnFolder->setEnabled(true);
        comboCompression->setEnabled(true);

        // eliminam arhive vechi
        if (globals::deleteArchives) {
            cleanupOldArchives();
        }

        // inscrim logul in fisier daca e "--autorun"
        if (globals::isAutorun)
            saveLogToFile();

        emit allJobsFinished(); // pu "--autorun" vezi in main.cpp

        return;
    }

    BackupJob &job = jobs[currentJob];

    log("----------------------------------------------------------------------------");
    log(tr("Arhivez: ") + toWinPath(job.file1CD));
    log("📦 " + job.dbName);

    //---------------------------------------------
    // MSSQL
    //---------------------------------------------
    if (job.typeDB == "mssql") {
        proceedWithArchiveMssql(job);
    } else if (job.typeDB == "one_file") {
        proceedWithArchive(job);
    }
}

// ======================================
// Setează icon status manual
// ======================================

void MainWindow::updateRowStatusIcon(int row, bool ok)
{
    table->removeCellWidget(row, 3);
    QTableWidgetItem *it = new QTableWidgetItem(ok ? "✔" : "❌");
    it->setTextAlignment(Qt::AlignCenter);
    table->setItem(row, 3, it);
}

// ======================================
// Detectare 7-Zip
// ======================================

QString MainWindow::get7zPath() const
{
#if defined(Q_OS_WIN)
    QStringList candidates = {
        "7z.exe",
        "7za.exe",
        "7zz.exe",
        "C:/Program Files/7-Zip/7z.exe",
        "C:/Program Files/7-Zip/7za.exe",
        "C:/Program Files/7-Zip/7zz.exe",
        "C:/Program Files (x86)/7-Zip/7z.exe",
        "C:/Program Files (x86)/7-Zip/7za.exe",
        "C:/Program Files (x86)/7-Zip/7zz.exe"
    };
#else
    QStringList candidates = { "7z", "7za", "7zz" };
#endif

    for (const QString& c : candidates) {
        QString p = QStandardPaths::findExecutable(c);
        if (!p.isEmpty()) {
            return p;  /** gasit in PATH */
        }

        if (QFile::exists(c)) {
            return c;  /** gasit ca fisier hardcoded */
        }
    }

    return QString();  /** nu a fost gasit */
}

void MainWindow::startDropboxUpload(const QString &localPath, const QString &fileSHA256)
{
    progressBarDropbox->setRange(0, 100);
    progressBarDropbox->setValue(0);
    currentStatusDropbox->setText(tr("🌍 Dropbox: inițierea încărcării..."));

    ConnectorDropbox conn;
    const QString access  = conn.accessToken();
    const QString refresh = conn.refreshToken();

    if (access.isEmpty() || refresh.isEmpty()) {
        log(tr("⛔ Dropbox: nu este conectat, încărcarea omisă."));
        m_waitingForDropbox = false;
        startNextJob();
        return;
    }

    m_dbxUploader = new DropboxUploader(access, refresh, this);

    const QString remotePath =
        "/" + QFileInfo(localPath).fileName();

    connect(m_dbxUploader, &DropboxUploader::uploadProgress,
            this, [this](qint64 sent, qint64 total) {

                if (total <= 0)
                    return;

                int percent = int(double(sent) / double(total) * 100.0);
                percent = qBound(0, percent, 100);

                progressBarDropbox->setValue(percent);

                currentStatusDropbox->setText(
                    QString(tr("Încărcarea în Dropbox: %1% (%2 / %3 MB))"))
                        .arg(QString::number(percent),
                             QString::number(sent  / 1024.0 / 1024.0, 'f', 1),
                             QString::number(total / 1024.0 / 1024.0, 'f', 1))
                    );
            });

    connect(m_dbxUploader, &DropboxUploader::uploadFinished,
            this, [this, localPath, fileSHA256](bool ok, const QString &msg) {

                if (!ok) {
                    log(tr("⛔ Dropbox: încărcarea nereușită: ") + msg);
                }
                else {
                    log(tr("🌍 Dropbox: încărcarea finalizată: ")
                        + QFileInfo(localPath).fileName());

                    /** upload SHA256 DUPĂ .7z */
                    if (!fileSHA256.isEmpty() &&
                        QFile::exists(fileSHA256)) {

                        log(tr("🌍 Dropbox: inițirea încărcării fișierului SHA256..."));

                        QMetaObject::invokeMethod(
                            this,
                            [this, fileSHA256]() {
                                startDropboxUpload(fileSHA256);
                            },
                            Qt::QueuedConnection
                            );
                        return;
                    }

                    /** FINAL JOB */
                    log(tr("✔ Arhivarea și încărcarea în Dropbox reușită"));
                }

                m_dbxUploader->deleteLater();
                m_dbxUploader = nullptr;

                m_waitingForDropbox = false;
                startNextJob();   /** urmatorul job */
            });

    connect(m_dbxUploader, &DropboxUploader::authError,
            this, [this](const QString &msg)
            {
                log(msg);
                currentStatusDropbox->setText(tr("Dropbox: este necesar autorizare"));
                progressBarDropbox->setValue(0);
                globals::syncDropbox = false;          /** dezactiveaza sincronizarea */
                globals::activate_syncDropbox = false;
                log(tr("Dropbox sync disabled. Please reconnect."));
            });

    log(tr("🌍 Dropbox: start încărcarea: ") + toWinPath(localPath));
    m_dbxUploader->uploadFile(localPath, remotePath);
}

bool MainWindow::createSha256File(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        log(tr("❌ Nu pot deschide fișierul pentru SHA-256: %1")
                .arg(toWinPath(filePath)));
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    /** Citim fisierul in blocuri mari */
    while (!file.atEnd()) {
        hash.addData(file.read(1024 * 1024));  // 1 MB
    }

    QByteArray sha = hash.result().toHex();
    file.close();

    /** Construim calea fisierului .sha256 */
    QString shaFile = filePath + ".sha256";
    QFile out(shaFile);

    if (!out.open(QFile::WriteOnly | QFile::Truncate)) {
        log(tr("❌ Nu pot crea fișierul SHA-256: %1").arg(toWinPath(shaFile)));
        return false;
    }

    /** Format standard: sha256 + spatiu + * + nume fisier */
    QString line = QString("%1 *%2\n")
                       .arg(QString::fromLatin1(sha),
                            QFileInfo(filePath).fileName());

    out.write(line.toUtf8());
    out.close();

    /** logarea */
    log(tr("🔐 Creat fișier SHA-256: %1").arg(toWinPath(shaFile)));
    return true;
}

void MainWindow::setPropertyVisible()
{
    currentStatusDropbox->setVisible(globals::syncDropbox);
    progressBarDropbox->setVisible(globals::syncDropbox);
    btnAbortDropbox->setVisible(globals::syncDropbox);
}

static QCheckBox* checkboxAt(QTableWidget *table, int row, int col)
{
    QWidget *w = table->cellWidget(row, col);
    if (!w) return nullptr;

    return w->findChild<QCheckBox*>();
}

QWidget *MainWindow::createCheckBoxWidget(QWidget *parent)
{
    QWidget *cw = new QWidget(parent);

    QHBoxLayout *h = new QHBoxLayout(cw);
    QCheckBox *cb = new QCheckBox(cw);

    h->addWidget(cb);
    h->setAlignment(Qt::AlignCenter);
    h->setContentsMargins(0, 0, 0, 0);

    cw->setLayout(h);
    return cw;
}

// ======================================
// Setarile -> load & save
// ======================================

void MainWindow::loadSettings()
{
    QFile f(settingsFilePath);
    if (!f.exists()) {

        /** --- presupunem ca prima lansare */
        /** limba implicita a aplicatiei */
        globals::currentLang = "app_ru_RU";
        lblSwitch->setChecked(globals::currentLang == "app_ro_RO");
        switchLanguage(globals::currentLang);
        retranslateUi();

        /** tema implicita a aplicatiei */
        globals::isDark = false;
        themeSwitch->setChecked(globals::isDark);
        applyTheme();

        /** compresia implicita */
        comboCompression->setCurrentIndex(5);

        /** setam latimea sectiilor */
        table->setColumnWidth(0, 62);
        table->setColumnWidth(1, 168);
        table->setColumnWidth(2, 488);

        /** ascundem elementele status si progresbar Dropbox */
        setPropertyVisible();

        return;
    }

    if (!f.open(QIODevice::ReadOnly))
        return;

    QByteArray raw = f.readAll();
    f.close();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return;

    QJsonObject obj = doc.object();

    PluginManager plugin_manager;
    plugin_manager.load();

    if (globals::pl_mssql) {
        createSubmenuMSSQL();
    }

    //---------------------------------------------------------------------------

    if (obj.contains("currentLang")) {
        globals::currentLang = obj["currentLang"].toString();
        lblSwitch->setChecked(globals::currentLang == "app_ro_RO");
        switchLanguage(globals::currentLang);
    }

    if (obj.contains("darkTheme")) {
        globals::isDark = obj["darkTheme"].toBool();
        themeSwitch->setChecked(globals::isDark);
        applyTheme();
    }

    if (obj.contains("compression"))
        comboCompression->setCurrentIndex(obj["compression"].toInt());

    if (obj.contains("backupFolder")) {
        backupFolder = obj["backupFolder"].toString();
        log(tr("✔ Folder backup încărcat din setări: ") + toWinPath(backupFolder));
    }

    if (obj.contains("backupExtFiles"))
        globals::backupExtFiles = obj["backupExtFiles"].toBool();

    if (obj.contains("createFileSHA256"))
        globals::createFileSHA256 = obj["createFileSHA256"].toBool();

    if (obj.contains("syncDropbox")) {
        globals::syncDropbox = obj["syncDropbox"].toBool();
        if (obj.contains("activate_syncDropbox"))
            globals::activate_syncDropbox = obj["activate_syncDropbox"].toBool();
        setPropertyVisible();
        checkDropboxAtStartup();
    }

    if (obj.contains("syncGoogleDrive"))
        globals::syncGoogleDrive = obj["syncGoogleDrive"].toBool();

    if (obj.contains("activate_syncGoogleDrive"))
        globals::activate_syncGoogleDrive = obj["activate_syncGoogleDrive"].toBool();

    if (obj.contains("questionCloseApp"))
        globals::questionCloseApp = obj["questionCloseApp"].toBool();

    if (obj.contains("setArchivePassword"))
        globals::setArchivePassword = obj["setArchivePassword"].toBool();

    if (obj.contains("archivePassword"))
        globals::archivePassword = decryptPassword(obj.value("archivePassword").toString());

    if (obj.contains("succ_dropbox"))
        globals::loginSuccesDropbox = obj["succ_dropbox"].toString();

    if (obj.contains("succ_gdrive"))
        globals::loginSuccesGoogleDrive = obj["succ_gdrive"].toString();

    if (obj.contains("paths_db")) {

        if (!obj["paths_db"].isArray())
            return;

        QJsonArray arr_db = obj["paths_db"].toArray();

        table->setRowCount(0); // curățăm tabelul

        for (const QJsonValue &val : std::as_const(arr_db)) {

            if (!val.isObject())
                continue;

            QJsonObject o = val.toObject();

            const bool mark    = o.value("mark").toBool(false);
            const QString name = o.value("name").toString();
            const QString path = o.value("path").toString();

            /** determinam variabile pu metadata interna */
            const QString typeDB =
                o.contains("typeDB")
                    ? o.value("typeDB").toString()
                    : QStringLiteral("one_file");

            const bool configured =
                o.contains("configured")
                    ? o.value("configured").toBool()
                    : true; // v1.7 era mereu configurat

            const QString configPath =
                o.value("configPath").toString(); // poate lipsi -> ""

            int row = table->rowCount();
            table->insertRow(row);

            /** 0. checkbox */
            QWidget *cw   = createCheckBoxWidget(table);
            QCheckBox *cb = cw->findChild<QCheckBox*>();
            if (cb)
                cb->setChecked(mark);

            table->setCellWidget(row, 0, cw);

            /** 1. nume BD */
            auto *dbItem = new QTableWidgetItem(name);
            table->setItem(row, 1, dbItem);

            /** 2. path BD */
            auto *dbPath = new QTableWidgetItem(path);
            table->setItem(row, 2, dbPath);

            /** 3. status */
            QTableWidgetItem *statusItem = new QTableWidgetItem("");
            statusItem->setTextAlignment(Qt::AlignCenter);
            table->setItem(row, 3, statusItem);

            /** restaurarea metadata interna -> IMPORTANT */
            dbItem->setData(Qt::UserRole,     typeDB);
            dbItem->setData(Qt::UserRole + 1, configured);
            dbItem->setData(Qt::UserRole + 2, configPath);

            log(tr("✔ Baza de date '%1' încărcată din setări.")
                    .arg(path));
        }
    }

    // ------------------------------------------
    // Restaurăm dimensiunile coloanelor tabelului
    // ------------------------------------------
    if (obj.contains("columnWidths") && obj["columnWidths"].isArray()) {
        QJsonArray arr = obj["columnWidths"].toArray();

        for (int c = 0; c < arr.size() && c < table->columnCount(); ++c) {
            int w = arr[c].toInt();
            if (w > 20)   // minim de siguranță
                table->setColumnWidth(c, w);
        }
    }

    if (obj.contains("deleteArchives"))
        globals::deleteArchives = obj["deleteArchives"].toBool();

    if (obj.contains("lastNrDay"))
        globals::lastNrDay = obj["lastNrDay"].toString().toInt();
}

void MainWindow::saveSettings()
{
    QJsonObject obj;

    // ----------------------------------------
    // Salvam plugin-ri
    // ----------------------------------------
    QJsonArray arr_plugins;
    QJsonObject obj_plugin;
    obj_plugin["mssql"]    = globals::pl_mssql;
    obj_plugin["rsync"]    = globals::pl_rsync;
    obj_plugin["onedrive"] = globals::pl_onedrive;
    arr_plugins.append(obj_plugin);

    obj["plugins"] = arr_plugins;

    // ----------------------------------------
    // Salvam date generale
    // ----------------------------------------
    obj["compression"]              = comboCompression->currentIndex();
    obj["backupFolder"]             = backupFolder;
    obj["currentLang"]              = currentLang;
    obj["darkTheme"]                = themeSwitch->isChecked();
    obj["backupExtFiles"]           = globals::backupExtFiles;
    obj["createFileSHA256"]         = globals::createFileSHA256;
    obj["questionCloseApp"]         = globals::questionCloseApp;
    obj["setArchivePassword"]       = globals::setArchivePassword;
    obj["archivePassword"]          = encryptPassword(globals::archivePassword);
    obj["syncDropbox"]              = globals::syncDropbox;
    obj["activate_syncDropbox"]     = globals::activate_syncDropbox;
    obj["syncGoogleDrive"]          = globals::syncGoogleDrive;
    obj["activate_syncGoogleDrive"] = globals::activate_syncGoogleDrive;
    obj["succ_dropbox"]             = globals::loginSuccesDropbox;
    obj["succ_gdrive"]              = globals::loginSuccesGoogleDrive;
    obj["deleteArchives"]           = globals::deleteArchives;
    obj["lastNrDay"]                = QString::number(globals::lastNrDay);

    // ------------------------------------------
    // Salvăm dimensiunile coloanelor tabelului
    // ------------------------------------------
    QJsonArray arr;
    for (int c = 0; c < table->columnCount(); ++c)
        arr.append(table->columnWidth(c));
    obj["columnWidths"] = arr;

    // ------------------------------------------
    // Salvăm baze de date marcate si nemarcate
    // ------------------------------------------
    QJsonArray arr_db;
    for (int i = 0; i < table->rowCount(); ++i) {

        QCheckBox *cb = checkboxAt(table, i, 0);
        auto *dbItem  = table->item(i, 1);
        auto *pathItem = table->item(i, 2);

        QJsonObject obj;
        obj["mark"] = cb && cb->isChecked();
        obj["name"] = dbItem->text();
        obj["path"] = pathItem->text();

        // metedata interna - e Important !!!
        obj["typeDB"] =
            dbItem->data(Qt::UserRole).toString();

        obj["configured"] =
            dbItem->data(Qt::UserRole + 1).toBool();

        obj["configPath"] =
            dbItem->data(Qt::UserRole + 2).toString();

        arr_db.append(obj);
    }
    obj["paths_db"] = arr_db;

    // ------------------------------------------
    // Salvăm in fisierul de satari .json
    // ------------------------------------------
    QFile f(settingsFilePath);
    if (f.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(obj);
        f.write(doc.toJson());
        f.close();
    }

}

// ======================================
// Retranslarea și evenimentele închiderii
// ======================================

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("1CArchiver v%1 – Backup al bazelor de date 1C:Enterprise")
                       .arg(VER));

    btnSelectAll->setText(tr("Selectează toate"));
    btnWithDb->setText(tr("Directoriu cu BD"));
    btnFolder->setText(tr("Alege folder backup"));
    btnArchive->setText(tr("Arhivează selectatele"));
    btnAbortDropbox->setText(tr("Oprește Dropbox"));
    btnGenerateTask->setText(tr("Generează Task XML"));

    btnAbout->setToolTip(tr("Despre aplicația"));
    btnSettings->setToolTip(tr("Setările aplicației"));
    btnSelectAll->setToolTip(tr("Selectează toate <br>"
                                "bazele de date din tabel"));
    btnWithDb->setToolTip(tr("Calea spre directoriu,<br>"
                             "unde sunt baze de date 1C"));
    btnFolder->setToolTip(tr("Calea spre directoriu,<br>"
                             "unde v-a fi păstrate arhive"));
    btnArchive->setToolTip(tr("Lansarea arhivării/sincronizării"));
    btnGenerateTask->setToolTip(tr("Generarea task"));

    themeLabel->setText(tr("Dark theme:"));
    lblLang->setText(tr("Limba RO:"));
    lblCompression->setText(tr("Compresie:"));

    currentStatus->setText(tr("Arhivarea: ... inactiv"));

    table->setHorizontalHeaderLabels({
        tr("Select"),
        tr("Denumire BD"),
        tr("Cale"),
        tr("Status")
    });
}

void MainWindow::checkDropboxAtStartup()
{
    /** verificam variabile globale daca active */
    if (!globals::syncDropbox || !globals::activate_syncDropbox)
        return;

    QSettings s("Oxvalprim", "1CArchiver");
    const QString access  = s.value("dropbox/access_token").toString();
    const QString refresh = s.value("dropbox/refresh_token").toString();

    /** Fără refresh token */
    if (refresh.isEmpty()) {
        setDropboxAuthRequired();
        return;
    }

    auto *checker = new DropboxHealthChecker(this);

    /** TOKEN OK */
    connect(checker, &DropboxHealthChecker::connected,
            this, [this]() {
                setDropboxConnected();
            });

    /** TOKEN EXPIRAT → încercăm refresh, NU afișăm eroare încă */
    connect(checker, &DropboxHealthChecker::authorizationRequired,
            this, [this, refresh]() {

                auto *oauth = new DropboxOAuth2_PKCE(this);

                connect(oauth, &DropboxOAuth2_PKCE::refreshSucceeded,
                        this, [this, oauth]() {
                            oauth->deleteLater();

                            QSettings s("Oxvalprim", "1CArchiver");
                            const QString newAccess =
                                s.value("dropbox/access_token").toString();

                            if (newAccess.isEmpty()) {
                                setDropboxAuthRequired();
                                return;
                            }

                            /** retry health-check pe token NOU */
                            auto *checker2 = new DropboxHealthChecker(this);

                            /** msg pu conectarea reusita */
                            connect(checker2, &DropboxHealthChecker::connected,
                                    this, [this]() {
                                        setDropboxConnected();
                                    });

                            /** msg pu necesitatea autorizarii */
                            connect(checker2, &DropboxHealthChecker::authorizationRequired,
                                    this, [this]() {
                                        setDropboxAuthRequired();
                                    });

                            /** pornim verificarea conectarii la Dropbox */
                            checker2->check(newAccess);
                        });

                connect(oauth, &DropboxOAuth2_PKCE::refreshFailed,
                        this, [this, oauth](const QString &) {
                            oauth->deleteLater();
                            setDropboxAuthRequired();       /** refresh a eșuat */
                        });

                oauth->refreshAccessToken(); /** refresh token Dropbox */
            });

    /** pornim verificarea conectarii la Dropbox */
    checker->check(access);
}

void MainWindow::setDropboxConnected()
{
    currentStatusDropbox->setText(tr("Dropbox: conectat (...așteptare)"));
    globals::syncDropbox = true;
}

void MainWindow::setDropboxAuthRequired()
{
    currentStatusDropbox->setText(tr("Dropbox: este necesar autorizarea"));
    globals::syncDropbox = false;
    globals::activate_syncDropbox = false;
}

void MainWindow::cleanupOldArchives()
{
    QDir dir(backupFolder);
    if (!dir.exists())
        return;

    /** determinam data limita */
    const QDateTime limit =
        QDateTime::currentDateTime().addDays(-globals::lastNrDay);

    /** filtru pu fisiere */
    const QStringList filters =
    {
        "*.7z",
        "*.zip",
        "*.sha256",
        "*.log",
        "*.txt"
    };

    /** bucla pu depistarea fisierelor */
    for (const QFileInfo &fi :
         dir.entryInfoList(filters, QDir::Files)) {

        if (fi.lastModified() < limit) {
            QFile::remove(fi.absoluteFilePath());
            log(tr("🗑 Șters: %1")
                    .arg(toWinPath(fi.absoluteFilePath())));
        }
    }
}

void MainWindow::createSubmenuMSSQL()
{
    menuAddDb = new QMenu(this);

    QAction *actAddFile  = menuAddDb->addAction(tr("➕ 1C File Database"));
    QAction *actAddMssql = menuAddDb->addAction(tr("➕ MSSQL Database"));

    menuAddDb->addSeparator();

    QAction *actEditMssql   = menuAddDb->addAction(tr("✏ Edit MSSQL config"));
    QAction *actRemoveMssql = menuAddDb->addAction(tr("🗑 Remove MSSQL config"));

    btnWithDb->setMenu(menuAddDb);

    connect(actAddFile,   &QAction::triggered,
            this, &MainWindow::onChooseDirWithDb);

    connect(actAddMssql, &QAction::triggered,
            this, &MainWindow::onAddMssqlDb);

    connect(actEditMssql, &QAction::triggered,
            this, &MainWindow::onEditMssqlDb);

    connect(actRemoveMssql, &QAction::triggered,
            this, &MainWindow::onRemoveMssqlDb);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    if (m_checkedUpdates)
        return;

    m_checkedUpdates = true;
    if (!globals::isAutorun) /** daca autorun nu verificam versiunea noua */
        checkForUpdates();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /** Dacă nu cerem confirmare → ieșim direct */
    if (!globals::questionCloseApp) {
        saveSettings();
        event->accept();
        return;
    }

    QMessageBox msg(
        QMessageBox::Question,
        tr("Finisarea lucrului"),
        tr("Doriți să închideți aplicația?"),
        QMessageBox::NoButton,
        this
        );

    QPushButton* yesButton = msg.addButton(tr("Da"), QMessageBox::YesRole);
    QPushButton* noButton  = msg.addButton(tr("Nu"), QMessageBox::NoRole);

    msg.exec();

    if (msg.clickedButton() == yesButton) {
        saveSettings();
        event->accept();
    } else if (msg.clickedButton() == noButton){
        event->ignore();
    }
}
