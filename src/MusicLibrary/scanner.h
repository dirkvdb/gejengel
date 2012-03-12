//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>

#include "utils/fileoperations.h"

using namespace utils;

namespace Gejengel
{

class Track;
class Album;
class MusicDb;
class IScanSubscriber;

class Scanner : public fileops::IFileIterator
{
public:
    Scanner(MusicDb& db, IScanSubscriber& subscriber, const std::vector<std::string>& albumArtFilenames);
    ~Scanner();

    void performScan(const std::string& libraryPath);
    void cancel();
    bool onFile(const std::string& filepath);

private:
    MusicDb&                        m_LibraryDb;
    IScanSubscriber&                m_ScanSubscriber;
    int32_t                         m_ScannedFiles;
    std::vector<std::string>        m_AlbumArtFilenames;
    bool                            m_InitialScan;
    bool							m_Stop;
};

}

#endif
