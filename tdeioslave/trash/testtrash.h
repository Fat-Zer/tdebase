/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TESTTRASH_H
#define TESTTRASH_H

#include <tqobject.h>

class TestTrash : public QObject
{
    Q_OBJECT

public:
    TestTrash() {}
    void setup();
    void cleanTrash();
    void runAll();

    // tests

    void urlTestFile();
    void urlTestDirectory();
    void urlTestSubDirectory();

    void trashFileFromHome();
    void trashPercentFileFromHome();
    void trashUtf8FileFromHome();
    void trashUmlautFileFromHome();
    void testTrashNotEmpty();
    void trashFileFromOther();
    void trashFileIntoOtherPartition();
    void trashFileOwnedByRoot();
    void trashSymlinkFromHome();
    void trashSymlinkFromOther();
    void trashBrokenSymlinkFromHome();
    void trashDirectoryFromHome();
    void trashReadOnlyDirFromHome();
    void trashDirectoryFromOther();
    void trashDirectoryOwnedByRoot();

    void tryRenameInsideTrash();

    void statRoot();
    void statFileInRoot();
    void statDirectoryInRoot();
    void statSymlinkInRoot();
    void statFileInDirectory();

    void copyFileFromTrash();
    void copyFileInDirectoryFromTrash();
    void copyDirectoryFromTrash();
    void copySymlinkFromTrash();

    void moveFileFromTrash();
    void moveFileInDirectoryFromTrash();
    void moveDirectoryFromTrash();
    void moveSymlinkFromTrash();

    void listRootDir();
    void listRecursiveRootDir();
    void listSubDir();

    void delRootFile();
    void delFileInDirectory();
    void delDirectory();

    void getFile();
    void restoreFile();
    void restoreFileFromSubDir();
    void restoreFileToDeletedDirectory();

    void emptyTrash();

private slots:
    void slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList& );

private:
    void trashFile( const TQString& origFilePath, const TQString& fileId );
    void trashSymlink( const TQString& origFilePath, const TQString& fileName, bool broken );
    void trashDirectory( const TQString& origPath, const TQString& fileName );
    void copyFromTrash( const TQString& fileId, const TQString& destPath, const TQString& relativePath = TQString::null );
    void moveFromTrash( const TQString& fileId, const TQString& destPath, const TQString& relativePath = TQString::null );

    TQString homeTmpDir() const;
    TQString otherTmpDir() const;
    TQString utf8FileName() const;
    TQString umlautFileName() const;
    TQString readOnlyDirPath() const;

    TQString m_trashDir;

    TQString m_otherPartitionTopDir;
    TQString m_otherPartitionTrashDir;
    bool m_tmpIsWritablePartition;
    int m_tmpTrashId;
    int m_otherPartitionId;

    int m_entryCount;
    TQStringList m_listResult;
};

#endif
