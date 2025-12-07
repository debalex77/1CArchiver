#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    // Detectează dacă sistemul folosește tema dark
    static bool isDark();
};

#endif // THEMEMANAGER_H
