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

#include "album.h"

#include <cstring>
#include <sstream>

namespace Gejengel
{

bool Album::operator==(const Album& otherAlbum) const
{
    return     (id				== otherAlbum.id)
			&& (artist          == otherAlbum.artist)
            && (title           == otherAlbum.title)
            && (year            == otherAlbum.year)
            && (genre           == otherAlbum.genre)
            && (trackCount      == otherAlbum.trackCount)
            && (discCount       == otherAlbum.discCount)
            && (durationInSec   == otherAlbum.durationInSec)
            && (fileSize        == otherAlbum.fileSize)
            && (dateAdded       == otherAlbum.dateAdded)
            && (artUrl			== otherAlbum.artUrl);
}

std::ostream& operator<<(std::ostream& os, const Album& album)
{
    os  << album.id << " " << album.artist << " - " << album.title << " (" << album.year << ")" << std::endl
        << album.genre << " " << album.discCount << " " << album.trackCount << std::endl
        << album.durationInSec << " " << album.fileSize << " " << album.dateAdded << " " << album.artUrl << std::endl;

    return os;
}

}
