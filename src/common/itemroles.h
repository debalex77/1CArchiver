#pragma once
#include <Qt>

namespace ItemRole {

enum Role : int {
    DbType      = Qt::UserRole,      /** "one_file" | "mssql" */
    Configured  = Qt::UserRole + 1,  /** configured -> true or false */
    ConfigPath  = Qt::UserRole + 2,  /** path config JSON */
    Operations  = Qt::UserRole + 3,  /** QVariantMap
                                      *  {
                                      *     "backup":   true,
                                      *     "export1c": true,
                                      *     "sha256":   true,
                                      *     "dropbox":  false
                                      *   }
                                      */
};

}
