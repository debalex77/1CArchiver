# =======================
# bit7z.pri – MSVC2022 x64
# =======================

BIT7Z_ROOT = $$PWD

# Include directory with bit7z headers
INCLUDEPATH += $$BIT7Z_ROOT/include

# Choose correct library depending on Debug/Release
CONFIG(release, debug|release) {
    message("Using bit7z Release library")
    LIBS += -L$$BIT7Z_ROOT/lib/x64/Release -lbit7z
} else {
    message("Using bit7z Debug library")
    LIBS += -L$$BIT7Z_ROOT/lib/x64/Debug -lbit7z
}

# Copy 7z.dll next to executable
win32 {
    DLL_DESTDIR = $$OUT_PWD

    QMAKE_POST_LINK += \
        xcopy /Y /Q \"$$BIT7Z_ROOT\\bin\\7z.dll\" \"$$DLL_DESTDIR\\\"
}
