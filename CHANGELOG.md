# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project follows Semantic Versioning.

---

## [Unreleased]

Planned improvements and fixes:
  - sending the backup log by email
  - archive synchronization via RSYNC
  - synchronization of archives with the external OneDrive service

---

# Changelog

---

## [1.10] – 2026-10-09

### 🇬🇧 English

#### New Features
- New plugin **Export 1C (.dt)**: databases are dumped with the 1C platform tools (`1cv8.exe DESIGNER /DumpIB`)
  - Works for **file** databases (`/F`) and **server** databases (`/S cluster\infobase`)
  - Configured per database: right click on the database in the table → **"Configure .dt export"** (1C user, password, optional path to `1cv8.exe`; for server databases also the 1C cluster and infobase name)
  - When `.dt` export is configured, it **replaces** the native backup (`.1CD` / `.bak`): `.dt` → `.7z` → `.sha256` → Dropbox (optional); the temporary `.dt` file is removed after archiving
  - The latest installed 1C platform version is detected automatically if no path is set
  - Export errors (exit code and the platform log) are shown in the log and the row is marked ❌; there is no silent fallback
- New plugin **Telegram**: after each backup (manual or `--autorun`) a bot sends a **summary** (computer, result, start time, duration, ✔/❌ per database) and the **full log** as a file
  - Mode **always** or **errors only**
  - **Test** button in the plugin window
  - In `--autorun` mode the application closes only after the report has been sent (30 s timeout per request)

#### Security & Data Handling
- The 1C password and the Telegram bot token are encrypted with **Windows DPAPI**
- The bot token is hidden in error messages

#### Bug Fixes
- Plugin configuration forms: the label of a hidden field is now hidden together with the field (e.g. "User" with Windows authentication for MSSQL)
- MSSQL: the row is marked ❌ when the `.bak` file remains locked by SQL Server

#### Notes
- The 1C password is passed on the `1cv8.exe` command line (the platform offers no other way) and is visible in the process list during the export
- For server databases the `1cv8.exe` version must match the version of the 1C server cluster; set the path explicitly if several platform versions are installed

---

### 🇷🇴 Română

#### Funcționalități noi
- Plugin nou **Export 1C (.dt)**: bazele se exportă cu uneltele platformei 1C (`1cv8.exe DESIGNER /DumpIB`)
  - Funcționează pentru baze **de fișiere** (`/F`) și baze **de server** (`/S cluster\bază`)
  - Configurare per bază: click dreapta pe bază în tabel → **„Configurare export .dt”** (utilizator 1C, parolă, opțional calea spre `1cv8.exe`; pentru bazele de server și clusterul 1C și numele bazei)
  - Când exportul `.dt` este configurat, acesta **înlocuiește** backup-ul nativ (`.1CD` / `.bak`): `.dt` → `.7z` → `.sha256` → Dropbox (opțional); fișierul `.dt` temporar se șterge după arhivare
  - Dacă nu este indicată calea, se detectează automat cea mai nouă versiune instalată a platformei 1C
  - Erorile exportului (codul returnat și logul platformei) apar în log, iar rândul este marcat ❌; nu există fallback ascuns
- Plugin nou **Telegram**: după fiecare arhivare (manuală sau `--autorun`) botul trimite un **rezumat** (calculator, rezultat, ora de start, durata, ✔/❌ pentru fiecare bază) și **logul complet** ca fișier
  - Mod **mereu** sau **doar la erori**
  - Buton **Test** în fereastra pluginurilor
  - În modul `--autorun` aplicația se închide abia după trimiterea raportului (timeout 30 s pe cerere)

#### Securitate și date
- Parola 1C și tokenul botului Telegram sunt criptate cu **Windows DPAPI**
- Tokenul botului este ascuns în mesajele de eroare

#### Corecții
- Formularele de configurare a pluginurilor: eticheta unui câmp ascuns se ascunde împreună cu câmpul (ex. „User” la autentificarea Windows pentru MSSQL)
- MSSQL: rândul este marcat ❌ când fișierul `.bak` rămâne blocat de SQL Server

#### Note
- Parola 1C este transmisă în linia de comandă `1cv8.exe` (platforma nu oferă altă metodă) și este vizibilă în lista proceselor pe durata exportului
- Pentru bazele de server versiunea `1cv8.exe` trebuie să coincidă cu versiunea clusterului 1C; indicați calea explicit dacă sunt instalate mai multe versiuni ale platformei

---

### 🇷🇺 Русский

#### Новые возможности
- Новый плагин **Выгрузка 1С (.dt)**: базы выгружаются средствами платформы 1С (`1cv8.exe DESIGNER /DumpIB`)
  - Работает для **файловых** (`/F`) и **серверных** баз (`/S кластер\база`)
  - Настройка для каждой базы: правый клик по базе в таблице → **«Настройка выгрузки .dt»** (пользователь 1С, пароль, при необходимости путь к `1cv8.exe`; для серверных баз также кластер 1С и имя базы)
  - Если выгрузка `.dt` настроена, она **заменяет** обычное резервное копирование (`.1CD` / `.bak`): `.dt` → `.7z` → `.sha256` → Dropbox (опционально); временный файл `.dt` удаляется после архивации
  - Если путь не указан, автоматически определяется последняя установленная версия платформы 1С
  - Ошибки выгрузки (код возврата и лог платформы) выводятся в лог, строка отмечается ❌; скрытого перехода на другой способ нет
- Новый плагин **Telegram**: после каждой архивации (ручной или `--autorun`) бот отправляет **сводку** (компьютер, результат, время начала, длительность, ✔/❌ по каждой базе) и **полный лог** файлом
  - Режим **всегда** или **только при ошибках**
  - Кнопка **Тест** в окне плагинов
  - В режиме `--autorun` приложение закрывается только после отправки отчёта (таймаут 30 с на запрос)

#### Безопасность
- Пароль 1С и токен бота Telegram шифруются с помощью **Windows DPAPI**
- Токен бота скрывается в сообщениях об ошибках

#### Исправления
- Формы настройки плагинов: подпись скрытого поля скрывается вместе с полем (например, «User» при Windows-аутентификации MSSQL)
- MSSQL: строка отмечается ❌, если файл `.bak` остаётся заблокированным SQL Server

#### Примечания
- Пароль 1С передаётся в командной строке `1cv8.exe` (платформа не поддерживает другой способ) и виден в списке процессов во время выгрузки
- Для серверных баз версия `1cv8.exe` должна совпадать с версией кластера 1С; укажите путь явно, если установлено несколько версий платформы

---

## [1.9] – 2026-10-05

### 🇬🇧 English

#### Dropbox
- Archives larger than **150 MB** are now uploaded in chunks (upload sessions); previously such uploads failed
- File names with **non-Latin characters** (e.g. Cyrillic database names) are uploaded correctly
- An expired access token is refreshed automatically during upload; previously the backup queue could hang
- The **"Stop Dropbox"** button is now active during uploads

#### Security & Data Handling
- Saved passwords (archive, MSSQL) are encrypted with **Windows DPAPI** instead of the previous fixed-key encryption; passwords saved by older versions are read and re-encrypted automatically
- The MSSQL password is no longer passed on the `sqlcmd` command line
- **Old archives cleanup** no longer deletes anything when the retention period is missing or `0`, and only removes files created by the application (`.7z`, `.7z.sha256`, `log_*.log`)
- The archive password is applied only when the password option is enabled
- Password-protected archives also encrypt the **file list** (7z headers): the password is required to open the archive, not only to extract it

#### Bug Fixes
- The UI no longer stays blocked when no database is selected; `--autorun` exits correctly in this case
- Update download: GitHub redirects are followed and errors are reported; the application no longer closes after a failed download
- Archive names with characters not allowed by Windows (e.g. `"`, `:`) are sanitized
- MSSQL: the real error message is shown; configuration errors are reported; progress works with SQL authentication
- MSSQL: configurations of the same database on different servers no longer overwrite each other; editing server/database updates the table row
- "Auto-detect 1C databases" adds only missing databases instead of clearing the table
- Settings: "No" in confirmation dialogs keeps the dialog open; closing with Esc applies the changes
- Task Scheduler: actions report failure when `schtasks` fails
- The application stops when `7z.dll` is missing
- Correct log order after archiving; the title bar follows the selected theme; damaged settings files are handled

#### Technical Improvements
- Updated **bit7z to v4.1.0** (MSVC 2019 for Qt5, MSVC 2022 for Qt6)
- The Qt5 package includes **OpenSSL 1.1.1w** (HTTPS for Dropbox and update checks)
- New build scripts `build_win_qt6.bat` / `build_win_qt5.bat`: build, ZIP, Inno Setup and QIF installers, SHA-256

#### Notes
- With DPAPI, saved passwords are bound to the **Windows user and computer**; after moving the configuration to another PC or account, passwords must be re-entered
- Do not downgrade to 1.8 after running 1.9: 1.8 cannot read DPAPI-encrypted passwords

---

### 🇷🇴 Română

#### Dropbox
- Arhivele mai mari de **150 MB** se încarcă pe bucăți (upload sessions); anterior încărcarea eșua
- Fișierele cu **caractere non-latine** (ex. denumiri de baze în chirilică) se încarcă corect
- Tokenul de acces expirat se reînnoiește automat în timpul încărcării; anterior coada de backup se putea bloca
- Butonul **„Oprește Dropbox”** este activ în timpul încărcării

#### Securitate și date
- Parolele salvate (arhivă, MSSQL) sunt criptate cu **Windows DPAPI** în locul criptării anterioare cu cheie fixă; parolele salvate de versiunile vechi sunt citite și recriptate automat
- Parola MSSQL nu mai este transmisă în linia de comandă `sqlcmd`
- **Eliminarea arhivelor vechi** nu mai șterge nimic dacă vechimea lipsește sau este `0` și șterge doar fișierele create de aplicație (`.7z`, `.7z.sha256`, `log_*.log`)
- Parola arhivei se aplică doar când opțiunea parolei este activată
- Arhivele cu parolă criptează și **lista fișierelor** (antetele 7z): parola este cerută la deschiderea arhivei, nu doar la extragere

#### Corecții
- Interfața nu mai rămâne blocată dacă nu este selectată nicio bază; `--autorun` se închide corect în acest caz
- Descărcarea actualizării: redirecționările GitHub sunt urmate, erorile sunt afișate; aplicația nu se mai închide după o descărcare eșuată
- Numele arhivelor cu caractere nepermise în Windows (ex. `"`, `:`) sunt corectate
- MSSQL: se afișează mesajul real de eroare; erorile de configurare sunt raportate; progresul funcționează cu autentificare SQL
- MSSQL: configurările aceleiași baze pe servere diferite nu se mai suprascriu; editarea serverului/bazei actualizează rândul din tabel
- „Detectare automată baze 1C” adaugă doar bazele lipsă, fără a goli tabelul
- Setări: „Nu” în dialogurile de confirmare păstrează fereastra deschisă; închiderea cu Esc aplică modificările
- Task Scheduler: acțiunile raportează eșecul când `schtasks` eșuează
- Aplicația se oprește dacă lipsește `7z.dll`
- Ordinea corectă a mesajelor după arhivare; bara de titlu urmează tema aleasă; fișierele de setări deteriorate sunt tratate

#### Îmbunătățiri tehnice
- Actualizare **bit7z la v4.1.0** (MSVC 2019 pentru Qt5, MSVC 2022 pentru Qt6)
- Pachetul Qt5 include **OpenSSL 1.1.1w** (HTTPS pentru Dropbox și verificarea actualizărilor)
- Scripturi noi de build `build_win_qt6.bat` / `build_win_qt5.bat`: compilare, ZIP, installere Inno Setup și QIF, SHA-256

#### Note
- Cu DPAPI, parolele salvate sunt legate de **utilizatorul Windows și calculator**; după mutarea configurării pe alt PC sau cont, parolele trebuie introduse din nou
- Nu reveniți la 1.8 după rularea 1.9: 1.8 nu poate citi parolele criptate DPAPI

---

### 🇷🇺 Русский

#### Dropbox
- Архивы размером более **150 МБ** загружаются частями (upload sessions); ранее загрузка завершалась ошибкой
- Файлы с **нелатинскими символами** (например, имена баз на кириллице) загружаются корректно
- Просроченный токен доступа обновляется автоматически во время загрузки; ранее очередь резервного копирования могла зависнуть
- Кнопка **«Остановить Dropbox»** активна во время загрузки

#### Безопасность
- Сохранённые пароли (архив, MSSQL) шифруются с помощью **Windows DPAPI** вместо прежнего шифрования с фиксированным ключом; пароли, сохранённые старыми версиями, читаются и перешифровываются автоматически
- Пароль MSSQL больше не передаётся в командной строке `sqlcmd`
- **Удаление старых архивов** ничего не удаляет, если срок хранения не указан или равен `0`, и удаляет только файлы, созданные приложением (`.7z`, `.7z.sha256`, `log_*.log`)
- Пароль архива применяется только при включённой опции пароля
- В архивах с паролем шифруется и **список файлов** (заголовки 7z): пароль запрашивается при открытии архива, а не только при извлечении

#### Исправления
- Интерфейс больше не блокируется, если не выбрана ни одна база; `--autorun` в этом случае корректно завершается
- Загрузка обновления: перенаправления GitHub обрабатываются, ошибки отображаются; приложение больше не закрывается после неудачной загрузки
- Имена архивов с недопустимыми в Windows символами (например, `"`, `:`) исправляются
- MSSQL: отображается реальное сообщение об ошибке; ошибки конфигурации сообщаются; прогресс работает при SQL-аутентификации
- MSSQL: конфигурации одной базы на разных серверах больше не перезаписывают друг друга; изменение сервера/базы обновляет строку таблицы
- «Автомат.определение баз данных 1С пользователя» добавляет только отсутствующие базы, не очищая таблицу
- Настройки: «Нет» в диалогах подтверждения оставляет окно открытым; закрытие по Esc применяет изменения
- Планировщик заданий: при ошибке `schtasks` выводится сообщение
- Приложение завершает работу при отсутствии `7z.dll`
- Правильный порядок сообщений после архивирования; заголовок окна следует выбранной теме; повреждённые файлы настроек обрабатываются

#### Технические улучшения
- Обновление **bit7z до v4.1.0** (MSVC 2019 для Qt5, MSVC 2022 для Qt6)
- Пакет Qt5 включает **OpenSSL 1.1.1w** (HTTPS для Dropbox и проверки обновлений)
- Новые скрипты сборки `build_win_qt6.bat` / `build_win_qt5.bat`: компиляция, ZIP, установщики Inno Setup и QIF, SHA-256

#### Примечания
- При использовании DPAPI сохранённые пароли привязаны к **пользователю Windows и компьютеру**; после переноса конфигурации на другой ПК или учётную запись пароли нужно ввести заново
- Не возвращайтесь к версии 1.8 после запуска 1.9: версия 1.8 не может прочитать пароли, зашифрованные DPAPI

---

## [1.8] – 2026-01-08

### 🇬🇧 English

#### New Features
- Added **MSSQL backup support (Beta)**:
  - Automatic creation of `.bak` files using `sqlcmd`
  - Real-time backup progress via SQL Server system views
  - Seamless integration into the existing backup pipeline
- Implemented **dynamic plugin system**:
  - Plugins can be enabled or disabled at runtime
  - MSSQL plugin activation via dedicated Plugin Manager
- Added **dynamic configuration UI from JSON schema**:
  - MSSQL configuration forms are generated dynamically
  - Supports validation, conditional fields, and presets
- Unified backup workflow:
  - MSSQL backups are converted internally to ONE_FILE jobs
  - `.bak` -> `.7z` -> `.sha256` -> Dropbox (optional)
  
#### Security & Data Handling
- Password fields are **encrypted before saving** in configuration files
- Temporary MSSQL `.bak` files are **automatically removed** after successful archive creation

#### UI / UX Improvements
- Added **Plugin Manager dialog** with advanced-user warning
- Context-aware menus for database addition:
  - 1C File Database
  - MSSQL Database
- Clear visual indicators for:
  - Configured / non-configured MSSQL databases
  - MSSQL Beta status
- Improved status messages and logs during MSSQL backup process

#### Technical Improvements
- Introduced `WorkerMSSQL` for MSSQL backup execution
- Improved thread safety and lambda capture correctness
- Fixed archive overwrite issues (`Wrong update mode`)
- Improved path handling and cross-platform include portability
- Refactored backup logic to reduce MainWindow complexity

#### Notes
- MSSQL support is currently **in beta testing**
- Tested with Microsoft SQL Server **2012–2019**
- Windows Authentication supported

---

### 🇷🇴 Română

#### Funcționalități noi
- Suport pentru **backup MSSQL (Beta)**:
  - Crearea automată a fișierelor `.bak` folosind `sqlcmd`
  - Afișarea progresului în timp real
  - Integrare completă în fluxul existent de backup
- Sistem de **pluginuri dinamice**:
  - Activare / dezactivare pluginuri în timp real
  - Gestionare prin Plugin Manager
- Interfață de configurare **dinamică din fișiere JSON**:
  - Formulare generate automat
  - Validare câmpuri și afișare condițională
- Flux unificat de backup:
  - MSSQL -> `.bak` -> `.7z` -> `.sha256` -> Dropbox (opțional)

#### Securitate și date
- Câmpurile de tip parolă sunt **criptate** la salvare
- Fișierele temporare `.bak` sunt **șterse automat** după arhivare reușită

#### UI / UX
- Dialog nou **Plugin Manager** cu mesaj de atenționare
- Meniu contextual pentru adăugare baze de date:
  - Bază 1C
  - Bază MSSQL
- Indicatori vizuali pentru:
  - Configurare MSSQL validă / invalidă
  - Funcționalitate MSSQL în beta
- Mesaje de status și log îmbunătățite

#### Îmbunătățiri tehnice
- Introducerea clasei `WorkerMSSQL`
- Corectarea capturilor lambda și gestionarea threadurilor
- Eliminarea erorilor de suprascriere arhivă
- Compatibilitate îmbunătățită cross-platform
- Refactorizare logică de backup pentru claritate

#### Note
- Backup-ul MSSQL este **în stadiu de beta-testare**
- Testat cu Microsoft SQL Server **2012–2019**
- Suport pentru autentificare Windows

---

### 🇷🇺 Русский

#### Новые возможности
- Добавлена поддержка **резервного копирования MSSQL (Beta)**:
  - Автоматическое создание файлов `.bak` через `sqlcmd`
  - Отображение прогресса в реальном времени
  - Полная интеграция в существующий процесс резервного копирования
- Реализована **плагинная архитектура**:
  - Включение и отключение плагинов во время работы
  - Управление через Plugin Manager
- **Динамический UI конфигурации из JSON**:
  - Формы создаются автоматически
  - Поддержка валидации и условных полей
- Унифицированный процесс резервного копирования:
  - MSSQL -> `.bak` -> `.7z` -> `.sha256` → Dropbox (опционально)

#### Безопасность
- Пароли **шифруются перед сохранением**
- Временные `.bak` файлы **удаляются автоматически** после успешного архивирования

#### Интерфейс
- Добавлен диалог **Plugin Manager** с предупреждением
- Контекстное меню добавления баз данных:
  - 1C
  - MSSQL
- Визуальные индикаторы:
  - Статус конфигурации MSSQL
  - MSSQL в стадии beta
- Улучшены сообщения состояния и логирование

#### Технические улучшения
- Добавлен класс `WorkerMSSQL`
- Исправлены ошибки захвата lambda
- Устранены проблемы обновления архивов
- Улучшена переносимость путей и include-файлов
- Оптимизирована архитектура MainWindow

#### Примечания
- Поддержка MSSQL находится **в стадии beta-тестирования**
- Протестировано с Microsoft SQL Server **2012–2019**
- Поддерживается Windows-аутентификация

---

## [1.7] – 2025-12-18

---

### 🇬🇧 English

#### Added
- General application description and improved informational texts.
- Automatic check for new application versions.
- Update notification dialog with version comparison and user-friendly interface.
- Optional automatic removal of old backup archives based on retention period.
- System tray notifications for backup start and completion.
- Improved autorun mode with background execution and tray-only notifications.

#### Improved
- More reliable handling of system tray messages (fixed missing notifications on application exit).
- Clearer and more consistent user messages in dialogs and tray notifications.
- Improved application startup flow to avoid UI blocking.

#### Fixed
- Fixed issues where tray notifications were not displayed due to immediate application shutdown.
- Fixed logic issues related to backup completion and background execution.
- Minor UI and wording fixes across the application.

---

### 🇷🇴 Română

#### Adăugat
- Descriere generală a aplicației și texte informative îmbunătățite.
- Verificare automată a existenței unei versiuni noi a aplicației.
- Dialog de notificare pentru actualizare, cu comparare corectă a versiunilor.
- Eliminare automată opțională a arhivelor vechi, pe baza numărului de zile configurat.
- Notificări în System Tray pentru pornirea și finalizarea arhivării.
- Mod autorun îmbunătățit, cu rulare în fundal și notificări exclusiv în tray.

#### Îmbunătățit
- Gestionare mai fiabilă a mesajelor din System Tray (remedierea cazurilor în care mesajele nu apăreau).
- Mesaje mai clare și coerente în dialoguri și notificări.
- Flux de pornire al aplicației optimizat, fără blocarea interfeței.

#### Corectat
- Corectarea problemei în care notificările tray nu erau afișate din cauza închiderii rapide a aplicației.
- Corectarea logicii de finalizare a backup-ului în modul automat.
- Corecții minore de interfață și formulare a mesajelor.

---

### 🇷🇺 Русский

#### Добавлено
- Общее описание приложения и улучшенные информационные тексты.
- Автоматическая проверка наличия новой версии приложения.
- Диалог уведомления об обновлении с корректным сравнением версий.
- Опциональное автоматическое удаление старых архивов по заданному сроку хранения.
- Уведомления в системном трее о начале и завершении архивирования.
- Улучшенный режим автозапуска с работой в фоновом режиме и уведомлениями только через трей.

#### Улучшено
- Более надёжная обработка уведомлений системного трея (исправлены случаи, когда уведомления не отображались).
- Более понятные и единообразные сообщения в диалогах и уведомлениях.
- Оптимизирован процесс запуска приложения без блокировки интерфейса.

#### Исправлено
- Исправлена проблема, при которой уведомления в трее не отображались из-за слишком быстрого завершения приложения.
- Исправлена логика завершения резервного копирования в автоматическом режиме.
- Небольшие исправления интерфейса и текстов сообщений.

---

## [1.6] - 2025-12-15

### 🇬🇧 English

#### Added
- Added **“Select database directory”** button to allow adding 1C databases located outside the user’s default directory.
- Added a **context menu for the database table**, providing:
  - Clear all rows
  - Remove selected row
  - Auto-detect 1C databases for the current user
- Added **Windows Task Scheduler integration**:
  - Ability to create a scheduled backup task directly from the application
  - Support for automatic application startup via `--autorun`
  - Background execution without showing the main window
  - System tray notification and warning message before backup starts
  - Graceful application exit after all backup jobs are finished

#### Fixed
- Fixed Dropbox synchronization by introducing a **startup health check (`DropboxHealthChecker`)**:
  - Proper validation of stored Dropbox access tokens
  - Automatic token refresh at application startup
  - Correct detection of Dropbox connection state
  - Eliminated false “authorization required” status after restart

🔸 🔸 🔸

### 🇷🇴 Română

#### Adăugat
- A fost adăugat butonul **„Alege directorul cu BD”**, care permite adăugarea bazelor de date 1C aflate în afara directorului implicit al utilizatorului.
- A fost adăugat **meniul contextual al tabelei**, care include:
  - Ștergerea tuturor rândurilor
  - Ștergerea rândului curent
  - Autodetectarea bazelor de date 1C ale utilizatorului curent
- A fost adăugată **integrarea cu Windows Task Scheduler**:
  - Crearea task-ului de backup direct din aplicație
  - Pornirea automată a aplicației folosind parametrul `--autorun`
  - Rulare în fundal fără afișarea ferestrei principale
  - Notificare în tray și mesaj de avertizare înainte de pornirea backup-ului
  - Închiderea automată a aplicației după finalizarea tuturor backup-urilor

#### Corectat
- A fost corectată sincronizarea Dropbox prin introducerea unui **mecanism de verificare la pornire (`DropboxHealthChecker`)**:
  - Verificarea corectă a token-ului Dropbox salvat
  - Reîmprospătarea automată a token-ului la pornirea aplicației
  - Detectarea corectă a stării conexiunii Dropbox
  - Eliminarea mesajelor false de tip „este necesară autorizarea” după repornire

🔸 🔸 🔸

### 🇷🇺 Русский

#### Добавлено
- Добавлена кнопка **«Выбрать каталог с БД»**, позволяющая добавлять базы данных 1С, расположенные вне стандартного каталога пользователя.
- Добавлено **контекстное меню таблицы**, включающее:
  - Удаление всех строк
  - Удаление текущей строки
  - Автоопределение баз данных 1С текущего пользователя
- Добавлена **интеграция с Планировщиком заданий Windows**:
  - Создание задания резервного копирования прямо из приложения
  - Автоматический запуск приложения с параметром `--autorun`
  - Фоновый режим работы без отображения главного окна
  - Уведомление в системном трее и предупреждающее сообщение перед началом архивации
  - Автоматическое завершение приложения после окончания всех задач резервного копирования

#### Исправлено
- Исправлена синхронизация с Dropbox путём внедрения **проверки состояния при запуске (`DropboxHealthChecker`)**:
  - Корректная проверка сохранённого Dropbox access token
  - Автоматическое обновление токена при запуске приложения
  - Корректное определение состояния подключения к Dropbox
  - Устранено ложное сообщение «требуется авторизация» после перезапуска
  
---

## [1.5] - 2025-12-13

### 🇬🇧 English

### Added
- Dropbox synchronization using OAuth2 PKCE
- Sequential workflow: backup → SHA-256 → Dropbox upload
- Optional upload of `.sha256` files
- Abort button for Dropbox upload

### Changed
- Backup and upload flow is now strictly sequential
- Installer updated to include Dropbox components
- Improved UI status and progress reporting

🔸 🔸 🔸

### 🇷🇴 Română

### Adăugat
- Sincronizare cu Dropbox folosind OAuth2 PKCE
- Flux secvențial: backup → SHA-256 → upload în Dropbox
- Upload opțional al fișierelor `.sha256`
- Buton de anulare pentru upload-ul Dropbox

### Modificat
- Fluxul de backup și upload este acum strict secvențial
- Installerul a fost actualizat pentru a include componentele Dropbox
- Îmbunătățirea afișării stării și a progresului în interfața utilizatorului

🔸 🔸 🔸

### 🇷🇺 Русский

### Добавлено
- Синхронизация с Dropbox с использованием OAuth2 PKCE
- Последовательный процесс: резервное копирование → SHA-256 → загрузка в Dropbox
- Опциональная загрузка файлов `.sha256`
- Кнопка отмены загрузки в Dropbox

### Изменено
- Процесс резервного копирования и загрузки теперь строго последовательный
- Установщик обновлён и включает компоненты Dropbox
- Улучшено отображение состояния и прогресса в пользовательском интерфейсе

---

## [1.4] - 2025-12-09

### Fixed
- Fixed random crashes related to QString construction
- Fixed `QIODevice::read: device not open` during uploads
- Fixed lambda capture and HTML formatting issues
- Fixed race conditions between backup and upload

---

## [1.3] - 2025-11-20

### Added
- Automatic backup of 1C file-based databases
- 7-Zip compression with password support
- Progress bar for archive creation

### Fixed
- Minor UI and stability issues
