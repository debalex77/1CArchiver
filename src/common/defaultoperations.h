#pragma once
#include <QVariantMap>
#include "src/globals.h"

inline QVariantMap defaultOperations()
{
    return {
        { "backup",   true  },
        { "export1c", false },
        { "sha256",   false },
        { "dropbox",  false }
    };
}

inline QVariantMap defaultOperationsWithGlobals()
{
    QVariantMap ops = defaultOperations();

    ops["export1c"] = globals::pl_export1c;
    ops["sha256"]   = globals::createFileSHA256;
    ops["dropbox"]  = globals::activate_syncDropbox;

    return ops;
}
