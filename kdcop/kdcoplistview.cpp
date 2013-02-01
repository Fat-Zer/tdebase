/*
 * Copyright (C) 2000 by Ian Reinhart Geiser <geiseri@kde.org>
 *
 * Licensed under the Artistic License.
 */

#include "kdcoplistview.h"
#include "kdcoplistview.moc"
#include <kdebug.h>
#include <tqdragobject.h>
#include <tqstringlist.h>
#include <tqregexp.h>


KDCOPListView::KDCOPListView ( TQWidget *parent, const char *name)
    : TDEListView(parent, name)
{
	kdDebug() << "Building new list." << endl;
	setDragEnabled(true);
}


KDCOPListView::~KDCOPListView ()
{

}

TQDragObject *KDCOPListView::dragObject()
{
	kdDebug() << "Drag object called... " << endl;
	if(!currentItem())
		return 0;
	else
		return new TQTextDrag(encode(this->selectedItem()), this);
}

void KDCOPListView::setMode( const TQString &theMode )
{
	mode = theMode;
}

TQString KDCOPListView::encode(TQListViewItem *theCode)
{
	DCOPBrowserItem * item = static_cast<DCOPBrowserItem *>(theCode);

	if (item->type() != DCOPBrowserItem::Function)
	return "";

	DCOPBrowserFunctionItem * fitem =
	static_cast<DCOPBrowserFunctionItem *>(item);

	TQString function = TQString::fromUtf8(fitem->function());
	TQString application = TQString::fromUtf8(fitem->app());
	TQString object = TQString::fromUtf8(fitem->object());

	kdDebug() << function << endl;
	TQString returnType = function.section(' ', 0,0);
	TQString returnCode = "";
	TQString normalisedSignature;
	TQStringList types;
	TQStringList names;

	TQString unNormalisedSignature(function);

	int s = unNormalisedSignature.find(' ');

	if ( s < 0 )
		s = 0;
	else
		s++;

	unNormalisedSignature = unNormalisedSignature.mid(s);

	int left  = unNormalisedSignature.find('(');
	int right = unNormalisedSignature.findRev(')');

	if (-1 == left)
	{
	// Fucked up function signature.
	return "";
	}
	if (left > 0 && left + 1 < right - 1)
	{
		types = TQStringList::split
		(',', unNormalisedSignature.mid(left + 1, right - left - 1));
		for (TQStringList::Iterator it = types.begin(); it != types.end(); ++it)
		{
			(*it) = (*it).stripWhiteSpace();
			int s = (*it).find(' ');
			if (-1 != s)
			{
				names.append((*it).mid(s + 1));
				(*it) = (*it).left(s);
			}
		}
	}

	if ( mode == "C++")
	{
		TQString args;
		for( unsigned int i = 0; i < names.count(); i++)
		{
			args += types[i] + " " + names[i] + ";\n";
		}
		TQString dcopRef = "DCOPRef m_" + application + object
			+ "(\""+ application + "\",\"" + object +"\");\n";

		TQString stringNames = names.join(",");
		TQString stringTypes = types.join(",");
		if( returnType != "void")
			returnType += " return" + returnType + " =";
		else
			returnType = "";
		returnCode = args
			+ dcopRef
			+ returnType
			+ "m_" + application + object
			+ ".call(\"" + unNormalisedSignature.left(left)
			+ "(" + stringTypes + ")\"";
			if(!stringNames.isEmpty())
				returnCode += ", ";
			returnCode += stringNames + ");\n";
	}
	else if (mode == "Shell")
	{
		returnCode = "dcop " + application + " " + object + " "
			+ unNormalisedSignature.left(left) + " " + names.join(" ");
	}
	else if (mode == "Python")
	{
		TQString setup;
		setup = "m_"
			+ application + object
			+ " = dcop.DCOPObject( \""
			+ application + "\",\""
		 	+ object + "\")\n";

		for( unsigned int i = 0; i < names.count(); i++)
		{
			setup += names[i] + " #set value here.\n";
		}
		returnCode = setup
			+ "reply"
			+ returnType
			+ " = m_"
			+ application + object + "."
			+ unNormalisedSignature.left(left)
			+ "(" + names.join(",") + ")\n";
	}
	return returnCode;
}

TQString KDCOPListView::getCurrentCode() const
{
	// fixing warning
	return TQString::null;
}
