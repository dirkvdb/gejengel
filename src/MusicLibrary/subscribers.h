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

#ifndef SUBSCRIBERS_H
#define SUBSCRIBERS_H

#include "utils/types.h"

namespace Gejengel
{

class Track;
class Album;

class ILibrarySubscriber
{
public:
    virtual ~ILibrarySubscriber() {}

    virtual void newTrack(const Track& track) {}
    virtual void deletedTrack(const std::string& id) {}
    virtual void updatedTrack(const Track& track) {}
    virtual void newAlbum(const Album& album) {}
    virtual void deletedAlbum(const std::string& id) {}
    virtual void updatedAlbum(const Album& album) {}
    virtual void libraryCleared() {}
};

class IScanSubscriber
{
public:
    virtual ~IScanSubscriber() {}
    virtual void scanStart(uint32_t numTracks) {}
    virtual void scanUpdate(uint32_t scannedTracks) {}
    virtual void scanFinish() {}
    virtual void scanFailed() {}
};

}

#endif
