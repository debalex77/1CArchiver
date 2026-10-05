# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project follows Semantic Versioning.

---

## [Unreleased]

Planned improvements and fixes:
  - archive synchronization via RSYNC
  - synchronization of archives with the external OneDrive service

---

# Changelog

---

## [1.9] – 2026-10-05

### 🇬🇧 English

#### New Features
- Added **Export 1C plugin**:
  - Full infobase dump (`.dt`) via `1cv8.exe DESIGNER /DumpIB`
  - File (`/F`) and client-server (`/S`) infobases
  - Per-database or global configuration (`%APPDATA%/plugins/export_1c/`)
  - `.dt` -> `.7z` -> `.sha256` -> Dropbox (optional)
  - Falls back to archiving the `.1CD` file when the plugin is disabled or not configured

#### Security & Data Handling
- Saved passwords are encrypted with **Windows DPAPI** instead of the previous fixed-key encryption
- Passwords saved by older versions are still read correctly
- Temporary `.dt` files are **automatically removed** after successful archive creation

#### Technical Improvements
- Fixed the order of completion events after archive creation (Dropbox upload and notifications could start too early)
- `7z.dll` is copied into the output folder for Debug builds
- Updated **bit7z to v4.1.0** (MSVC 2022)
- Installers and portable packages for Qt5 and Qt6 are built by **GitHub Actions**

#### Notes
- With DPAPI, saved passwords are bound to the **Windows user and computer**; after moving the configuration to another PC or account, passwords must be re-entered

---

### 🇷🇴 Română

#### Funcționalități noi
- Plugin nou **Export 1C**:
  - Descărcarea completă a bazei (`.dt`) prin `1cv8.exe DESIGNER /DumpIB`
  - Baze de tip fișier (`/F`) și client-server (`/S`)
  - Configurare per bază sau globală (`%APPDATA%/plugins/export_1c/`)
  - `.dt` -> `.7z` -> `.sha256` -> Dropbox (opțional)
  - Dacă pluginul e dezactivat sau neconfigurat, se arhivează fișierul `.1CD` ca înainte

#### Securitate și date
- Parolele salvate sunt criptate cu **Windows DPAPI** în locul criptării anterioare cu cheie fixă
- Parolele salvate de versiunile vechi sunt citite în continuare corect
- Fișierele temporare `.dt` sunt **șterse automat** după arhivare reușită

#### Îmbunătățiri tehnice
- Corectată ordinea evenimentelor de finalizare după crearea arhivei (încărcarea în Dropbox și notificările puteau porni prea devreme)
- `7z.dll` se copiază în folderul de ieșire și pentru build-urile Debug
- **bit7z actualizat la v4.1.0** (MSVC 2022)
- Installerele și pachetele portabile Qt5 și Qt6 sunt construite de **GitHub Actions**

#### Note
- Cu DPAPI, parolele salvate sunt legate de **utilizatorul Windows și de calculator**; după mutarea configurației pe alt PC sau alt cont, parolele trebuie introduse din nou

---

### 🇷🇺 Русский

#### Новые возможности
- Добавлен плагин **Выгрузка 1С**:
  - Полная выгрузка информационной базы (`.dt`) через `1cv8.exe DESIGNER /DumpIB`
  - Файловые (`/F`) и клиент-серверные (`/S`) базы
  - Настройка для отдельной базы или общая (`%APPDATA%/plugins/export_1c/`)
  - `.dt` -> `.7z` -> `.sha256` -> Dropbox (опционально)
  - Если плагин отключён или не настроен, архивируется файл `.1CD`, как раньше

#### Безопасность
- Сохранённые пароли шифруются с помощью **Windows DPAPI** вместо прежнего шифрования с фиксированным ключом
- Пароли, сохранённые предыдущими версиями, по-прежнему читаются корректно
- Временные `.dt` файлы **удаляются автоматически** после успешного архивирования

#### Технические улучшения
- Исправлен порядок событий завершения после создания архива (загрузка в Dropbox и уведомления могли начинаться слишком рано)
- `7z.dll` копируется в выходную папку и для Debug-сборок
- **bit7z обновлён до v4.1.0** (MSVC 2022)
- Установщики и портативные пакеты Qt5 и Qt6 собираются через **GitHub Actions**

#### Примечания
- С DPAPI сохранённые пароли привязаны к **пользователю Windows и компьютеру**; после переноса настроек на другой ПК или учётную запись пароли нужно ввести заново

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
