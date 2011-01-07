/*
 *   Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as 
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef __menufile_h__
#define __menufile_h__

#include <tqdom.h>
#include <tqstring.h>

class MenuFile
{
public:
   MenuFile(const TQString &file);
   ~MenuFile();
   
   bool load();
   bool save();
   void create();
   TQString error() { return m_error; } // Returns the last error message

   enum ActionType {
       ADD_ENTRY = 0,
       REMOVE_ENTRY,
       ADD_MENU,
       REMOVE_MENU,
       MOVE_MENU
   };

   struct ActionAtom
   {
      ActionType action;
      TQString arg1;
      TQString arg2;
   };

   /**
    * Create action atom and push it on the stack
    */
   ActionAtom *pushAction(ActionType action, const TQString &arg1, const TQString &arg2);

   /**
    * Pop @p atom from the stack.
    * @p atom must be last item on the stack
    */
   void popAction(ActionAtom *atom);

   /**
    * Perform the specified action
    */  
   void performAction(const ActionAtom *);
   
   /**
    * Perform all actions currently on the stack, remove them from the stack and
    * save result
    * @return whether save was successful
    */
   bool performAllActions();
   
   /**
    * Returns whether the stack contains any actions
    */
   bool dirty();
   
   void addEntry(const TQString &menuName, const TQString &menuId);
   void removeEntry(const TQString &menuName, const TQString &menuId);
   
   void addMenu(const TQString &menuName, const TQString &menuFile);
   void moveMenu(const TQString &oldMenu, const TQString &newMenu);
   void removeMenu(const TQString &menuName);

   void setLayout(const TQString &menuName, const TQStringList &layout);

   /**
    * Returns a unique menu-name for a new menu under @p menuName 
    * inspired by @p newMenu and not part of @p excludeList
    */
   TQString uniqueMenuName(const TQString &menuName, const TQString &newMenu, const TQStringList &excludeList);

protected:
   /**
    * Finds menu @p menuName in @p elem. 
    * If @p create is true, the menu is created if it doesn't exist yet.
    * @return The menu dom-node of @p menuName
    */
   TQDomElement findMenu(TQDomElement elem, const TQString &menuName, bool create);
   
private:
   TQString m_error;
   TQString m_fileName;

   TQDomDocument m_doc;
   bool m_bDirty;
   
   TQPtrList<ActionAtom> m_actionList;
   TQStringList m_removedEntries;
};


#endif
