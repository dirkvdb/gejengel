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

#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include <string>

#include "utils/types.h"
#include "reader.h"

namespace audio
{

class Frame;
struct Format;

class IDecoder
{
public:
    IDecoder(const std::string& uri) : m_Filepath(uri), m_AudioClock(0.0) {}
    virtual ~IDecoder() {}

    virtual bool decodeAudioFrame(Frame& audioFrame) = 0;
    virtual void seekAbsolute(double time) = 0;
    virtual void seekRelative(double offset) = 0;

    virtual Format getAudioFormat() = 0;

    virtual double  getAudioClock() { return m_AudioClock; }
    virtual double  getDuration() = 0;
    virtual double  getProgress() = 0;
    virtual int32_t getFrameSize() = 0;

protected:
    std::string         m_Filepath;
    double              m_AudioClock;
};

}

#endif

