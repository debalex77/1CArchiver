#include "mainwindow.h"
#include "ibaseparser.h"
#include "compressworker.h"
#include "switchbutton.h"
#include "src/version.h"
#include "src/globals.h"
#include "src/utils.h"

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

// ======================================
// Constructor
// ======================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_lib(nullptr),
    m_compressor(nullptr)
{
    resize(900, 600);
    setWindowIcon(QIcon(":/icons/icons/backup.png"));
    setWindowTitle(tr("1CArchiver v%1 – Backup al bazelor de date 1C:Enterprise")
                       .arg(VER));

    // ---------------------------------------------------------
    // Tema si limba aplicatiei
    // ---------------------------------------------------------

    QHBoxLayout* topBar = new QHBoxLayout;
    topBar->setContentsMargins(0,0,0,0);
    topBar->setSpacing(10);

    //--- btn setari
    btnSettings = new QToolButton(this);
    btnSettings->setIcon(QIcon(":/icons/icons/settings.png"));
    connect(btnSettings, &QToolButton::clicked, this, [&](){
        app_settings = new AppSettings(this);
        app_settings->setAttribute(Qt::WA_DeleteOnClose);
        app_settings->show();
    });

    //--- tema
    themeLabel = new QLabel(this);
    // themeLabel->setText(tr("Dark theme:"));

    themeSwitch = new SwitchButton(this);
    themeSwitch->setChecked(globals::isDark);

    connect(themeSwitch, &SwitchButton::toggled, this, [&](bool on) {
        globals::isDark = on;
        applyTheme();
    });

    //--- limba
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

    // ---------------------------------------------------------
    // central widget
    // ---------------------------------------------------------

    QWidget *w = new QWidget(this);
    setCentralWidget(w);
    QVBoxLayout *v = new QVBoxLayout(w);

    v->addLayout(topBar);

    // ---------------------------------------------------------
    // drumul spre setarile aplicatiei
    // ---------------------------------------------------------

    settingsFilePath = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
                           .filePath("settings.json");
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    // ---------------------------------------------------------
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
    v->addWidget(table);

    // ---------------------------------------------------------
    // UI – butoane
    // ---------------------------------------------------------
    QHBoxLayout *btns = new QHBoxLayout;

    btnSelectAll    = new QPushButton(tr("Selectează toate"));
    btnFolder       = new QPushButton(tr("Alege folder backup"));
    btnArchive      = new QPushButton(tr("Arhivează selectatele"));
    // btnGenerateTask = new QPushButton(tr("Generează Task XML"));

    btns->addWidget(btnSelectAll);
    btns->addWidget(btnFolder);
    btns->addWidget(btnArchive);
    btns->addStretch();
    // btns->addWidget(btnGenerateTask);

    // ---------------------------------------------------------
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

    // ---------------------------------------------------------
    // UI – progres + status
    // ---------------------------------------------------------
    currentStatus = new QLabel("Idle", this);
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    v->addWidget(currentStatus);
    v->addWidget(progressBar);

    // ---------------------------------------------------------
    // UI – log
    // ---------------------------------------------------------
    logBox = new QTextEdit(this);
    logBox->setReadOnly(true);
    v->addWidget(logBox);

    // ---------------------------------------------------------
    // Conectări
    // ---------------------------------------------------------
    connect(btnSelectAll, &QPushButton::clicked, this, &MainWindow::onSelectAll);
    connect(btnFolder,    &QPushButton::clicked, this, &MainWindow::onChooseBackupFolder);
    connect(btnArchive,   &QPushButton::clicked, this, &MainWindow::onStartArchive);

    // ---------------------------------------------------------
    // Init folder backup
    // ---------------------------------------------------------
    backupFolder = QDir::homePath() + "/Backups_1C";
    QDir().mkpath(backupFolder);

    // ---------------------------------------------------------
    // Incarcam datele din .json
    // ---------------------------------------------------------
    loadSettings();

    // ---------------------------------------------------------
    // Verificam instalarea 7-zip
    // ---------------------------------------------------------
    check7ZipInstallation();

    // ---------------------------------------------------------
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

    // ---------------------------------------------------------
    // CITIRE ibases.v8i
    // ---------------------------------------------------------
    QString ibasesPath = QDir(QDir::homePath() + "/AppData/Roaming/1C/1CEStart")
                             .filePath("ibases.v8i");

    bases = IBASEParser::parse(ibasesPath);

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

        table->setCellWidget(i, 0, cw);
        table->setItem(i, 1, new QTableWidgetItem(bases[i].displayName));
        table->setItem(i, 2, new QTableWidgetItem(bases[i].filePath));

        QTableWidgetItem *statusItem = new QTableWidgetItem("");
        statusItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 3, statusItem);
    }


    // ---------------------------------------------------------
    // bottom bar
    // ---------------------------------------------------------

    auto* footerLayout = new QHBoxLayout;
    footerLayout->setContentsMargins(0, 10, 0, 5);
    footerLayout->setAlignment(Qt::AlignCenter);

    auto* infoOrganization = new QLabel(this);
    infoOrganization->setStyleSheet("color: #888; font-size: 11px;");
    infoOrganization->setText(QString(ORGANIZATION) + ", " + QString::number(QDate::currentDate().year()));
    footerLayout->addWidget(infoOrganization);

    v->addLayout(footerLayout);

    connect(this, &MainWindow::jobFinishedSignal, this, &MainWindow::startNextJob);

    enableDarkTitlebar(this);

}

// ======================================
// Destructor
// ======================================
MainWindow::~MainWindow()
{
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
    jobs.clear();
    currentJob = -1;

    for (int i = 0; i < table->rowCount(); ++i)
    {
        QCheckBox *cb = table->cellWidget(i,0)->findChild<QCheckBox *>();
        if (!cb || !cb->isChecked())
            continue;

        QString folder = table->item(i,2)->text();
        QString file1cd = QDir(folder).filePath("1Cv8.1CD"); //QString file1cd = folder + "/1Cv8.1CD";

        QFileInfo f(file1cd);
        if (!f.exists()) {
            log(tr("Nu găsesc 1Cv8.1CD în: ") + folder);
            continue;
        }

        BackupJob j;
        j.row         = i;
        j.dbName      = table->item(i,1)->text();
        j.dbFolder    = folder;
        j.file1CD     = file1cd;
        j.archivePath = buildArchiveName(j.dbName);
        j.spinner     = nullptr;
        j.statusLabel = nullptr;
        j.archiveWholeFolder = globals::backupExtFiles;

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
        retranslateUi();            // forțează textul implicit
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

void MainWindow::check7ZipInstallation()
{
    QString exe = get7zPath();

    if (exe.isEmpty()) {
        QMessageBox::critical(
            this, tr("7-Zip lipsă"),
            tr("Nu am găsit executabilul 7z.exe.\n"
            "Instalează 7-Zip de pe https://www.7-zip.org/")
            );
        return;
    }

    QString dll = QFileInfo(exe).absolutePath() + "/7z.dll";
    if (!QFile::exists(dll)) {
        QMessageBox::critical(
            this, tr("7-Zip DLL lipsă"),
            tr("Am găsit 7z.exe dar nu și 7z.dll în același folder.\n"
               "bit7z NU va putea comprima arhive fără această bibliotecă.")
            );
        return;
    }

    log(tr("Determinat 7-zip: ") + toWinPath(exe));
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


// ======================================
// Pornește job
// ======================================
void MainWindow::startNextJob()
{
    currentJob++;

    if (currentJob >= jobs.size()) {
        log("----------------------------------------------------------------------------");
        log(tr("Toate backup-urile finalizate."));
        currentStatus->setText("Gata.");
        progressBar->setValue(100);
        return;
    }

    BackupJob &job = jobs[currentJob];

    log("----------------------------------------------------------------------------");
    log(tr("Arhivez: ") + toWinPath(job.file1CD));
    log("📦 " + job.dbName);

    // ----------------------------------------------------------
    // CALCUL DIMENSIUNE TOTALĂ pentru progres
    // ----------------------------------------------------------
    uint64_t totalBytes = 0;

    if (job.archiveWholeFolder) {
        totalBytes = folderSize(job.dbFolder);
    } else {
        QFileInfo fi(job.file1CD);
        totalBytes = fi.size();
    }

    // UI setup
    progressBar->setValue(0);
    currentStatus->setText("Arhivare...");

    QLabel* lbl = new QLabel(this);
    lbl->setAlignment(Qt::AlignCenter);

    QMovie* mv;
    if (globals::isDark)
        mv = new QMovie(":/icons/icons/spinner.gif");
    else
        mv = new QMovie(":/icons/icons/Fading balls.gif");
    mv->setScaledSize(QSize(20,20));
    lbl->setMovie(mv);
    mv->start();

    table->setCellWidget(job.row, 3, lbl);

    // THREAD
    QThread* t   = new QThread;
    QString inputPath = job.archiveWholeFolder ? job.dbFolder : job.file1CD;
    auto* worker = new CompressWorker(inputPath,
                                      job.archivePath,
                                      comboCompression->currentIndex(),
                                      job.archiveWholeFolder,
                                      totalBytes,
                                      globals::archivePassword);
    worker->moveToThread(t);

    connect(t, &QThread::started, worker, &CompressWorker::process);

    // PROGRESS
    connect(worker, &CompressWorker::progress, this, [=](int pct){
        progressBar->setValue(pct);
        currentStatus->setText(QString(tr("Progres: %1%")).arg(pct));
    });

    // FINISHED
    connect(worker, &CompressWorker::finished, this, [=](bool ok, QString){
        mv->stop();
        table->removeCellWidget(job.row, 3);

        auto* it = new QTableWidgetItem(ok ? "✔" : "❌");
        it->setTextAlignment(Qt::AlignCenter);
        table->setItem(job.row, 3, it);

        // 🔥 Dimensiunea arhivei
        QFileInfo info(job.archivePath);
        double sizeMB = info.size() / 1024.0 / 1024.0;

        log(QString(tr("📦 Arhivă: %1 (%2 MB)"))
                .arg(toWinPath(job.archivePath),
                     QString::number(sizeMB, 'f', 1)));

        if (ok && globals::createFileSHA256) {
            createSha256File(job.archivePath);
        }

        log(ok ? tr("✔ Backup reușit") : tr("❌ Backup eșuat"));

        emit jobFinishedSignal(ok);
        t->quit();
    });

    connect(t, &QThread::finished, worker, &QObject::deleteLater);
    connect(t, &QThread::finished, t, &QObject::deleteLater);

    t->start();
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
            return p;     // găsit în PATH
        }

        if (QFile::exists(c)) {
            return c;     // găsit ca fișier hardcoded
        }
    }

    return QString();      // nu a fost găsit
}

bool MainWindow::createSha256File(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        log(tr("❌ Nu pot deschide fișierul pentru SHA-256: %1").arg(toWinPath(filePath)));
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    // Citește fișierul în blocuri mari → rapid și sigur
    while (!file.atEnd()) {
        hash.addData(file.read(1024 * 1024));  // 1 MB
    }

    QByteArray sha = hash.result().toHex();
    file.close();

    // Construim calea fișierului .sha256
    QString shaFile = filePath + ".sha256";
    QFile out(shaFile);

    if (!out.open(QFile::WriteOnly | QFile::Truncate)) {
        log(tr("❌ Nu pot crea fișierul SHA-256: %1").arg(toWinPath(shaFile)));
        return false;
    }

    // Format standard: sha256 + spatiu + * + nume fisier
    QString line = QString("%1 *%2\n")
                       .arg(QString::fromLatin1(sha),
                            QFileInfo(filePath).fileName());

    out.write(line.toUtf8());
    out.close();

    log(tr("🔐 Creat fișier SHA-256: %1").arg(toWinPath(shaFile)));
    return true;
}

void MainWindow::loadSettings()
{
    QFile f(settingsFilePath);
    if (!f.exists()) {
        //--- presupunem ca prima lansare

        globals::currentLang = "app_ru_RU";
        lblSwitch->setChecked(globals::currentLang == "app_ro_RO");
        switchLanguage(globals::currentLang);

        globals::isDark = false;
        themeSwitch->setChecked(globals::isDark);
        applyTheme();

        comboCompression->setCurrentIndex(5);

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
        log(tr("Folder backup încărcat din setări: ") + toWinPath(backupFolder));
    }

    if (obj.contains("backupExtFiles"))
        globals::backupExtFiles = obj["backupExtFiles"].toBool();

    if (obj.contains("createFileSHA256"))
        globals::createFileSHA256 = obj["createFileSHA256"].toBool();

    if (obj.contains("questionCloseApp"))
        globals::questionCloseApp = obj["questionCloseApp"].toBool();

    if (obj.contains("setArchivePassword"))
        globals::setArchivePassword = obj["setArchivePassword"].toBool();

    if (obj.contains("archivePassword"))
        globals::archivePassword = decryptPassword(obj.value("archivePassword").toString());

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
}

void MainWindow::saveSettings()
{
    QJsonObject obj;
    obj["compression"]        = comboCompression->currentIndex();
    obj["backupFolder"]       = backupFolder;
    obj["currentLang"]        = currentLang;
    obj["darkTheme"]          = themeSwitch->isChecked();
    obj["backupExtFiles"]     = globals::backupExtFiles;
    obj["createFileSHA256"]   = globals::createFileSHA256;
    obj["questionCloseApp"]   = globals::questionCloseApp;
    obj["setArchivePassword"] = globals::setArchivePassword;
    obj["archivePassword"]    = encryptPassword(globals::archivePassword);

    // ------------------------------------------
    // Salvăm dimensiunile coloanelor tabelului
    // ------------------------------------------
    QJsonArray arr;
    for (int c = 0; c < table->columnCount(); ++c)
        arr.append(table->columnWidth(c));
    obj["columnWidths"] = arr;

    QFile f(settingsFilePath);
    if (f.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(obj);
        f.write(doc.toJson());
        f.close();
    }

}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("1CArchiver v%1 – Backup al bazelor de date 1C:Enterprise")
                       .arg(VER));

    btnSelectAll->setText(tr("Selectează toate"));
    btnFolder->setText(tr("Alege folder backup"));
    btnArchive->setText(tr("Arhivează selectatele"));
    // btnGenerateTask->setText(tr("Generează Task XML"));

    themeLabel->setText(tr("Dark theme:"));
    lblLang->setText(tr("Limba RO:"));
    lblCompression->setText(tr("Compresie:"));

    currentStatus->setText(tr("Idle"));

    table->setHorizontalHeaderLabels({
        tr("Select"),
        tr("Denumire BD"),
        tr("Cale"),
        tr("Status")
    });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Dacă nu cerem confirmare → ieșim direct
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
