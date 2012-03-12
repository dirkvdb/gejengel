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

#ifndef AUDIOFORMAT_H
#define AUDIOFORMAT_H

#include "utils/types.h"

namespace Gejengel
{

struct AudioFormat
{
    AudioFormat()
    : bits(0), rate(0), numChannels(0), framesPerPacket(0)
    {}
    
    bool operator==(const AudioFormat& otherFormat) const
    {
        return     (bits            == otherFormat.bits)
                && (rate            == otherFormat.rate)
                && (numChannels     == otherFormat.numChannels)
                && (framesPerPacket == otherFormat.framesPerPacket);
    }

    uint32_t bits;
    uint32_t rate;
    uint32_t numChannels;
    uint32_t framesPerPacket;
};

}

#endif
