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

#ifndef ALBUM_ART_PROVIDER_H
#define ALBUM_ART_PROVIDER_H

#include "utils/types.h"
#include "utils/subscriber.h"

namespace Gejengel
{
    
class Track;
class Album;
class AlbumArt;

class IAlbumArtProvider
{
public:
    virtual void getAlbumArt(const Album& album, utils::ISubscriber<const AlbumArt&>& subscriber) = 0;
    virtual void getAlbumArt(const Track& track, utils::ISubscriber<const AlbumArt&>& subscriber) = 0;
    virtual void getAlbumArtFromSource(const Album& album, uint32_t size, utils::ISubscriber<const AlbumArt&>& subscriber) = 0;
    virtual void getAlbumArtFromSource(const Track& track, uint32_t size, utils::ISubscriber<const AlbumArt&>& subscriber) = 0;
};

}

#endif
