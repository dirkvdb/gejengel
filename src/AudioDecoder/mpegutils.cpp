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

#include "mpegutils.h"

#include "reader.h"
#include "utils/log.h"

#include <cstring>
#include <cmath>

using namespace std;
using namespace utils;

namespace audio
{

namespace MpegUtils
{

static inline void endianSwap(uint32_t& value)
{
    value = (value >> 24) |
            ((value << 8) & 0x00FF0000) |
            ((value >> 8) & 0x0000FF00) |
            (value << 24);
}

static uint32_t convertSize(uint8_t size[4])
{
    uint32_t result = 0;

    for (int32_t i = 0; i < 4; ++i)
    {
        uint32_t currentByte = static_cast<uint32_t>(size[i]);
        result |= currentByte << (3 - i)  * 7;
    }

    return result;
}

uint32_t skipId3Tag(IReader& reader)
{
    struct Id3Header
    {
        uint8_t     id[3];
        uint16_t    version;
        uint8_t     flags;
        uint8_t     size[4];
    };

    Id3Header header;
    uint64_t startPos = reader.currentPosition();
    reader.read(header.id, 3);
    
    if (memcmp(header.id, "ID3", 3) == 0)
    {
        uint8_t data[7];
        reader.read(data, 7);
        
        memcpy(reinterpret_cast<char*>(&header.version), data, 2);
        memcpy(reinterpret_cast<char*>(&header.flags), &data[2], 1);
        memcpy(reinterpret_cast<char*>(&header.size), &data[3], 4);

        uint32_t id3Size = 10 + convertSize(header.size);
        if (header.flags & (0x10))
        {
            //bit 4 is active (10 byte footer)
            id3Size += 10;
        }

        reader.seekAbsolute(id3Size);
        return id3Size;
    }
    else
    {
        reader.seekAbsolute(startPos);
    }

    return 0;
}

uint32_t searchMpegHeader(IReader& reader, MpegHeader& header, uint32_t id3Offset, uint32_t& xingPos)
{
    uint64_t startPos = reader.currentPosition();

    for (int32_t i = id3Offset; i < 1048576; ++i)
    {
        reader.seekAbsolute(i);

        uint32_t size = readMpegHeader(reader, header, xingPos);
        if (size > 0 /*|| stream.fail()*/)
        {
            return size;
        }
    }

    reader.seekAbsolute(startPos);
    return 0;
}

uint32_t readMpegHeader(IReader& reader, MpegHeader& header, uint32_t& xingPos)
{
    uint64_t startPos = reader.currentPosition();
    
    uint8_t headerBytes[4];
    reader.read(reinterpret_cast<uint8_t*>(headerBytes), 4);
    //if (stream.fail())
    //{
    //    reader.seekAbsolute(startPos);
    //    return 0;
    //}
    
    if ((headerBytes[0] != 0xFF) || ((headerBytes[1] & 0xE0) != 0xE0))
    {
        reader.seekAbsolute(startPos);
        return 0;
    }

    static const int32_t freeFormatCode = 0x00;
    static const int32_t mpegV1Bitrates[15] = { 0, 32,  40,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320 };
    static const int32_t mpegV2Bitrates[15] = { 0,  8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160 };
    static const int32_t sampleRates[3] = { 44100, 48000, 32000 };

    int32_t versionCode = (headerBytes[1] & 0x18) >> 3;
    switch (versionCode)
    {
    case 0x00:
        header.version = MpegHeader::Mpeg2_5;
        break;
    case 0x02:
        header.version = MpegHeader::Mpeg2;
        break;
    case 0x03:
        header.version = MpegHeader::Mpeg1;
        break;
    default:
        reader.seekAbsolute(startPos);
        return 0;
    }

    int32_t layerCode   = (headerBytes[1] & 0x06) >> 1;
    switch (layerCode)
    {
    case 0x01:
        header.layer = MpegHeader::Layer3;
        header.samplesPerFrame = (header.version == MpegHeader::Mpeg1) ? 1152 : 576;
        break;
    case 0x02:
        header.layer = MpegHeader::Layer2;
        header.samplesPerFrame = 1152;
        break;
    case 0x03:
        header.layer = MpegHeader::Layer1;
        header.samplesPerFrame = 384;
        break;
    default:
        reader.seekAbsolute(startPos);
        return 0;
    }

    int32_t bitrateCode = (headerBytes[2] & 0xF0) >> 4;
    if (!(bitrateCode >= 0x00 && bitrateCode <= 0x0E))
    {
        log::debug("Invalid bitrate code: %d", bitrateCode);
        reader.seekAbsolute(startPos);
        return 0;
    }

    if (bitrateCode == freeFormatCode)
    {
        header.bitRate = 0;
    }

    int32_t sampleRateCode = (headerBytes[2] & 0x0c) >> 2;
    if (!(sampleRateCode >= 0x00 && sampleRateCode <= 0x02))
    {
        log::debug("Invalid samplerate code: %d", sampleRateCode);
        reader.seekAbsolute(startPos);
        return 0;
    }

    header.sampleRate = sampleRates[sampleRateCode];

    int32_t channelCode = (headerBytes[3] & 0xC0) >> 6;
    switch (channelCode)
    {
    case 0x00:
        header.channelMode = MpegHeader::Stereo;
        header.numChannels = 2;
        break;
    case 0x01:
        header.channelMode = MpegHeader::JointStereo;
        header.numChannels = 2;
        break;
    case 0x02:
        header.channelMode = MpegHeader::DualChannel;
        header.numChannels = 2;
        break;
    case 0x03:
        header.channelMode = MpegHeader::Mono;
        header.numChannels = 1;
        break;
    default:
        header.channelMode = MpegHeader::UnknownChannelMode;
        log::debug("Invalid channelMode: %d", channelCode);
        reader.seekAbsolute(startPos);
        return 0;
    }

    if (header.version == MpegHeader::Mpeg1)
    {
        xingPos = (header.channelMode == MpegHeader::Mono) ? 21 : 36;
    }
    else
    {
        xingPos = (header.channelMode == MpegHeader::Mono) ? 13 : 21;
    }

    int32_t padding = (headerBytes[2] & 0x02) >> 1;
    if (bitrateCode != 0)
    {
        header.bitRate = (header.version == MpegHeader::Mpeg1) ? mpegV1Bitrates[bitrateCode] : mpegV2Bitrates[bitrateCode];
        return static_cast<uint32_t>(floor(144.0 * header.bitRate * 1000.0 / header.sampleRate) + padding);
    }

    //size is ignored when bitrate == 0 (freeformat)

    if (header.version == MpegHeader::Mpeg1)
    {
        header.bitRate = ((header.samplesPerFrame / 4 - padding) * header.sampleRate + 11) / 12;
    }
    else
    {
        header.bitRate = ((header.samplesPerFrame - padding) * header.sampleRate + 143) / 144;
    }
    
    return 1;
}

uint32_t readXingHeader(IReader& reader, XingHeader& header)
{
    uint64_t startPos = reader.currentPosition();
    uint32_t size = 8;
    
    char identifier[4];
    reader.read(reinterpret_cast<uint8_t*>(identifier), 4);

    if ((memcmp(identifier, "Xing", 4) == 0))
    {
        header.vbr = true;
    }
    else if (memcmp(identifier, "Info", 4) == 0)
    {
        header.vbr = false;
    }
    else
    {
        reader.seekAbsolute(startPos);
        return 0;
    }

    uint32_t xingFlags;
    reader.read(reinterpret_cast<uint8_t*>(&xingFlags), 4);
    #ifndef WORDS_BIGENDIAN
    endianSwap(xingFlags);
    #endif

    if (xingFlags & 0x0001)
    {
        reader.read(reinterpret_cast<uint8_t*>(&header.numFrames), 4);
        #ifndef WORDS_BIGENDIAN
        endianSwap(header.numFrames);
        #endif
        size += 4;
    }

    if (xingFlags & 0x0002)
    {
        reader.read(reinterpret_cast<uint8_t*>(&header.numBytes), 4);
        #ifndef WORDS_BIGENDIAN
        endianSwap(header.numBytes);
        #endif
        size += 4;
    }

    if (xingFlags & 0x0003)
    {
        reader.read(reinterpret_cast<uint8_t*>(&header.toc), 100);
        size += 100;
        header.hasToc = true;
    }

    if (xingFlags & 0x0004)
    {
        reader.read(reinterpret_cast<uint8_t*>(&header.quality), 4);
        #ifndef WORDS_BIGENDIAN
        endianSwap(header.quality);
        #endif
        size += 4;
    }

    return size;
}

uint32_t readLameHeader(IReader& reader, LameHeader& header)
{
    uint64_t startPos = reader.currentPosition();

    uint8_t identifier[4];
    reader.read(identifier, 4);

    if (memcmp(identifier, "LAME", 4) != 0)
    {
        reader.seekAbsolute(startPos);
        return 0;
    }

    const uint32_t delaysOffset = 17;
    
    uint8_t encoderDelays[3];
    reader.seekRelative(delaysOffset);
    reader.read(reinterpret_cast<uint8_t*>(encoderDelays), 3);

    header.encoderDelay = (static_cast<uint32_t>(encoderDelays[0]) << 4) | ((encoderDelays[1] & 0xF0) >> 4);
    header.zeroPadding = ((static_cast<uint32_t>(encoderDelays[1]) & 0x0F) << 8) | encoderDelays[2];

    if (header.encoderDelay > 2880)
    {
        header.encoderDelay = 0;
    }

    if (header.zeroPadding > 2 * 1152)
    {
        header.zeroPadding = 0;
    }

    //set read pointer behind lame tag
    reader.seekRelative(12);

    return 36;
}

}

}
