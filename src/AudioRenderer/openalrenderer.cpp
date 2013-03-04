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

#include "openalrenderer.h"

#include "audioframe.h"
#include "audioformat.h"
#include "utils/numericoperations.h"
#include "utils/log.h"

#include <cassert>
#include <stdexcept>

using namespace std;
using namespace utils;

namespace Gejengel
{

OpenALRenderer::OpenALRenderer()
: AudioRenderer()
, m_pAudioDevice(nullptr)
, m_pAlcContext(nullptr)
, m_AudioSource(0)
, m_CurrentBuffer(0)
, m_Volume(100)
, m_AudioFormat(AL_FORMAT_STEREO16)
, m_Frequency(0)
{
    m_pAudioDevice = alcOpenDevice(nullptr);

    if (m_pAudioDevice)
    {
        m_pAlcContext = alcCreateContext(m_pAudioDevice, nullptr);
        alcMakeContextCurrent(m_pAlcContext);
    }

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        log::warn("Openal creation error $d", err);
    }
    
    alGenBuffers(NUM_BUFFERS, m_AudioBuffers);
    alGenSources(1, &m_AudioSource);
}

OpenALRenderer::~OpenALRenderer()
{
    alSourceStop(m_AudioSource);
    alDeleteSources(1, &m_AudioSource);
    alDeleteBuffers(NUM_BUFFERS, m_AudioBuffers);

    if (m_pAudioDevice)
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_pAlcContext);
        alcCloseDevice(m_pAudioDevice);
    }
}

void OpenALRenderer::setFormat(const AudioFormat& format)
{
    switch (format.bits)
    {
    case 8:
        m_AudioFormat = format.numChannels == 1 ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
        break;
    case 16:
        m_AudioFormat = format.numChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        break;
    default:
        throw logic_error("OpenAlRenderer: unsupported format");
    }

    m_Frequency = format.rate;
}

bool OpenALRenderer::hasBufferSpace(uint32_t dataSize)
{
    int queued = 0;
    alGetSourcei(m_AudioSource, AL_BUFFERS_QUEUED, &queued);

    if (queued == 0)
    {
        log::debug("OpenalRenderer: xrun");
    }

    return queued < NUM_BUFFERS;
}

void OpenALRenderer::queueFrame(const AudioFrame& frame)
{
    assert(frame.getFrameData());
    alBufferData(m_AudioBuffers[m_CurrentBuffer], m_AudioFormat, frame.getFrameData(), frame.getDataSize(), m_Frequency);
    alSourceQueueBuffers(m_AudioSource, 1, &m_AudioBuffers[m_CurrentBuffer]);
    m_PtsQueue.push_back(frame.getPts());

    ++m_CurrentBuffer;
    m_CurrentBuffer %= NUM_BUFFERS;

    ALenum err = alGetError();
    if (err != AL_NO_ERROR)
    {
        log::warn("Openal queueFrame error %d", err);
    }
}

void OpenALRenderer::clearBuffers()
{
    stop(false);

    int queued = 0;
    alGetSourcei(m_AudioSource, AL_BUFFERS_QUEUED, &queued);

    if (queued > 0)
    {
        ALuint* buffers = new ALuint[queued];
        alSourceUnqueueBuffers(m_AudioSource, queued, buffers);
        delete[] buffers;
    }
    m_PtsQueue.clear();
}

void OpenALRenderer::flushBuffers()
{
    int processed = 0;
    alGetSourcei(m_AudioSource, AL_BUFFERS_PROCESSED, &processed);

    while(processed--)
    {
        ALuint buffer;
        alSourceUnqueueBuffers(m_AudioSource, 1, &buffer);
        m_PtsQueue.pop_front();

        ALenum err = alGetError();
        if (err != AL_NO_ERROR)
        {
            log::warn("Openal flushBuffers error %d", err);
        }
    }
}

bool OpenALRenderer::isPlaying()
{
    ALenum state;
    alGetSourcei(m_AudioSource, AL_SOURCE_STATE, &state);

    return (state == AL_PLAYING);
}

void OpenALRenderer::play()
{
    if (!isPlaying() && !m_PtsQueue.empty())
    {
        alSourcePlay(m_AudioSource);
    }
}

void OpenALRenderer::pause()
{
    if (isPlaying())
    {
        alSourcePause(m_AudioSource);
    }
}

void OpenALRenderer::resume()
{
    play();
}

void OpenALRenderer::stop(bool drain)
{
    alSourceStop(m_AudioSource);
    flushBuffers();
}

int OpenALRenderer::getBufferSize()
{
    return NUM_BUFFERS;
}

void OpenALRenderer::setVolume(int volume)
{
    numericops::clip(m_Volume, 0, 100);
    m_Volume = volume;
    alSourcef(m_AudioSource, AL_GAIN, m_Volume / 100.f);
}

int OpenALRenderer::getVolume()
{
    return m_Volume;
}

double OpenALRenderer::getCurrentPts()
{
    return m_PtsQueue.empty() ? 0 : m_PtsQueue.front();
}

}
