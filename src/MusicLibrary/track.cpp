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

#include "track.h"

namespace Gejengel
{

bool Track::operator==(const Track& otherItem) const
{
    return     (id              == otherItem.id)
            && (albumId         == otherItem.albumId)
            && (filepath        == otherItem.filepath)
            && (artist          == otherItem.artist)
            && (title           == otherItem.title)
            && (album           == otherItem.album)
            && (albumArtist     == otherItem.albumArtist)
            && (genre           == otherItem.genre)
            && (composer        == otherItem.composer)
            && (year            == otherItem.year)
            && (trackNr         == otherItem.trackNr)
            && (discNr          == otherItem.discNr)
            && (durationInSec   == otherItem.durationInSec)
            && (bitrate         == otherItem.bitrate)
            && (sampleRate      == otherItem.sampleRate)
            && (channels        == otherItem.channels)
            && (fileSize        == otherItem.fileSize)
            && (artUrl        	== otherItem.artUrl);

}

std::ostream& operator<<(std::ostream& os, const Track& item)
{
    os  << item.id << " (" << item.albumId << ")" << std::endl
        << item.filepath << std::endl
        << item.artist << std::endl
        << item.title << std::endl
        << item.album << std::endl
        << item.albumArtist << std::endl
        << item.genre << std::endl
        << item.composer << std::endl
        << item.year << " " << item.trackNr << " " << item.discNr << std::endl
        << item.durationInSec << " " << item.bitrate << " " << item.sampleRate << " "
        << item.channels << std::endl << item.fileSize << " " << item.modifiedTime << std::endl;

    return os;
}

}
