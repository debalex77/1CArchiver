# 1CArchiver  
### Backup rapid și sigur pentru bazele de date 1C:Enterprise  
![license](https://img.shields.io/badge/license-MIT-blue.svg)
![platform](https://img.shields.io/badge/platform-Windows%20x64-blue)
![qt](https://img.shields.io/badge/Qt-6.9.3-brightgreen)
![cpp](https://img.shields.io/badge/C%2B%2B-17-orange)
![bit7z](https://img.shields.io/badge/bit7z-4.0.10-lightgrey)

---

## 📌 Descriere

**1CArchiver** este o aplicație modernă Qt/C++ pentru backup automat și manual al bazelor de date **1C:Enterprise (1С:Предприятие)**.  
Aplicația arhivează fișierul `1Cv8.1CD` sau întregul director al bazei de date, generează fișier SHA-256 și oferă progres de arhivare în timp real.

Proiectul este compatibil cu:
- **Windows 64-bit**  
- **Qt 6.9.3 (MSVC 2022)**  
- **bit7z 4.0.10 (SevenZip SDK)**  

---

## ✨ Funcționalități

### 🔐 Arhivare și securitate
- Arhivare **.7z** folosind **LZMA / LZMA2** (through bit7z)
- Setare nivel compresie (`0–9`)
- Posibilitatea de a arhiva:
  - doar fișierul `1Cv8.1CD`
  - întregul director al bazei (mod extensii)
- Suport pentru **parolă** (opțional)
- Generarea automată a fișierului **.sha256**

### 🎛 Interfață modernă
- UI complet în **Qt Widgets**
- **Theme switcher** Light/Dark
- Traduceri dinamice:
  - 🇷🇺 Rusă (`ru_RU`)
  - 🇷🇴 Română (`ro_RO`)
- QTableWidget cu stil personalizat
- QProgressBar + spinner animat în timpul arhivării

### ⚙️ Funcții tehnice
- Worker-thread bazat pe `QThread` pentru arhivare (nu blochează UI)
- Calcul progres prin callback bit7z (bytes processed)
- Setări persistente în `settings.json`
- Backup folder configurabil
- Generator automats pentru fișier **Task XML**

### 🖥 Instalator profesionist
- Creator installer cu **Qt Installer Framework 4.10**
- Icon, Logo, Watermark personalizate
- Creare shortcut pe desktop și în Start Menu
- Descărcare automată și instalare **Visual C++ Redistributable**
- Generare hash SHA-256 pentru instalator

---

## 🚀 Compilare & Dependențe

### 🔧 Cerințe
- Qt 6.9.3 (MSVC 2022)
- Visual Studio Build Tools 2022
- 7-Zip instalat (pentru `7z.dll`)
- bit7z v4.0.10

### 🔨 Compilare

```bash
qmake
nmake


