//
//
// "Desktop Icons Options" Tab for KDesktop configuration
//
// (c) Martin R. Jones 1996
//
// Port to KControl, split from "Misc" Tab, Port to KControl2
// (c) David Faure 1998
// Desktop menus, paths
// (c) David Faure 2000

#ifndef __ROOT_OPTIONS_H
#define __ROOT_OPTIONS_H


#include <tqstring.h>

#include <kcmodule.h>
#include <kio/global.h>
#include <kurl.h>

class TQCheckBox;
class TQComboBox;
class TQPushButton;

class TDEConfig;
class KListView;
class KURLRequester;

namespace TDEIO { class Job; }

//-----------------------------------------------------------------------------
// The "Path" Tab contains :
// The paths for Desktop, Autostart and Documents

class DesktopPathConfig : public TDECModule
{
        Q_OBJECT
public:
        DesktopPathConfig(TQWidget *parent = 0L, const char *name = 0L );
        virtual void load();
        virtual void load( bool useDefaults );
        virtual void save();
        virtual void defaults();

private slots:
    void slotEntries( TDEIO::Job * job, const TDEIO::UDSEntryList& list);

private:
        // Desktop Paths
        KURLRequester *urDesktop;
        KURLRequester *urAutostart;
        KURLRequester *urDocument;

        bool moveDir( const KURL & src, const KURL & dest, const TQString & type );
        bool m_ok;
        KURL m_copyToDest; // used when the destination directory already exists
        KURL m_copyFromSrc;

private slots:
        void slotResult( TDEIO::Job * job );
};

#endif // __ROOT_OPTIONS_H
