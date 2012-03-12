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

#ifndef TRACK_H
#define TRACK_H

#include <string>
#include <iostream>

#include "libraryitem.h"
#include "utils/types.h"

namespace Gejengel
{

class Track : public LibraryItem
{
public:
    Track()
    : year(0), trackNr(0), discNr(0), durationInSec(0)
    , bitrate(0), sampleRate(0), channels(0), fileSize(0), modifiedTime(0)
    {}

    bool operator==(const Track& otherItem) const;
    friend std::ostream& operator<<(std::ostream &os, const Track& item);

    std::string     albumId;
    std::string     filepath;
    std::string     artist;
    std::string     title;
    std::string     album;
    std::string     albumArtist;
    std::string     genre;
    std::string     composer;
    uint32_t        year;
    uint32_t        trackNr;
    uint32_t        discNr;
    uint32_t        durationInSec;
    uint32_t        bitrate;
    uint32_t        sampleRate;
    uint32_t        channels;
    int64_t         fileSize;
    uint32_t        modifiedTime;
    std::string		artUrl;
};

std::ostream& operator<<(std::ostream& os, const Track& item);

}

#endif
