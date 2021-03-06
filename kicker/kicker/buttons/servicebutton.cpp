/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <tqdragobject.h>
#include <tqtooltip.h>

#include <kdesktopfile.h>
#include <tdelocale.h>
#include <kiconeffect.h>
#include <kicontheme.h>
#include <kpropertiesdialog.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "global.h"
#include "kicker.h"

#include "servicebutton.h"
#include "servicebutton.moc"

ServiceButton::ServiceButton(const TQString& desktopFile, TQWidget* parent)
  : PanelButton(parent, "ServiceButton"),
    _service(0)
{
    loadServiceFromId(desktopFile);
    initialize();
}

ServiceButton::ServiceButton(const KService::Ptr &service, TQWidget* parent)
  : PanelButton(parent, "ServiceButton"),
    _service(service),
    _id(service->storageId())
{
    if (_id.startsWith("/"))
    {
       TQString tmp = TDEGlobal::dirs()->relativeLocation("appdata", _id);
       if (!tmp.startsWith("/"))
          _id = ":"+tmp;
    }
    initialize();
}

ServiceButton::ServiceButton( const TDEConfigGroup& config, TQWidget* parent )
  : PanelButton(parent, "ServiceButton"),
    _service(0)
{
    TQString id;
    if (config.hasKey("StorageId"))
       id = config.readPathEntry("StorageId");
    else
       id = config.readPathEntry("DesktopFile");
    loadServiceFromId(id);
    initialize();
}

ServiceButton::~ServiceButton()
{
}

void ServiceButton::loadServiceFromId(const TQString &id)
{
    _id = id;
    /* this is a KService::Ptr
       don't need to delete it
    delete _service;
    */
    _service = 0;

    if (_id.startsWith(":"))
    {
       _id = locate("appdata", id.mid(1));
       if (!_id.isEmpty())
       {
          KDesktopFile df(_id, true);
          _service = new KService(&df);
       }
    }
    else
    {
       _service = KService::serviceByStorageId(_id);
       if (_service)
       {
          _id = _service->storageId();
       }
    }

    if (_service)
    {
        backedByFile(_service->desktopEntryPath());
    }

    if (_id.startsWith("/"))
    {
       TQString tmp = TDEGlobal::dirs()->relativeLocation("appdata", _id);
       if (!tmp.startsWith("/"))
          _id = ":"+tmp;
    }
}

void ServiceButton::initialize()
{
    readDesktopFile();
    connect(this, TQT_SIGNAL(clicked()), TQT_SLOT(slotExec()));
}

void ServiceButton::readDesktopFile()
{
    if ( !_service || !_service->isValid() )
    {
        m_valid = false;
        return;
    }

    if (!_service->genericName().isEmpty())
    {
        TQToolTip::add(this, _service->genericName());
    }
    else if (_service->comment().isEmpty())
    {
        TQToolTip::add(this, _service->name());
    }
    else
    {
        TQToolTip::add(this, _service->name() + " - " + _service->comment());
    }

    setTitle( _service->name() );
    setIcon( _service->icon() );
}

void ServiceButton::saveConfig( TDEConfigGroup& config ) const
{
    config.writePathEntry("StorageId", _id );
    if (!config.hasKey("DesktopFile") && _service)
       config.writePathEntry("DesktopFile", _service->desktopEntryPath());
}

void ServiceButton::dragEnterEvent(TQDragEnterEvent *ev)
{
    if ((ev->source() != this) && KURLDrag::canDecode(ev))
        ev->accept(rect());
    else
        ev->ignore(rect());
    PanelButton::dragEnterEvent(ev);
}

void ServiceButton::dropEvent( TQDropEvent* ev )
{
    KURL::List uriList;
    if( KURLDrag::decode( ev, uriList ) && _service ) {
        kapp->propagateSessionManager();
        KRun::run( *_service, uriList );
    }
    PanelButton::dropEvent(ev);
}

void ServiceButton::startDrag()
{
    TQString path = _service->desktopEntryPath();

    // If the path to the desktop file is relative, try to get the full
    // path from KStdDirs.
    path = locate("apps", path);

    KURL url;
    url.setPath(path);
    emit dragme(KURL::List(url), labelIcon());
}

void ServiceButton::slotExec()
{
    // this allows the button to return to a non-pressed state
    // before launching
    TQTimer::singleShot(0, this, TQT_SLOT(performExec()));
}

void ServiceButton::performExec()
{
    if (!_service) return;

    KURL::List uriList;
    kapp->propagateSessionManager();
    KRun::run( *_service, uriList );
}

void ServiceButton::properties()
{
    if (!_service)
    {
        return;
    }

    TQString path = _service->desktopEntryPath();

    // If the path to the desktop file is relative, try to get the full
    // path from KStdDirs.
    path = locate("apps", path);
    KURL serviceURL;
    serviceURL.setPath(path);

    // the KPropertiesDialog deletes itself, so this isn't a memory leak
    KPropertiesDialog* dialog = new KPropertiesDialog(serviceURL, 0, 0,
                                                      false, false);
    dialog->setFileNameReadOnly(true);
    connect(dialog, TQT_SIGNAL(saveAs(const KURL &, KURL &)),
            this, TQT_SLOT(slotSaveAs(const KURL &, KURL &)));
    connect(dialog, TQT_SIGNAL(propertiesClosed()),
            this, TQT_SLOT(slotUpdate()));
    dialog->show();
}

void ServiceButton::slotUpdate()
{
    loadServiceFromId(_id);
    readDesktopFile();
    emit requestSave();
}

void ServiceButton::slotSaveAs(const KURL &oldUrl, KURL &newUrl)
{
    TQString oldPath = oldUrl.path();
    if (locateLocal("appdata", oldUrl.fileName()) != oldPath)
    {
       TQString path = KickerLib::newDesktopFile(oldUrl);
       newUrl.setPath(path);
       _id = path;
    }
}

bool ServiceButton::checkForBackingFile()
{
    TQString id = _id;
    loadServiceFromId(_id);

    // we need to reset the _id back to whatever it was
    // so that when we get called again to check on it we
    // know still know what to be looking for.
    _id = id;

    return _service != 0;
}
