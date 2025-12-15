# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project follows Semantic Versioning.

---

## [Unreleased]

- Planned improvements and fixes.

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
