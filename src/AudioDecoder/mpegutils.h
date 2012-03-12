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

#ifndef MPEG_UTILS_H
#define MPEG_UTILS_H

#include <fstream>

#include "utils/types.h"

namespace Gejengel
{
    
class IReader;

namespace MpegUtils
{

struct MpegHeader
{
    enum Version
    {
        Mpeg1,
        Mpeg2,
        Mpeg2_5,
        UnknownVersion
    };

    enum Layer
    {
        Layer1 = 1,
        Layer2 = 2,
        Layer3 = 3,
        UnknownLayer
    };

    enum ChannelMode
    {
        Mono,
        Stereo,
        DualChannel,
        JointStereo,
        UnknownChannelMode
    };

    MpegHeader()
    : bitRate(0), sampleRate(0), samplesPerFrame(0), version(UnknownVersion)
    , layer(UnknownLayer), numChannels(0), channelMode(UnknownChannelMode)
    {}

    uint32_t    bitRate; //bitrate of 0 means freeformat
    uint32_t    sampleRate;
    uint32_t    samplesPerFrame;
    Version     version;
    Layer       layer;
    uint32_t    numChannels;
    ChannelMode channelMode;
};

struct XingHeader
{
    XingHeader()
    : numFrames(0), numBytes(0), quality(0), hasToc(false), vbr(false)
    {}
    
    uint32_t    numFrames;
    uint32_t    numBytes;
    uint8_t     toc[100];
    uint32_t    quality;
    bool        hasToc;
    bool        vbr;
};

struct LameHeader
{
    LameHeader()
    : encoderDelay(0), zeroPadding(0), replayGain(0), mp3Gain(0)
    {}
    
    uint32_t    encoderDelay;
    uint32_t    zeroPadding;
    uint32_t    replayGain;
    uint32_t    mp3Gain;
};

uint32_t skipId3Tag(IReader& reader);
uint32_t searchMpegHeader(IReader& reader, MpegHeader& header, uint32_t id3Offset, uint32_t& xingPos);
uint32_t readMpegHeader(IReader& reader, MpegHeader& header, uint32_t& xingPos);
uint32_t readXingHeader(IReader& reader, XingHeader& header);
uint32_t readLameHeader(IReader& reader, LameHeader& header);

}

}

#endif
