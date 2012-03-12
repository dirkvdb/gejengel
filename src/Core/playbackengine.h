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

#ifndef PLAYBACK_ENGINE_H
#define PLAYBACK_ENGINE_H

#include "gejengeltypes.h"

#include "utils/types.h"

namespace Gejengel
{

class Track;
class GejengelCore;

class PlaybackEngine
{
public:
    PlaybackEngine(GejengelCore& core) : m_Core(core) {}
    virtual ~PlaybackEngine() {}

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    virtual void prev() = 0;
    virtual void next() = 0;

    virtual PlaybackState getState() = 0;
    virtual bool isPlaying() = 0;

    virtual void seek(double seconds) = 0;
    virtual double getCurrentTime() = 0;
    virtual double getDuration() = 0;

    virtual void setVolume(int32_t volume) = 0;
    virtual int32_t getVolume() const = 0;

    virtual const Track& getTrack() const = 0;

protected:
    GejengelCore&   m_Core;
};

}

#endif
