/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __PasswdDlg_h_Incluced__
#define __PasswdDlg_h_Incluced__

#include <kpassdlg.h>

class KDEpasswd1Dialog
    : public KPasswordDialog
{
    Q_OBJECT

public:
    KDEpasswd1Dialog();
    ~KDEpasswd1Dialog();

    static int getPassword(TQCString &password);

protected:
    bool checkPassword(const char *password);
};
    

class KDEpasswd2Dialog
    : public KPasswordDialog
{
    Q_OBJECT

public:
    KDEpasswd2Dialog(const char *oldpass, TQCString user);
    ~KDEpasswd2Dialog();

protected:
    bool checkPassword(const char *password);
    
private:
    const char *m_Pass;
    TQCString m_User;
};
    


#endif // __PasswdDlg_h_Incluced__
