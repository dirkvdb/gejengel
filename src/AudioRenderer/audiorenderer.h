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

#ifndef AUDIORENDERER_H
#define AUDIORENDERER_H

#include "utils/types.h"

namespace Gejengel
{

class AudioFrame;
struct AudioFormat;

class AudioRenderer
{
public:
    AudioRenderer();
    virtual ~AudioRenderer();

    virtual void setFormat(const AudioFormat& format) = 0;

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop(bool drain) = 0;
    virtual void setVolume(int volume) = 0;
    virtual int getVolume() = 0;

    virtual bool isPlaying() = 0;

    virtual bool hasBufferSpace(uint32_t dataSize) = 0;
    virtual void flushBuffers() = 0;
    virtual void queueFrame(const AudioFrame& frame) = 0;

    virtual double getCurrentPts() = 0;
};

}

#endif
