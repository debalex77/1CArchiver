# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
and this project follows Semantic Versioning.

---

## [Unreleased]
- Planned improvements and fixes.

---

## [1.4.0] - 2025-12-13

### Added
- Dropbox synchronization using OAuth2 PKCE
- Sequential workflow: backup → SHA-256 → Dropbox upload
- Optional upload of `.sha256` files
- Abort button for Dropbox upload

### Changed
- Backup and upload flow is now strictly sequential
- Installer updated to include Dropbox components
- Improved UI status and progress reporting

---

## [1.4.0] - 2025-12-09

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
