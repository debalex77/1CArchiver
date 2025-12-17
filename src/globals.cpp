#include "globals.h"

namespace globals {

    bool isAutorun          = false;    /** autorun app */
    bool isDark             = false;    /** tema aplicatiei */
    QString currentLang     = nullptr;  /** limaba intefetei */

    bool backupExtFiles     = false;    /** backup-ul fisierelor externe -> tot directoriu bd */
    bool createFileSHA256   = false;    /** citirea sumei SHA256 a backupului si crearea fisierului */

    bool questionCloseApp   = false;    /** interogarea la finisarea programei */

    bool setArchivePassword = false;    /** setarea parolei arhivului */
    QString archivePassword = nullptr;  /** parola propriu-zisa a arhivei */

    bool syncDropbox              = false;    /** sincronizarea cu Dropbox */
    bool activate_syncDropbox     = false;    /** activarea sincronizarii Dropbox */
    QString loginSuccesDropbox    = nullptr;  /** data logarii in Dropbox */

    bool syncGoogleDrive          = false;    /** sincronizarea cu GoogleDrive */
    bool activate_syncGoogleDrive = false;    /** activarea sincronizarii GoogleDrive */
    QString loginSuccesGoogleDrive = nullptr; /** data loagrii GoogleDrive */

    bool deleteArchives = false; /** eliminarea arhivelor */
    int lastNrDay = -1;          /** arhive mai mari de 3 zile (exemplu) */

}
