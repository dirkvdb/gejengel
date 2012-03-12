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

#ifndef OPENAL_RENDERER_H
#define OPENAL_RENDERER_H

#include <AL/al.h>
#include <AL/alc.h>

#include <deque>
#include "audiorenderer.h"

#define NUM_BUFFERS 100

namespace Gejengel
{

class AudioFrame;
struct AudioFormat;

class OpenALRenderer : public AudioRenderer
{
public:
    OpenALRenderer();
    virtual ~OpenALRenderer();

    void setFormat(const AudioFormat& format);
    bool hasBufferSpace(uint32_t dataSize);
    void queueFrame(const AudioFrame& frame);
    void clearBuffers();
    void flushBuffers();
    int getBufferSize();
    double getCurrentPts();
    void play();
    void pause();
    void resume();
    void stop(bool drain);
    void setVolume(int volume);
    int getVolume();
    bool isPlaying();

private:

    ALCdevice*          m_pAudioDevice;
    ALCcontext*         m_pAlcContext;
    ALuint              m_AudioSource;
    ALuint              m_AudioBuffers[NUM_BUFFERS];
    int                 m_CurrentBuffer;
    int                 m_Volume;
    ALenum              m_AudioFormat;
    ALsizei             m_Frequency;

    std::deque<double>  m_PtsQueue;
};

}

#endif
