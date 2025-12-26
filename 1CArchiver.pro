QT += core gui widgets network

CONFIG += c++17
CONFIG += debug_and_release

TEMPLATE = app
TARGET = 1CArchiver

SOURCES += \
    main.cpp \
    src/aboutdialog.cpp \
    src/appsettings.cpp \
    src/compressworker.cpp \
    src/core/pluginactivator.cpp \
    src/core/pluginmanager.cpp \
    src/dropbox/connectordropbox.cpp \
    src/dropbox/dropboxconnectdialog.cpp \
    src/dropbox/dropboxhealthchecker.cpp \
    src/dropbox/dropboxoauth2_pkce.cpp \
    src/dropbox/dropboxuploader.cpp \
    src/globals.cpp \
    src/lineeditpassword.cpp \
    src/mainwindow.cpp \
    src/ibaseparser.cpp \
    src/scheduler/scheduledtaskdialog.cpp \
    src/switchbutton.cpp \
    src/thememanager.cpp \
    src/ui/dynamicpluginform.cpp \
    src/ui/pluginconfigdialog.cpp \
    src/updatechecker.cpp \
    src/updatedialog.cpp

HEADERS += \
    src/aboutdialog.h \
    src/appsettings.h \
    src/compressworker.h \
    src/core/IBackupPlugin.h \
    src/core/pluginactivator.h \
    src/core/pluginmanager.h \
    src/dropbox/connectordropbox.h \
    src/dropbox/dropboxconnectdialog.h \
    src/dropbox/dropboxhealthchecker.h \
    src/dropbox/dropboxoauth2_pkce.h \
    src/dropbox/dropboxuploader.h \
    src/globals.h \
    src/lineeditpassword.h \
    src/mainwindow.h \
    src/ibaseparser.h \
    src/IBASEEntry.h \
    src/scheduler/scheduledtaskdialog.h \
    src/switchbutton.h \
    src/thememanager.h \
    src/ui/dynamicpluginform.h \
    src/ui/pluginconfigdialog.h \
    src/updatechecker.h \
    src/updatedialog.h \
    src/utils.h \
    src/version.h

RESOURCES += resources/resources.qrc \
    installer/config/installer.qrc

FORMS += \
    src/dropbox/dropboxconnectdialog.ui

TRANSLATIONS += \
    resources/translations/1CArchiver_ru_RU.ts
#lupdate 1CArchiver.pro -ts resources/translations/1CArchiver_app_ru_RU.ts
#lrelease resources/translations/1CArchiver_app_ru_RU.ts -qm resources/translations/1CArchiver_app_ru_RU.qm

DISTFILES += \
    CHANGELOG.md \
    LICENSE \
    PRIVACY.md \
    README.md \
    README_RU.md \
    app.manifest \
    app.rc \
    assets/first_image.png \
    assets/first_image_ru.png \
    build_script/build_win.bat \
    build_script/build_win_old.bat \
    docs/1CArchiver-User-Manual-RO.pdf \
    docs/1CArchiver-User-Manual-RU.pdf \
    index.html \
    installer/config/backup.png \
    installer/config/config.xml \
    installer/config/style.qss \
    installer/config/welcome.html \
    installer/packages/com.oxvalprim.archiver/data/com.oxvalprim.1CAchiver.desktop \
    installer/packages/com.oxvalprim.archiver/meta/installscript.qs \
    installer/packages/com.oxvalprim.archiver/meta/license.txt \
    installer/packages/com.oxvalprim.archiver/meta/package.xml \
    ru/index.html \
    sitemap.xml \
    version.txt

# IMPORTANT: Biblioteci Windows necesare pentru bit7z
LIBS += -loleaut32 -lole32 -luuid

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/bit7z/lib/x64/release/ -lbit7z
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/bit7z/lib/x64/debug/ -lbit7z

# Corecție: include directorul părinte, nu subdirectorul bit7z
INCLUDEPATH += $$PWD/3rdparty/bit7z/include
DEPENDPATH += $$PWD/3rdparty/bit7z/include

win32 {
    QMAKE_PROJECT_DEPTH = 0
    RC_FILE = app.rc
    LIBS += -ladvapi32

    DLL_SRC = $$shell_path($$PWD/3rdparty/bit7z/bin/7z.dll)
    DLL_DST = $$shell_path($$OUT_PWD/release/7z.dll)

    QMAKE_POST_LINK += cmd /c copy /Y "$$DLL_SRC" "$$DLL_DST"
}


