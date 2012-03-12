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

#ifndef ALBUM_H
#define ALBUM_H

#include <string>
#include <vector>
#include <iosfwd>

#include "libraryitem.h"
#include "utils/types.h"

namespace Gejengel
{

class Album : public LibraryItem
{
public:
    Album(const std::string& id = "")
    : LibraryItem(id)
    , year(0), trackCount(0), discCount(0), durationInSec(0), fileSize(0), dateAdded(0)
    {}
    
    bool operator==(const Album& otherItem) const;
    friend std::ostream& operator<<(std::ostream &os, const Album& item);

    std::string             artist;
    std::string             title;
    std::string             genre;
    uint32_t                year;
    uint32_t                trackCount;
    uint32_t                discCount;
    uint32_t                durationInSec;
    uint32_t                fileSize;
    time_t                  dateAdded;
    std::string             artUrl;
};

std::ostream& operator<<(std::ostream& os, const Album& item);

}

#endif
